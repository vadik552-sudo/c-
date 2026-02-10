#pragma once

#include <string>

struct HttpResult {
    bool success;
    unsigned long statusCode;
    std::wstring body;
    std::wstring error;
};

HttpResult PerformHttpGet(const std::wstring& url, unsigned int timeoutSeconds);
