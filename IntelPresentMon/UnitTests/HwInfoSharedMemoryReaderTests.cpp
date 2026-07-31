// Copyright (C) 2026 Raymond-Leung7
// SPDX-License-Identifier: MIT
#include <CppUnitTest.h>
#include <Core/source/pmon/HwInfoSharedMemoryReader.h>
#include <Windows.h>
#include <algorithm>
#include <chrono>
#include <cstring>
#include <format>
#include <stdexcept>
#include <string_view>
#include <utility>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace HwInfoSharedMemoryReaderTests
{
    namespace
    {
        constexpr size_t kMappingSize = 4096;
        constexpr uint32_t kSensorOffset = 44;
        constexpr uint32_t kSensorSize = 264;
        constexpr uint32_t kEntryOffset = kSensorOffset + kSensorSize;
        constexpr uint32_t kEntrySize = 316;

        int64_t CurrentUnixTime_()
        {
            return std::chrono::duration_cast<std::chrono::seconds>(
                std::chrono::system_clock::now().time_since_epoch()).count();
        }

        class TestMapping_
        {
        public:
            explicit TestMapping_(std::wstring name) : name_{ std::move(name) }
            {
                hMapping_ = CreateFileMappingW(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE,
                    0, (DWORD)kMappingSize, name_.c_str());
                if (!hMapping_) {
                    throw std::runtime_error{ "CreateFileMappingW failed" };
                }
                pData_ = reinterpret_cast<uint8_t*>(MapViewOfFile(
                    hMapping_, FILE_MAP_ALL_ACCESS, 0, 0, kMappingSize));
                if (!pData_) {
                    CloseHandle(hMapping_);
                    hMapping_ = nullptr;
                    throw std::runtime_error{ "MapViewOfFile failed" };
                }
                std::memset(pData_, 0, kMappingSize);
            }

            ~TestMapping_()
            {
                if (pData_) {
                    UnmapViewOfFile(pData_);
                }
                if (hMapping_) {
                    CloseHandle(hMapping_);
                }
            }

            TestMapping_(const TestMapping_&) = delete;
            TestMapping_& operator=(const TestMapping_&) = delete;

            TestMapping_(TestMapping_&& other) noexcept
                : name_{ std::move(other.name_) },
                hMapping_{ std::exchange(other.hMapping_, nullptr) },
                pData_{ std::exchange(other.pData_, nullptr) }
            {}

            TestMapping_& operator=(TestMapping_&&) = delete;

            const std::wstring& Name() const noexcept
            {
                return name_;
            }

            template<typename T>
            void Write(size_t offset, T value)
            {
                std::memcpy(pData_ + offset, &value, sizeof(value));
            }

            void WriteString(size_t offset, std::string_view value, size_t capacity)
            {
                const size_t length = (std::min)(value.size(), capacity - 1);
                std::memcpy(pData_ + offset, value.data(), length);
                pData_[offset + length] = 0;
            }

        private:
            std::wstring name_;
            HANDLE hMapping_ = nullptr;
            uint8_t* pData_ = nullptr;
        };

        TestMapping_ MakeValidMapping_(std::wstring suffix)
        {
            TestMapping_ mapping{ std::format(
                L"Local\\PresentMon-HwInfoReaderTest-{}-{}", GetCurrentProcessId(), suffix) };
            mapping.Write<uint32_t>(0, 0x53695748u);
            mapping.Write<uint32_t>(4, 1);
            mapping.Write<uint32_t>(8, 1);
            mapping.Write<int64_t>(12, CurrentUnixTime_());
            mapping.Write<uint32_t>(20, kSensorOffset);
            mapping.Write<uint32_t>(24, kSensorSize);
            mapping.Write<uint32_t>(28, 1);
            mapping.Write<uint32_t>(32, kEntryOffset);
            mapping.Write<uint32_t>(36, kEntrySize);
            mapping.Write<uint32_t>(40, 2);

            mapping.Write<uint32_t>(kSensorOffset, 7);
            mapping.Write<uint32_t>(kSensorOffset + 4, 0);
            mapping.WriteString(kSensorOffset + 8, "CPU [#0]", 128);

            mapping.Write<uint32_t>(kEntryOffset, 1);
            mapping.Write<uint32_t>(kEntryOffset + 4, 0);
            mapping.Write<uint32_t>(kEntryOffset + 8, 101);
            mapping.WriteString(kEntryOffset + 12, "CPU Package", 128);
            mapping.WriteString(kEntryOffset + 268, "C", 16);
            mapping.Write<double>(kEntryOffset + 284, 72.5);

            const size_t secondEntry = kEntryOffset + kEntrySize;
            mapping.Write<uint32_t>(secondEntry, 5);
            mapping.Write<uint32_t>(secondEntry + 4, 0);
            mapping.Write<uint32_t>(secondEntry + 8, 102);
            mapping.WriteString(secondEntry + 12, "CPU Package Power", 128);
            mapping.WriteString(secondEntry + 268, "W", 16);
            mapping.Write<double>(secondEntry + 284, 45.25);
            return mapping;
        }
    }

    TEST_CLASS(SharedMemoryReaderTests)
    {
    public:
        TEST_METHOD(OpensAndRefreshesValidSnapshot)
        {
            auto mapping = MakeValidMapping_(L"valid");
            p2c::pmon::HwInfoSharedMemoryReader reader{ mapping.Name(), L"" };

            Assert::IsTrue(reader.Open());
            Assert::IsTrue(reader.IsOpen());
            Assert::AreEqual((size_t)2, reader.Columns().size());
            Assert::AreEqual((size_t)2, reader.Values().size());
            Assert::IsTrue(reader.Columns()[0].contains("CPU Package"));
            Assert::IsTrue(reader.Columns()[1].contains("CPU Package Power"));
            Assert::AreEqual(72.5, reader.Values()[0], 0.001);
            Assert::AreEqual(45.25, reader.Values()[1], 0.001);
            const auto initialTime = reader.LastUpdate();
            Assert::IsTrue(initialTime > 0);

            mapping.Write<int64_t>(12, initialTime + 2);
            mapping.Write<double>(kEntryOffset + 284, 73.25);
            Assert::IsTrue(reader.Refresh());
            Assert::AreEqual(73.25, reader.Values()[0], 0.001);
            Assert::AreEqual(initialTime + 2, reader.LastUpdate());
            Assert::IsTrue(reader.SampleAgeMs() >= 0);
        }

        TEST_METHOD(RejectsOutOfBoundsLayout)
        {
            auto mapping = MakeValidMapping_(L"invalid");
            mapping.Write<uint32_t>(32, 0xFFFFFFF0u);
            p2c::pmon::HwInfoSharedMemoryReader reader{ mapping.Name(), L"" };

            Assert::IsFalse(reader.Open());
            Assert::IsFalse(reader.IsOpen());
        }

        TEST_METHOD(PrefersVersionTwoUtf8Names)
        {
            TestMapping_ mapping{ std::format(
                L"Local\\PresentMon-HwInfoReaderTest-{}-v2", GetCurrentProcessId()) };
            constexpr uint32_t sensorSize = 392;
            constexpr uint32_t entryOffset = kSensorOffset + sensorSize;
            constexpr uint32_t entrySize = 460;
            mapping.Write<uint32_t>(0, 0x53695748u);
            mapping.Write<uint32_t>(4, 2);
            mapping.Write<uint32_t>(8, 1);
            mapping.Write<int64_t>(12, CurrentUnixTime_());
            mapping.Write<uint32_t>(20, kSensorOffset);
            mapping.Write<uint32_t>(24, sensorSize);
            mapping.Write<uint32_t>(28, 1);
            mapping.Write<uint32_t>(32, entryOffset);
            mapping.Write<uint32_t>(36, entrySize);
            mapping.Write<uint32_t>(40, 1);
            mapping.WriteString(kSensorOffset + 8, "Original Sensor", 128);
            mapping.WriteString(kSensorOffset + 136, "Legacy Sensor", 128);
            mapping.WriteString(kSensorOffset + 264, "UTF8 Sensor", 128);
            mapping.Write<uint32_t>(entryOffset + 4, 0);
            mapping.Write<uint32_t>(entryOffset + 8, 201);
            mapping.WriteString(entryOffset + 12, "Original Reading", 128);
            mapping.WriteString(entryOffset + 140, "Legacy Reading", 128);
            mapping.WriteString(entryOffset + 268, "Old", 16);
            mapping.Write<double>(entryOffset + 284, 12.5);
            mapping.WriteString(entryOffset + 316, "UTF8 Reading", 128);
            mapping.WriteString(entryOffset + 444, "New", 16);

            p2c::pmon::HwInfoSharedMemoryReader reader{ mapping.Name(), L"" };
            Assert::IsTrue(reader.Open());
            Assert::AreEqual((size_t)1, reader.Columns().size());
            Assert::IsTrue(reader.Columns()[0].contains("UTF8 Sensor"));
            Assert::IsTrue(reader.Columns()[0].contains("UTF8 Reading"));
            Assert::IsTrue(reader.Columns()[0].contains("[New]"));
            Assert::AreEqual(12.5, reader.Values()[0], 0.001);
        }

        TEST_METHOD(RejectsStaleInitialSnapshot)
        {
            auto mapping = MakeValidMapping_(L"stale");
            mapping.Write<int64_t>(12, CurrentUnixTime_() - 31);
            p2c::pmon::HwInfoSharedMemoryReader reader{ mapping.Name(), L"" };

            Assert::IsFalse(reader.Open());
        }

        TEST_METHOD(RejectsUnsupportedFutureVersion)
        {
            auto mapping = MakeValidMapping_(L"future-version");
            mapping.Write<uint32_t>(4, 3);
            p2c::pmon::HwInfoSharedMemoryReader reader{ mapping.Name(), L"" };

            Assert::IsFalse(reader.Open());
        }

        TEST_METHOD(DetectsSensorIdentityChange)
        {
            auto mapping = MakeValidMapping_(L"identity-change");
            p2c::pmon::HwInfoSharedMemoryReader reader{ mapping.Name(), L"" };
            Assert::IsTrue(reader.Open());

            mapping.Write<uint32_t>(kSensorOffset, 8);
            mapping.Write<int64_t>(12, reader.LastUpdate() + 1);
            Assert::IsFalse(reader.Refresh());
        }

        TEST_METHOD(ReadsLiveSnapshotWhenAvailable)
        {
            p2c::pmon::HwInfoSharedMemoryReader reader;
            if (!reader.Open()) {
                return;
            }

            Assert::IsFalse(reader.Columns().empty());
            Assert::AreEqual(reader.Columns().size(), reader.Values().size());
            Assert::IsTrue(reader.LastUpdate() > 0);
            Assert::IsTrue(reader.Refresh());
        }
    };
}
