// Copyright (C) 2026 Raymond-Leung7
// SPDX-License-Identifier: MIT
#include "HwInfoSharedMemoryReader.h"
#include <algorithm>
#include <cstring>
#include <format>
#include <limits>
#include <string_view>
#include <utility>

namespace p2c::pmon
{
    namespace
    {
        constexpr wchar_t kMappingName[] = L"Global\\HWiNFO_SENS_SM2";
        constexpr wchar_t kMutexName[] = L"Global\\HWiNFO_SM2_MUTEX";
        constexpr uint32_t kHeaderMagic = 0x53695748u;
        constexpr uint32_t kMinimumSupportedVersion = 1;
        constexpr uint32_t kMaximumSupportedVersion = 2;
        constexpr size_t kHeaderSize = 44;
        constexpr size_t kSensorIdOffset = 0;
        constexpr size_t kSensorInstanceOffset = 4;
        constexpr size_t kSensorNameOriginalOffset = 8;
        constexpr size_t kSensorNameUserOffset = 136;
        constexpr size_t kSensorNameUtf8Offset = 264;
        constexpr size_t kSensorNameLength = 128;
        constexpr size_t kSensorMinimumSize = 264;
        constexpr size_t kSensorUtf8MinimumSize = 392;
        constexpr size_t kEntryReadingTypeOffset = 0;
        constexpr size_t kEntrySensorIndexOffset = 4;
        constexpr size_t kEntryIdOffset = 8;
        constexpr size_t kEntryNameOriginalOffset = 12;
        constexpr size_t kEntryNameUserOffset = 140;
        constexpr size_t kEntryNameLength = 128;
        constexpr size_t kEntryUnitOffset = 268;
        constexpr size_t kEntryUnitLength = 16;
        constexpr size_t kEntryValueOffset = 284;
        constexpr size_t kEntryNameUtf8Offset = 316;
        constexpr size_t kEntryUnitUtf8Offset = 444;
        constexpr size_t kEntryMinimumSize = 292;
        constexpr size_t kEntryUtf8MinimumSize = 460;
        constexpr uint32_t kMaximumSensorCount = 4096;
        constexpr uint32_t kMaximumEntryCount = 65536;
        constexpr uint32_t kMaximumElementSize = 65536;
        constexpr auto kStaleTimeout = std::chrono::seconds{ 30 };
        constexpr auto kFutureTimestampTolerance = std::chrono::seconds{ 5 };
        constexpr int kOpenSnapshotAttempts = 20;
        constexpr DWORD kOpenSnapshotRetryDelayMs = 5;

#pragma pack(push, 1)
        struct SharedHeader_
        {
            uint32_t magic;
            uint32_t version;
            uint32_t revision;
            int64_t lastUpdate;
            uint32_t sensorOffset;
            uint32_t sensorElementSize;
            uint32_t sensorCount;
            uint32_t entryOffset;
            uint32_t entryElementSize;
            uint32_t entryCount;
        };
#pragma pack(pop)

        static_assert(sizeof(SharedHeader_) == kHeaderSize);

        template<typename T>
        T ReadValue_(const uint8_t* pData, size_t offset)
        {
            T value{};
            std::memcpy(&value, pData + offset, sizeof(value));
            return value;
        }

        bool SectionFits_(uint32_t offset, uint32_t elementSize, uint32_t count, size_t viewSize)
        {
            if ((size_t)offset > viewSize) {
                return false;
            }
            if (count != 0 && (size_t)elementSize > (std::numeric_limits<size_t>::max)() / (size_t)count) {
                return false;
            }
            const size_t byteCount = (size_t)elementSize * (size_t)count;
            return byteCount <= viewSize - (size_t)offset;
        }

        std::string ReadFixedString_(const uint8_t* pData, size_t length)
        {
            const auto* pChars = reinterpret_cast<const char*>(pData);
            const auto* pEnd = std::find(pChars, pChars + length, '\0');
            return { pChars, pEnd };
        }

