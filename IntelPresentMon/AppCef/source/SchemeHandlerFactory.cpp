// Copyright (C) 2022 Intel Corporation
// SPDX-License-Identifier: MIT
#include "SchemeHandlerFactory.h"
#include "SchemeFileHandler.h"
#include <Core/source/infra/util/FolderResolver.h>
#include <include/cef_parser.h>
#include <format>
#include "util/Logging.h"

#ifdef NDEBUG
#define IS_DEBUG false
#else
#define IS_DEBUG true
#endif

namespace p2c::client::cef
{
    SchemeHandlerFactory::SchemeHandlerFactory(SchemeMode mode, bool hardFail, std::string localHost, std::string localPort, std::string webRoot)
        :
        baseDir_{ infra::util::FolderResolver::ResolveInstallPath() / "ipm-ui-vue" },
        mode_{ mode },
        hardFail_{ hardFail },
        localHost_{ std::move(localHost) },
        localPort_{ std::move(localPort) }
    {
        if (!webRoot.empty()) {
            baseDir_ = webRoot;
        }
    }

    // Return a new scheme handler instance to handle the request.
    CefRefPtr<CefResourceHandler> SchemeHandlerFactory::Create(
        CefRefPtr<CefBrowser> browser,
        CefRefPtr<CefFrame> frame,
        const CefString& scheme_name,
        CefRefPtr<CefRequest> request)
    {
        const auto DoErrorMessage = [&](const wchar_t* title, const wchar_t* body) {
            if (hardFail_) {
                MessageBoxW(
                    browser->GetHost()->GetWindowHandle(),
                    body, title,
                    MB_ICONERROR | MB_APPLMODAL
                );
            }
        };

        if (mode_ == SchemeMode::Web) {
            // anything goes if web mode
            // but don't worry about loading app files (only use default schema managers)
            pmlog_dbg(std::format("Processing request URL: {}", request->GetURL().ToString()));
            return nullptr;
        }
        else if (mode_ == SchemeMode::Local) {
            CefURLParts url_parts;
            if (!CefParseURL(request->GetURL(), url_parts)) {
                pmlog_error(std::format("Failed parsing URL: {}", request->GetURL().ToString())).no_trace();
                DoErrorMessage(L"URL \u9519\u8BEF", L"URL \u89E3\u6790\u5931\u8D25\uFF0C\u8BF7\u67E5\u770B\u65E5\u5FD7\u3002");
            }
            else if (CefString(&url_parts.host) != localHost_ && CefString(&url_parts.port) != localPort_) {
                if constexpr (IS_DEBUG) {
                    pmlog_warn(std::format("URL does not match dev endpoint: {}", request->GetURL().ToString()));
                }
                else {
                    pmlog_error(std::format("URL does not match dev endpoint: {}", request->GetURL().ToString())).no_trace();
                    DoErrorMessage(L"URL \u9519\u8BEF",
                        L"URL \u4E0E\u5F00\u53D1\u7AEF\u70B9\u4E0D\u5339\u914D\uFF0C\u8BF7\u67E5\u770B\u65E5\u5FD7\u3002");
                    std::terminate();
                }
            }
            else {
                pmlog_dbg(std::format("Processing request URL: {}", request->GetURL().ToString()));
            }
            return nullptr;
        }
        // otherwise mode is File (filesystem app assets) by default
        if (scheme_name == "https") {
            CefURLParts url_parts;
            if (!CefParseURL(request->GetURL(), url_parts)) {
                pmlog_error(std::format("Failed parsing URL: {}", request->GetURL().ToString())).no_trace();
                DoErrorMessage(L"URL \u9519\u8BEF", L"URL \u89E3\u6790\u5931\u8D25\uFF0C\u8BF7\u67E5\u770B\u65E5\u5FD7\u3002");
                return nullptr;
            }
            else if (const auto host = CefString(&url_parts.host); host != "app") {
                pmlog_error(std::format("Non-app domain in File mode: {}", request->GetURL().ToString())).no_trace();
                DoErrorMessage(L"URL \u9519\u8BEF",
                    L"\u6587\u4EF6\u6A21\u5F0F\u4E0D\u652F\u6301\u975E app \u57DF\u540D\uFF0C\u8BF7\u67E5\u770B\u65E5\u5FD7\u3002");
                return nullptr;
            }
            else {
                pmlog_dbg(std::format("Processing request URL: {}", request->GetURL().ToString()));
            }
            return new SchemeFileHandler(baseDir_);
        }
        // any other scheme in File mode is an error
        else {
            pmlog_error(std::format("Wrong scheme for File mode: {}", request->GetURL().ToString())).no_trace();
            DoErrorMessage(L"URL \u9519\u8BEF",
                L"\u6587\u4EF6\u6A21\u5F0F\u4F7F\u7528\u4E86\u9519\u8BEF\u7684 URL \u65B9\u6848\uFF0C\u8BF7\u67E5\u770B\u65E5\u5FD7\u3002");
            return new SchemeFileHandler(baseDir_);
        }
    }
}
