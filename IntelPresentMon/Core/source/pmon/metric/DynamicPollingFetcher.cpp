#include "DynamicPollingFetcher.h"
#include <PresentMonAPIWrapper/PresentMonAPIWrapper.h>
#include <CommonUtilities//str/String.h>
#include <ranges>
#include <string_view>

namespace p2c::pmon::met
{
    using ::pmon::util::str::ToWide;

    namespace
    {
        std::wstring LocalizeEnumDisplayName_(PM_ENUM enumId, int key, std::wstring_view fallback)
        {
            switch (enumId) {
            case PM_ENUM_DEVICE_VENDOR:
                if ((PM_DEVICE_VENDOR)key == PM_DEVICE_VENDOR_UNKNOWN) {
                    return L"\u672A\u77E5";
                }
                break;
            case PM_ENUM_PRESENT_MODE:
                switch ((PM_PRESENT_MODE)key) {
                case PM_PRESENT_MODE_UNKNOWN:
                    return L"\u672A\u77E5";
                case PM_PRESENT_MODE_HARDWARE_LEGACY_FLIP:
                    return L"\u786C\u4EF6\uFF1A\u4F20\u7EDF\u7FFB\u8F6C";
                case PM_PRESENT_MODE_HARDWARE_LEGACY_COPY_TO_FRONT_BUFFER:
                    return L"\u786C\u4EF6\uFF1A\u4F20\u7EDF\u590D\u5236\u5230\u524D\u7F13\u51B2\u533A";
                case PM_PRESENT_MODE_HARDWARE_INDEPENDENT_FLIP:
                    return L"\u786C\u4EF6\uFF1A\u72EC\u7ACB\u7FFB\u8F6C";
                case PM_PRESENT_MODE_COMPOSED_FLIP:
                    return L"\u5408\u6210\uFF1A\u7FFB\u8F6C";
                case PM_PRESENT_MODE_COMPOSED_COPY_WITH_GPU_GDI:
                    return L"\u5408\u6210\uFF1A\u4F7F\u7528 GPU GDI \u590D\u5236";
                case PM_PRESENT_MODE_COMPOSED_COPY_WITH_CPU_GDI:
                    return L"\u5408\u6210\uFF1A\u4F7F\u7528 CPU GDI \u590D\u5236";
                case PM_PRESENT_MODE_HARDWARE_COMPOSED_INDEPENDENT_FLIP:
                    return L"\u786C\u4EF6\u5408\u6210\uFF1A\u72EC\u7ACB\u7FFB\u8F6C";
                }
                break;
            case PM_ENUM_GRAPHICS_RUNTIME:
                if ((PM_GRAPHICS_RUNTIME)key == PM_GRAPHICS_RUNTIME_UNKNOWN) {
                    return L"\u672A\u77E5";
                }
                break;
            case PM_ENUM_FRAME_TYPE:
                switch ((PM_FRAME_TYPE)key) {
                case PM_FRAME_TYPE_NOT_SET:
                case PM_FRAME_TYPE_APPLICATION:
                case PM_FRAME_TYPE_REPEATED:
                    return L"\u5E94\u7528\u7A0B\u5E8F";
                case PM_FRAME_TYPE_UNSPECIFIED:
                    return L"\u672A\u6307\u5B9A";
                }
                break;
            }
            return std::wstring{ fallback };
        }
    }


    DynamicPollingFetcher::DynamicPollingFetcher(const PM_QUERY_ELEMENT& qel, const pmapi::intro::Root& introRoot,
        std::shared_ptr<DynamicQuery> pQuery)
        :
        pQuery_{ std::move(pQuery) },
        offset_{ (uint32_t)qel.dataOffset }
    {
        // overlay will always indicate preferred unit in the widget labels
        // so we must scale from output unit if necessary to match
        const auto metric = introRoot.FindMetric(qel.metric);
        const auto type = metric.GetDataTypeInfo().GetPolledType();
        if (metric.GetUnit() != metric.GetPreferredUnitHint()) {
            scale_ = (float)introRoot.FindUnit(metric.GetUnit())
                .MakeConversionFactor(metric.GetPreferredUnitHint());
        }
    }

    TypedDynamicPollingFetcher<PM_ENUM>::TypedDynamicPollingFetcher(const PM_QUERY_ELEMENT& qel, const pmapi::intro::Root& introRoot,
        std::shared_ptr<DynamicQuery> pQuery, PM_ENUM enumId,
        std::shared_ptr<const pmapi::EnumMap::KeyMap> pKeyMap)
        :
        DynamicPollingFetcher{ qel, introRoot, std::move(pQuery) },
        enumId_{ enumId },
        pKeyMap_{ std::move(pKeyMap) }
    {}
    std::wstring TypedDynamicPollingFetcher<PM_ENUM>::ReadStringValue()
    {
        if (auto pBlobBytes = pQuery_->GetBlobData()) {
            const int key = *reinterpret_cast<const int*>(&pBlobBytes[offset_]);
            return LocalizeEnumDisplayName_(enumId_, key, pKeyMap_->at(key).wideName);
        }
        return {};
    }

    std::optional<float> TypedDynamicPollingFetcher<PM_ENUM>::ReadValue()
    {
        return {};
    }

    std::shared_ptr<DynamicPollingFetcher> MakeDynamicPollingFetcher(const PM_QUERY_ELEMENT& qel,
        const pmapi::intro::Root& introRoot, std::shared_ptr<DynamicQuery> pQuery)
    {
        const auto dataTypeInfo = introRoot.FindMetric(qel.metric).GetDataTypeInfo();
        switch (dataTypeInfo.GetPolledType()) {
        case PM_DATA_TYPE_BOOL:
            return std::make_shared<TypedDynamicPollingFetcher<bool>>(qel, introRoot, std::move(pQuery));
        case PM_DATA_TYPE_INT32:
            return std::make_unique<TypedDynamicPollingFetcher<int32_t>>(qel, introRoot, std::move(pQuery));
        case PM_DATA_TYPE_UINT32:
            return std::make_unique<TypedDynamicPollingFetcher<uint32_t>>(qel, introRoot, std::move(pQuery));
        case PM_DATA_TYPE_UINT64:
            return std::make_unique<TypedDynamicPollingFetcher<uint64_t>>(qel, introRoot, std::move(pQuery));
        case PM_DATA_TYPE_DOUBLE:
            return std::make_unique<TypedDynamicPollingFetcher<double>>(qel, introRoot, std::move(pQuery));
        case PM_DATA_TYPE_STRING:
            return std::make_unique<TypedDynamicPollingFetcher<const char*>>(qel, introRoot, std::move(pQuery));
        case PM_DATA_TYPE_ENUM: {
            const auto enumId = dataTypeInfo.GetEnumId();
            return std::make_unique<TypedDynamicPollingFetcher<PM_ENUM>>(qel, introRoot, std::move(pQuery),
                enumId, pmapi::EnumMap::GetKeyMap(enumId));
        }
        }
        // TODO: maybe throw exception here?
        return {};
    }
}
