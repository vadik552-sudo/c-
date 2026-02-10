#include "WinHttpClient.h"

#include <windows.h>
#include <winhttp.h>

#include <sstream>
#include <vector>

#pragma comment(lib, "winhttp.lib")

namespace {

std::wstring LastErrorToString(DWORD errorCode) {
    LPWSTR messageBuffer = nullptr;
    const DWORD flags = FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS;
    const DWORD size = FormatMessageW(flags, nullptr, errorCode, 0, reinterpret_cast<LPWSTR>(&messageBuffer), 0, nullptr);
    std::wstring message;
    if (size > 0 && messageBuffer != nullptr) {
        message.assign(messageBuffer, size);
        LocalFree(messageBuffer);
    } else {
        std::wstringstream stream;
        stream << L"WinAPI error " << errorCode;
        message = stream.str();
    }
    return message;
}

std::wstring Utf8ToUtf16(const std::string& value) {
    if (value.empty()) {
        return std::wstring();
    }
    const int required = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, value.data(), static_cast<int>(value.size()), nullptr, 0);
    if (required <= 0) {
        return std::wstring();
    }
    std::wstring output(static_cast<size_t>(required), L'\0');
    const int written = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, value.data(), static_cast<int>(value.size()), output.data(), required);
    if (written <= 0) {
        return std::wstring();
    }
    return output;
}

std::wstring BytesToUtf16Fallback(const std::string& value) {
    if (value.empty()) {
        return std::wstring();
    }
    std::wstring output;
    output.reserve(value.size());
    for (unsigned char ch : value) {
        output.push_back(static_cast<wchar_t>(ch));
    }
    return output;
}

class WinHttpHandle {
public:
    WinHttpHandle() noexcept : handle_(nullptr) {}
    explicit WinHttpHandle(HINTERNET handle) noexcept : handle_(handle) {}
    ~WinHttpHandle() {
        Reset();
    }

    WinHttpHandle(const WinHttpHandle&) = delete;
    WinHttpHandle& operator=(const WinHttpHandle&) = delete;

    WinHttpHandle(WinHttpHandle&& other) noexcept : handle_(other.handle_) {
        other.handle_ = nullptr;
    }

    WinHttpHandle& operator=(WinHttpHandle&& other) noexcept {
        if (this != &other) {
            Reset();
            handle_ = other.handle_;
            other.handle_ = nullptr;
        }
        return *this;
    }

    HINTERNET Get() const noexcept {
        return handle_;
    }

    bool IsValid() const noexcept {
        return handle_ != nullptr;
    }

private:
    void Reset() noexcept {
        if (handle_ != nullptr) {
            WinHttpCloseHandle(handle_);
            handle_ = nullptr;
        }
    }

    HINTERNET handle_;
};

}  // namespace

HttpResult PerformHttpGet(const std::wstring& url, unsigned int timeoutSeconds) {
    HttpResult result{};
    result.success = false;
    result.statusCode = 0;

    URL_COMPONENTS components{};
    components.dwStructSize = sizeof(components);
    components.dwSchemeLength = static_cast<DWORD>(-1);
    components.dwHostNameLength = static_cast<DWORD>(-1);
    components.dwUrlPathLength = static_cast<DWORD>(-1);
    components.dwExtraInfoLength = static_cast<DWORD>(-1);

    if (!WinHttpCrackUrl(url.c_str(), static_cast<DWORD>(url.size()), 0, &components)) {
        result.error = L"Не удалось разобрать URL: " + LastErrorToString(GetLastError());
        return result;
    }

    std::wstring host(components.lpszHostName, components.dwHostNameLength);
    std::wstring path(components.lpszUrlPath ? components.lpszUrlPath : L"", components.dwUrlPathLength);
    std::wstring extra(components.lpszExtraInfo ? components.lpszExtraInfo : L"", components.dwExtraInfoLength);
    std::wstring resource = path + extra;
    if (resource.empty()) {
        resource = L"/";
    }

    const bool secure = components.nScheme == INTERNET_SCHEME_HTTPS;
    const unsigned int timeoutMs = timeoutSeconds > 0 ? timeoutSeconds * 1000U : 30000U;

    WinHttpHandle session(WinHttpOpen(L"KaspiKassaAPIv3/1.0", WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY,
                                      WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0));
    if (!session.IsValid()) {
        result.error = L"Не удалось создать WinHTTP сессию: " + LastErrorToString(GetLastError());
        return result;
    }

    if (!WinHttpSetTimeouts(session.Get(), static_cast<int>(timeoutMs), static_cast<int>(timeoutMs), static_cast<int>(timeoutMs), static_cast<int>(timeoutMs))) {
        result.error = L"Не удалось установить таймауты: " + LastErrorToString(GetLastError());
        return result;
    }

    WinHttpHandle connect(WinHttpConnect(session.Get(), host.c_str(), components.nPort, 0));
    if (!connect.IsValid()) {
        result.error = L"Не удалось подключиться к хосту: " + LastErrorToString(GetLastError());
        return result;
    }

    DWORD flags = secure ? WINHTTP_FLAG_SECURE : 0;
    WinHttpHandle request(WinHttpOpenRequest(connect.Get(), L"GET", resource.c_str(), nullptr,
                                             WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, flags));
    if (!request.IsValid()) {
        result.error = L"Не удалось создать HTTP запрос: " + LastErrorToString(GetLastError());
        return result;
    }

    if (!WinHttpSendRequest(request.Get(), WINHTTP_NO_ADDITIONAL_HEADERS, 0,
                            WINHTTP_NO_REQUEST_DATA, 0, 0, 0)) {
        result.error = L"Ошибка отправки HTTP запроса: " + LastErrorToString(GetLastError());
        return result;
    }

    if (!WinHttpReceiveResponse(request.Get(), nullptr)) {
        result.error = L"Ошибка получения HTTP ответа: " + LastErrorToString(GetLastError());
        return result;
    }

    DWORD statusCode = 0;
    DWORD statusCodeSize = sizeof(statusCode);
    if (!WinHttpQueryHeaders(request.Get(), WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
                             WINHTTP_HEADER_NAME_BY_INDEX, &statusCode, &statusCodeSize, WINHTTP_NO_HEADER_INDEX)) {
        result.error = L"Не удалось прочитать HTTP код ответа: " + LastErrorToString(GetLastError());
        return result;
    }
    result.statusCode = statusCode;

    std::string responseBytes;
    for (;;) {
        DWORD available = 0;
        if (!WinHttpQueryDataAvailable(request.Get(), &available)) {
            result.error = L"Ошибка WinHttpQueryDataAvailable: " + LastErrorToString(GetLastError());
            return result;
        }
        if (available == 0) {
            break;
        }

        std::vector<char> buffer(available);
        DWORD read = 0;
        if (!WinHttpReadData(request.Get(), buffer.data(), available, &read)) {
            result.error = L"Ошибка WinHttpReadData: " + LastErrorToString(GetLastError());
            return result;
        }
        responseBytes.append(buffer.data(), buffer.data() + read);
    }

    std::wstring utf16 = Utf8ToUtf16(responseBytes);
    if (utf16.empty() && !responseBytes.empty()) {
        utf16 = BytesToUtf16Fallback(responseBytes);
    }
    result.body = std::move(utf16);
    result.success = true;
    return result;
}
