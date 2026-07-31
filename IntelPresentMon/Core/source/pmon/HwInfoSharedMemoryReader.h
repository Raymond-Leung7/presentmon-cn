// Copyright (C) 2026 Raymond-Leung7
// SPDX-License-Identifier: MIT
#pragma once
#include <Windows.h>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace p2c::pmon
{
    class HwInfoSharedMemoryReader
    {
    public:
        HwInfoSharedMemoryReader();
        HwInfoSharedMemoryReader(std::wstring mappingName, std::wstring mutexName);
        ~HwInfoSharedMemoryReader();
        HwInfoSharedMemoryReader(const HwInfoSharedMemoryReader&) = delete;
        HwInfoSharedMemoryReader& operator=(const HwInfoSharedMemoryReader&) = delete;

        bool Open() noexcept;
        bool Refresh() noexcept;
        const std::vector<std::string>& Columns() const noexcept;
        const std::vector<double>& Values() const noexcept;
        int64_t LastUpdate() const noexcept;
        int64_t SampleAgeMs() const noexcept;
        bool IsOpen() const noexcept;

    private:
        struct EntryKey_
        {
            uint32_t sensorId = 0;
            uint32_t sensorInstance = 0;
            uint32_t readingType = 0;
            uint32_t entryId = 0;

            bool operator==(const EntryKey_&) const = default;
        };

        void Close_() noexcept;
        bool ReadSnapshot_(bool initializeColumns) noexcept;

        HANDLE hMapping_ = nullptr;
        HANDLE hMutex_ = nullptr;
        const uint8_t* pView_ = nullptr;
        size_t viewSize_ = 0;
        uint32_t sensorOffset_ = 0;
        uint32_t sensorElementSize_ = 0;
        uint32_t sensorCount_ = 0;
        uint32_t entryOffset_ = 0;
        uint32_t entryElementSize_ = 0;
        uint32_t entryCount_ = 0;
        std::vector<EntryKey_> entryKeys_;
        std::vector<std::string> columns_;
        std::vector<double> values_;
        int64_t lastUpdate_ = 0;
        std::chrono::steady_clock::time_point lastChangeObserved_{};
        bool isOpen_ = false;
        std::wstring mappingName_;
        std::wstring mutexName_;
    };
}
