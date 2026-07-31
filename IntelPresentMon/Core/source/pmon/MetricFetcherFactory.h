// Copyright (C) 2022 Intel Corporation
// SPDX-License-Identifier: MIT
#pragma once
#include <PresentMonAPIWrapper/PresentMonAPIWrapper.h>
#include "metric/MetricFetcher.h"
#include "metric/DynamicPollingFetcher.h"
#include "DynamicQuery.h"
#include "../kernel/OverlaySpec.h"
#include "../pmon/PresentMon.h"
#include <CommonUtilities/str/String.h>
#include <memory>
#include <vector>
#include <span>
#include <ranges>
#include <string>


namespace p2c::pmon
{
    namespace {
        namespace rn = std::ranges;
        namespace vi = std::views;
        using ::pmon::util::str::ToWide;
        using ::pmon::util::str::ToLower;

        inline const wchar_t* GetLocalizedStatDisplayName_(PM_STAT stat) noexcept
        {
            switch (stat) {
            case PM_STAT_AVG:
                return L"\u5E73\u5747";
            case PM_STAT_PERCENTILE_99:
                return L"99%";
            case PM_STAT_PERCENTILE_95:
                return L"95%";
            case PM_STAT_PERCENTILE_90:
                return L"90%";
            case PM_STAT_PERCENTILE_01:
                return L"1%";
            case PM_STAT_PERCENTILE_05:
                return L"5%";
            case PM_STAT_PERCENTILE_10:
                return L"10%";
            case PM_STAT_MAX:
                return L"\u6700\u5927";
            case PM_STAT_MIN:
                return L"\u6700\u5C0F";
            case PM_STAT_MID_LERP:
                return L"\u4E2D\u70B9\u63D2\u503C";
            case PM_STAT_NEWEST_POINT:
                return L"\u6700\u65B0";
            case PM_STAT_OLDEST_POINT:
                return L"\u6700\u65E9";
            case PM_STAT_COUNT:
                return L"\u8BA1\u6570";
            case PM_STAT_NON_ZERO_AVG:
                return L"\u975E\u96F6\u5E73\u5747";
            default:
                return nullptr;
            }
        }
    }

    class MetricFetcherFactory
    {
    public:
        // types
        struct BuildResult
        {
            struct FetcherPair
            {
                kern::QualifiedMetric qualifiedMetric;
                std::shared_ptr<met::MetricFetcher> pFetcher;
            };
            std::vector<FetcherPair> fetchers;
            std::shared_ptr<DynamicQuery> pQuery;
        };
        struct MetricInfo
        {
            std::wstring fullName;
            std::wstring unitLabel;
            bool isNonNumeric = true;
        };
        struct MetricLabelOptions
        {
            bool includeDeviceId = false;
            bool includeDeviceName = false;
        };
        // functions
        MetricFetcherFactory(pmon::PresentMon& pm)
            :
            pm_{ pm }
        {}
        // ** enumerate metrics reflection => introspect async endpoint
        MetricInfo GetMetricInfo(const kern::QualifiedMetric& qmet, MetricLabelOptions opts = {},
            const std::string& displayName = {}) const
        {
            MetricInfo info;

            auto& intro = pm_.GetIntrospectionRoot();
            const auto metric = intro.FindMetric((PM_METRIC)qmet.metricId);
            info.fullName = displayName.empty()
                ? ToWide(metric.Introspect().GetName())
                : ToWide(displayName);
            // find max array size among all devices with availability
            uint32_t arraySize = 0;
            for (auto&& dmi : metric.GetDeviceMetricInfo()) {
                if (!dmi.IsAvailable()) continue;
                arraySize = std::max(arraySize, dmi.GetArraySize());
            }
            // add [i] to end of metric name if it's an array metric
            if (arraySize > 1) {
                info.fullName += std::format(L" [{}]", qmet.arrayIndex);
            }
            if (opts.includeDeviceId) {
                info.fullName += std::format(L" <{}>", qmet.deviceId);
            }
            if (opts.includeDeviceName) {
                for (auto&& device : intro.GetDevices()) {
                    if (device.GetId() == qmet.deviceId) {
                        info.fullName += std::format(L" {{{}}}", ToWide(device.GetName()));
                        break;
                    }
                }
            }
            // add stat to name (but exclude midpoint (mpt)
            if (qmet.statId != PM_STAT_MID_POINT) {
                if (const auto pStatName = GetLocalizedStatDisplayName_((PM_STAT)qmet.statId)) {
                    info.fullName += std::format(L" ({})", pStatName);
                }
                else if (auto statAbbv = intro.FindEnumKey(PM_ENUM_STAT, qmet.statId).GetShortName(); !statAbbv.empty()) {
                    info.fullName += std::format(L" ({})", ToWide(statAbbv));
                }
            }
            // do a case insensitive compare to see if the metric starts with "ms" and if
            // so, remove it from the name
            std::wstring lowerFullName = ToLower(info.fullName);
            if (lowerFullName[0] == L'm' && lowerFullName[1] == L's') {
                info.fullName.erase(0, 2);
                size_t nonSpace = info.fullName.find_first_not_of(L" ");
                info.fullName.erase(0, nonSpace);
            }
            // add unit abbreviation to the end if metric is not dimensionless
            if (auto&& unit = metric.GetPreferredUnitHint(); unit != PM_UNIT_DIMENSIONLESS && unit) {
                info.unitLabel = ToWide(metric.IntrospectPreferredUnitHint().GetShortName());
            }
            const auto dataType = metric.GetDataTypeInfo().GetPolledType();
            info.isNonNumeric = dataType == PM_DATA_TYPE_ENUM || dataType == PM_DATA_TYPE_STRING;
            return info;
        }
        BuildResult Build(uint32_t pid, double winSizeMs, double metricOffsetMs, std::span<const kern::QualifiedMetric> qmets)
        {
            auto& intro = pm_.GetIntrospectionRoot();
            // construct query
            auto pQuery = std::make_shared<DynamicQuery>(pm_.GetSession(), winSizeMs, metricOffsetMs, qmets);
            // construct fetchers from filled query elements and return result
            const auto elements = pQuery->ExtractElements();
            BuildResult result;
            for (auto& e : elements) {
                result.fetchers.push_back({
                    .qualifiedMetric = kern::QualifiedMetric{
                        .metricId = e.metric,
                        .statId = e.stat,
                        .arrayIndex = e.arrayIndex,
                        .deviceId = e.deviceId,
                    },
                    .pFetcher = met::MakeDynamicPollingFetcher(e, intro, pQuery)
                });
            }
            result.pQuery = std::move(pQuery);
            return result;
        }
    private:
        pmon::PresentMon& pm_;
        // ** special metric container
    };
}