        std::string ConvertToUtf8_(std::string_view input)
        {
            if (input.empty()) {
                return {};
            }

            const auto convert = [input](UINT codePage, DWORD flags) -> std::string {
                const int inputLength = (int)input.size();
                const int wideLength = MultiByteToWideChar(codePage, flags, input.data(), inputLength, nullptr, 0);
                if (wideLength <= 0) {
                    return {};
                }
                std::wstring wide((size_t)wideLength, L'\0');
                if (MultiByteToWideChar(codePage, flags, input.data(), inputLength, wide.data(), wideLength) <= 0) {
                    return {};
                }
                const int utf8Length = WideCharToMultiByte(
                    CP_UTF8, 0, wide.data(), wideLength, nullptr, 0, nullptr, nullptr);
                if (utf8Length <= 0) {
                    return {};
                }
                std::string utf8((size_t)utf8Length, '\0');
                if (WideCharToMultiByte(
                    CP_UTF8, 0, wide.data(), wideLength, utf8.data(), utf8Length, nullptr, nullptr) <= 0) {
                    return {};
                }
                return utf8;
            };

            if (auto utf8 = convert(CP_UTF8, MB_ERR_INVALID_CHARS); !utf8.empty()) {
                return utf8;
            }
            if (auto utf8 = convert(CP_ACP, 0); !utf8.empty()) {
                return utf8;
            }
            return std::string{ input };
        }

        std::string SelectName_(const uint8_t* pData, size_t elementSize, size_t utf8Offset,
            size_t utf8MinimumSize, size_t userOffset, size_t originalOffset, size_t length)
        {
            if (elementSize >= utf8MinimumSize) {
                auto name = ConvertToUtf8_(ReadFixedString_(pData + utf8Offset, length));
                if (!name.empty()) {
                    return name;
                }
            }
            auto name = ConvertToUtf8_(ReadFixedString_(pData + userOffset, length));
            if (name.empty()) {
                name = ConvertToUtf8_(ReadFixedString_(pData + originalOffset, length));
            }
            return name;
        }

        bool TryGetInitialAge_(int64_t lastUpdate, std::chrono::steady_clock::duration& age)
        {
            if (lastUpdate <= 0) {
                return false;
            }

            const auto now = std::chrono::system_clock::now();
            const auto sampleTime = std::chrono::system_clock::time_point{
                std::chrono::seconds{ lastUpdate } };
            if (sampleTime > now + kFutureTimestampTolerance) {
                return false;
            }
            if (sampleTime >= now) {
                age = std::chrono::steady_clock::duration::zero();
                return true;
            }

            age = std::chrono::duration_cast<std::chrono::steady_clock::duration>(now - sampleTime);
            return age <= kStaleTimeout;
        }
    }

    HwInfoSharedMemoryReader::~HwInfoSharedMemoryReader()
    {
        Close_();
    }

    HwInfoSharedMemoryReader::HwInfoSharedMemoryReader()
        : mappingName_{ kMappingName }, mutexName_{ kMutexName }
    {}

    HwInfoSharedMemoryReader::HwInfoSharedMemoryReader(std::wstring mappingName, std::wstring mutexName)
        : mappingName_{ std::move(mappingName) }, mutexName_{ std::move(mutexName) }
    {}

    bool HwInfoSharedMemoryReader::Open() noexcept
    {
        Close_();

        hMapping_ = OpenFileMappingW(FILE_MAP_READ, FALSE, mappingName_.c_str());
        if (!hMapping_) {
            return false;
        }

        pView_ = reinterpret_cast<const uint8_t*>(MapViewOfFile(hMapping_, FILE_MAP_READ, 0, 0, 0));
        if (!pView_) {
            Close_();
            return false;
        }

        MEMORY_BASIC_INFORMATION memoryInfo{};
        if (VirtualQuery(pView_, &memoryInfo, sizeof(memoryInfo)) == 0 || memoryInfo.RegionSize < kHeaderSize) {
            Close_();
            return false;
        }
        const auto viewOffset = (size_t)(pView_ - reinterpret_cast<const uint8_t*>(memoryInfo.BaseAddress));
        if (viewOffset >= memoryInfo.RegionSize) {
            Close_();
            return false;
        }
        viewSize_ = memoryInfo.RegionSize - viewOffset;
        if (!mutexName_.empty()) {
            hMutex_ = OpenMutexW(SYNCHRONIZE | MUTEX_MODIFY_STATE, FALSE, mutexName_.c_str());
            if (!hMutex_) {
                Close_();
                return false;
            }
        }

        bool snapshotReady = false;
        for (int attempt = 0; attempt < kOpenSnapshotAttempts && !snapshotReady; ++attempt) {
            snapshotReady = ReadSnapshot_(true);
            if (!snapshotReady) {
                Sleep(kOpenSnapshotRetryDelayMs);
            }
        }
        if (!snapshotReady) {
            Close_();
            return false;
        }
        isOpen_ = true;
        return true;
    }

    bool HwInfoSharedMemoryReader::Refresh() noexcept
    {
        if (!isOpen_ || !pView_) {
            return false;
        }
        if (!ReadSnapshot_(false)) {
            return false;
        }
        return std::chrono::steady_clock::now() - lastChangeObserved_ <= kStaleTimeout;
    }

    const std::vector<std::string>& HwInfoSharedMemoryReader::Columns() const noexcept
    {
        return columns_;
    }

    const std::vector<double>& HwInfoSharedMemoryReader::Values() const noexcept
    {
        return values_;
    }

    int64_t HwInfoSharedMemoryReader::LastUpdate() const noexcept
    {
        return lastUpdate_;
    }

    int64_t HwInfoSharedMemoryReader::SampleAgeMs() const noexcept
    {
        if (!isOpen_ || lastChangeObserved_ == std::chrono::steady_clock::time_point{}) {
            return 0;
        }
        return std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - lastChangeObserved_).count();
    }

    bool HwInfoSharedMemoryReader::IsOpen() const noexcept
    {
        return isOpen_;
    }

    void HwInfoSharedMemoryReader::Close_() noexcept
    {
        isOpen_ = false;
        if (pView_) {
            UnmapViewOfFile(pView_);
            pView_ = nullptr;
        }
        if (hMutex_) {
            CloseHandle(hMutex_);
            hMutex_ = nullptr;
        }
        if (hMapping_) {
            CloseHandle(hMapping_);
            hMapping_ = nullptr;
        }
        viewSize_ = 0;
        sensorOffset_ = 0;
        sensorElementSize_ = 0;
        sensorCount_ = 0;
        entryOffset_ = 0;
        entryElementSize_ = 0;
        entryCount_ = 0;
        entryKeys_.clear();
        columns_.clear();
        values_.clear();
        lastUpdate_ = 0;
        lastChangeObserved_ = {};
    }

    bool HwInfoSharedMemoryReader::ReadSnapshot_(bool initializeColumns) noexcept
    {
        bool ownsMutex = false;
        if (hMutex_) {
            const DWORD waitResult = WaitForSingleObject(hMutex_, 20);
            if (waitResult != WAIT_OBJECT_0 && waitResult != WAIT_ABANDONED) {
                return false;
            }
            ownsMutex = true;
        }

        const auto releaseMutex = [this, &ownsMutex] {
            if (ownsMutex) {
                ReleaseMutex(hMutex_);
                ownsMutex = false;
            }
        };

        try {
            SharedHeader_ header{};
            std::memcpy(&header, pView_, sizeof(header));
            if (header.magic != kHeaderMagic ||
                header.version < kMinimumSupportedVersion ||
                header.version > kMaximumSupportedVersion ||
                header.sensorOffset < kHeaderSize || header.entryOffset < kHeaderSize ||
                header.sensorCount == 0 || header.entryCount == 0 ||
                header.sensorElementSize < kSensorMinimumSize ||
                header.entryElementSize < kEntryMinimumSize ||
                header.sensorElementSize > kMaximumElementSize ||
                header.entryElementSize > kMaximumElementSize ||
                header.sensorCount > kMaximumSensorCount ||
                header.entryCount > kMaximumEntryCount ||
                !SectionFits_(header.sensorOffset, header.sensorElementSize, header.sensorCount, viewSize_) ||
                !SectionFits_(header.entryOffset, header.entryElementSize, header.entryCount, viewSize_)) {
                releaseMutex();
                return false;
            }

            if (!initializeColumns &&
                (header.sensorOffset != sensorOffset_ ||
                    header.sensorElementSize != sensorElementSize_ ||
                    header.sensorCount != sensorCount_ ||
                    header.entryOffset != entryOffset_ ||
                    header.entryElementSize != entryElementSize_ ||
                    header.entryCount != entryCount_)) {
                releaseMutex();
                return false;
            }

            std::vector<EntryKey_> entryKeys;
            std::vector<std::string> columns;
            std::vector<double> values;
            entryKeys.reserve(header.entryCount);
            if (initializeColumns) {
                columns.reserve(header.entryCount);
            }
            values.reserve(header.entryCount);

            for (uint32_t i = 0; i < header.entryCount; ++i) {
                const auto* pEntry = pView_ + (size_t)header.entryOffset +
                    (size_t)i * (size_t)header.entryElementSize;
                const uint32_t readingType = ReadValue_<uint32_t>(pEntry, kEntryReadingTypeOffset);
                const uint32_t sensorIndex = ReadValue_<uint32_t>(pEntry, kEntrySensorIndexOffset);
                const uint32_t entryId = ReadValue_<uint32_t>(pEntry, kEntryIdOffset);
                if (sensorIndex >= header.sensorCount) {
                    releaseMutex();
                    return false;
                }

                const auto* pSensor = pView_ + (size_t)header.sensorOffset +
                    (size_t)sensorIndex * (size_t)header.sensorElementSize;
                const uint32_t sensorId = ReadValue_<uint32_t>(pSensor, kSensorIdOffset);
                const uint32_t sensorInstance = ReadValue_<uint32_t>(pSensor, kSensorInstanceOffset);
                entryKeys.push_back({ sensorId, sensorInstance, readingType, entryId });
                values.push_back(ReadValue_<double>(pEntry, kEntryValueOffset));

                if (initializeColumns) {
                    auto sensorName = SelectName_(pSensor, header.sensorElementSize,
                        kSensorNameUtf8Offset, kSensorUtf8MinimumSize, kSensorNameUserOffset,
                        kSensorNameOriginalOffset, kSensorNameLength);
                    auto entryName = SelectName_(pEntry, header.entryElementSize,
                        kEntryNameUtf8Offset, kEntryUtf8MinimumSize, kEntryNameUserOffset,
                        kEntryNameOriginalOffset, kEntryNameLength);
                    const size_t unitOffset = header.entryElementSize >= kEntryUtf8MinimumSize
                        ? kEntryUnitUtf8Offset
                        : kEntryUnitOffset;
                    const auto unit = ConvertToUtf8_(ReadFixedString_(pEntry + unitOffset, kEntryUnitLength));
                    if (sensorName.empty()) {
                        sensorName = std::format("Sensor {}", sensorIndex);
                    }
                    if (entryName.empty()) {
                        entryName = std::format("Reading {}", entryId);
                    }
                    if (unit.empty()) {
                        columns.push_back(std::format(
                            "HWiNFO::{}::{} (Entry {})", sensorName, entryName, i));
                    }
                    else {
                        columns.push_back(std::format(
                            "HWiNFO::{}::{} [{}] (Entry {})", sensorName, entryName, unit, i));
                    }
                }
            }

            const int64_t finalLastUpdate = ReadValue_<int64_t>(pView_, 12);
            if (finalLastUpdate != header.lastUpdate ||
                (!initializeColumns && entryKeys != entryKeys_)) {
                releaseMutex();
                return false;
            }

            std::chrono::steady_clock::duration initialAge{};
            if (initializeColumns && !TryGetInitialAge_(header.lastUpdate, initialAge)) {
                releaseMutex();
                return false;
            }

            if (initializeColumns) {
                sensorOffset_ = header.sensorOffset;
                sensorElementSize_ = header.sensorElementSize;
                sensorCount_ = header.sensorCount;
                entryOffset_ = header.entryOffset;
                entryElementSize_ = header.entryElementSize;
                entryCount_ = header.entryCount;
                entryKeys_ = std::move(entryKeys);
                columns_ = std::move(columns);
                lastChangeObserved_ = std::chrono::steady_clock::now() - initialAge;
            }
            else if (header.lastUpdate != lastUpdate_) {
                lastChangeObserved_ = std::chrono::steady_clock::now();
            }

            values_ = std::move(values);
            lastUpdate_ = header.lastUpdate;
            releaseMutex();
            return true;
        }
        catch (...) {
            releaseMutex();
            return false;
        }
    }
}
