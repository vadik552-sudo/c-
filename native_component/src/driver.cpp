#include "driver.h"

#include <Windows.h>
#include <Shellapi.h>
#include <winhttp.h>

#include <algorithm>
#include <array>
#include <fstream>

#pragma comment(lib, "winhttp.lib")

namespace {

using std::size_t;

struct MethodDef {
    const char16_t* en;
    long params;
    bool hasRet;
};

constexpr std::array<MethodDef, static_cast<size_t>(MethodId::Count)> kMethods = {{
    {u"GetVersion", 0, true},
    {u"GetDescription", 0, true},
    {u"GetParameters", 0, true},
    {u"SetParameter", 2, true},
    {u"Open", 0, true},
    {u"Close", 0, true},
    {u"DeviceTest", 0, true},
    {u"GetAdditionalActions", 0, true},
    {u"DoAdditionalAction", 1, true},
    {u"GetLastError", 0, true},
}};

std::u16string FromWcharT(const WCHAR_T* src) {
    if (src == nullptr) {
        return {};
    }
    std::u16string out;
    while (*src != 0) {
        out.push_back(static_cast<char16_t>(*src));
        ++src;
    }
    return out;
}

}  // namespace

KaspiKassaDriver::KaspiKassaDriver() {
    wchar_t tempPath[MAX_PATH] = {};
    const DWORD len = GetTempPathW(MAX_PATH, tempPath);
    if (len > 0) {
        logPath_ = std::wstring(tempPath) + L"KaspiKassaAPIv3_driver.log";
    } else {
        logPath_ = L"KaspiKassaAPIv3_driver.log";
    }
}

KaspiKassaDriver::~KaspiKassaDriver() = default;

const char16_t* KaspiKassaDriver::ClassName() {
    return u"KaspiKassaAPIv3";
}

bool_t ADDIN_STDCALL KaspiKassaDriver::Init(void* pConnection) {
    connection_ = pConnection;
    Log(L"Init completed");
    return 1;
}

bool_t ADDIN_STDCALL KaspiKassaDriver::setMemManager(void* pMemMgr) {
    memMgr_ = static_cast<IMemoryManager*>(pMemMgr);
    return memMgr_ != nullptr ? 1 : 0;
}

long ADDIN_STDCALL KaspiKassaDriver::GetInfo() {
    return APP_CAP_NATIVE;
}

void ADDIN_STDCALL KaspiKassaDriver::Done() {
    opened_ = false;
    connection_ = nullptr;
    Log(L"Done called");
}

bool_t ADDIN_STDCALL KaspiKassaDriver::RegisterExtensionAs(WCHAR_T** wsExtensionName) {
    if (wsExtensionName == nullptr || memMgr_ == nullptr) {
        return 0;
    }
    void* mem = nullptr;
    const std::u16string value = std::u16string(ClassName());
    const std::uint32_t bytes = static_cast<std::uint32_t>((value.size() + 1) * sizeof(WCHAR_T));
    if (!memMgr_->AllocMemory(&mem, bytes) || mem == nullptr) {
        return 0;
    }
    auto* out = static_cast<WCHAR_T*>(mem);
    for (size_t i = 0; i < value.size(); ++i) {
        out[i] = static_cast<WCHAR_T>(value[i]);
    }
    out[value.size()] = 0;
    *wsExtensionName = out;
    return 1;
}

long ADDIN_STDCALL KaspiKassaDriver::GetNProps() { return 0; }
long ADDIN_STDCALL KaspiKassaDriver::FindProp(const WCHAR_T*) { return -1; }
const WCHAR_T* ADDIN_STDCALL KaspiKassaDriver::GetPropName(long, long) { return nullptr; }
bool_t ADDIN_STDCALL KaspiKassaDriver::GetPropVal(long, tVariant*) { return 0; }
bool_t ADDIN_STDCALL KaspiKassaDriver::SetPropVal(long, tVariant*) { return 0; }
bool_t ADDIN_STDCALL KaspiKassaDriver::IsPropReadable(long) { return 0; }
bool_t ADDIN_STDCALL KaspiKassaDriver::IsPropWritable(long) { return 0; }

long ADDIN_STDCALL KaspiKassaDriver::GetNMethods() {
    return static_cast<long>(MethodId::Count);
}

long ADDIN_STDCALL KaspiKassaDriver::FindMethod(const WCHAR_T* wsMethodName) {
    if (wsMethodName == nullptr) {
        return -1;
    }
    const std::u16string name = ToLower(FromWcharT(wsMethodName));
    for (long i = 0; i < static_cast<long>(MethodId::Count); ++i) {
        if (name == ToLower(std::u16string(kMethods[static_cast<size_t>(i)].en))) {
            return i;
        }
    }
    return -1;
}

const WCHAR_T* ADDIN_STDCALL KaspiKassaDriver::GetMethodName(long methodNum, long) {
    if (methodNum < 0 || methodNum >= static_cast<long>(MethodId::Count)) {
        return nullptr;
    }
    return reinterpret_cast<const WCHAR_T*>(kMethods[static_cast<size_t>(methodNum)].en);
}

long ADDIN_STDCALL KaspiKassaDriver::GetNParams(long methodNum) {
    if (methodNum < 0 || methodNum >= static_cast<long>(MethodId::Count)) {
        return 0;
    }
    return kMethods[static_cast<size_t>(methodNum)].params;
}

bool_t ADDIN_STDCALL KaspiKassaDriver::GetParamDefValue(long, long, tVariant*) { return 0; }

bool_t ADDIN_STDCALL KaspiKassaDriver::HasRetVal(long methodNum) {
    if (methodNum < 0 || methodNum >= static_cast<long>(MethodId::Count)) {
        return 0;
    }
    return kMethods[static_cast<size_t>(methodNum)].hasRet ? 1 : 0;
}

bool_t ADDIN_STDCALL KaspiKassaDriver::CallAsProc(long methodNum, tVariant* paParams, long sizeArray) {
    if (methodNum == static_cast<long>(MethodId::SetParameter)) {
        if (sizeArray != 2 || paParams == nullptr) {
            SetLastError(u"SetParameter expects 2 parameters");
            return 0;
        }
        std::u16string name;
        if (!ReadVariantString(paParams[0], &name, u"SetParameter.name")) {
            return 0;
        }
        return SetParameterInternal(name, paParams[1]) ? 1 : 0;
    }
    if (methodNum == static_cast<long>(MethodId::Close)) {
        opened_ = false;
        return 1;
    }
    return 0;
}

bool_t ADDIN_STDCALL KaspiKassaDriver::CallAsFunc(long methodNum, tVariant* pvarRetValue, tVariant* paParams, long sizeArray) {
    if (pvarRetValue == nullptr) {
        return 0;
    }

    switch (static_cast<MethodId>(methodNum)) {
        case MethodId::GetVersion:
            return SetVariantString(pvarRetValue, u"1.0.0.1") ? 1 : 0;
        case MethodId::GetDescription:
            return SetVariantString(pvarRetValue, u"KaspiKassaAPIv3 NativeAPI driver for 1C") ? 1 : 0;
        case MethodId::GetParameters:
            return SetVariantString(
                pvarRetValue,
                u"<Settings><Page Name=\"Main\"><Group Name=\"Connection\"><Parameter Name=\"BaseURL\" Type=\"String\"/>"
                u"<Parameter Name=\"Token\" Type=\"String\"/><Parameter Name=\"PointId\" Type=\"String\"/>"
                u"<Parameter Name=\"TimeoutMs\" Type=\"Number\"/><Parameter Name=\"PollIntervalMs\" Type=\"Number\"/>"
                u"<Parameter Name=\"VerboseLog\" Type=\"Boolean\"/></Group></Page></Settings>")
                       ? 1
                       : 0;
        case MethodId::SetParameter: {
            if (sizeArray != 2 || paParams == nullptr) {
                SetLastError(u"SetParameter expects 2 parameters");
                return SetVariantBool(pvarRetValue, false) ? 1 : 0;
            }
            std::u16string name;
            if (!ReadVariantString(paParams[0], &name, u"SetParameter.name")) {
                return SetVariantBool(pvarRetValue, false) ? 1 : 0;
            }
            return SetVariantBool(pvarRetValue, SetParameterInternal(name, paParams[1])) ? 1 : 0;
        }
        case MethodId::Open:
            opened_ = true;
            SetLastError(u"");
            return SetVariantBool(pvarRetValue, true) ? 1 : 0;
        case MethodId::Close:
            opened_ = false;
            return SetVariantBool(pvarRetValue, true) ? 1 : 0;
        case MethodId::DeviceTest:
            return SetVariantBool(pvarRetValue, HttpGetBaseUrl()) ? 1 : 0;
        case MethodId::GetAdditionalActions:
            return SetVariantString(
                pvarRetValue,
                u"<Actions><Action Name=\"TestConnection\" Caption=\"Проверить подключение\"/>"
                u"<Action Name=\"ShowLastLog\" Caption=\"Показать последний лог\"/></Actions>")
                       ? 1
                       : 0;
        case MethodId::DoAdditionalAction: {
            if (sizeArray != 1 || paParams == nullptr) {
                SetLastError(u"DoAdditionalAction expects 1 parameter");
                return SetVariantBool(pvarRetValue, false) ? 1 : 0;
            }
            std::u16string action;
            if (!ReadVariantString(paParams[0], &action, u"DoAdditionalAction.action")) {
                return SetVariantBool(pvarRetValue, false) ? 1 : 0;
            }
            action = ToLower(action);
            if (action == u"testconnection") {
                return SetVariantBool(pvarRetValue, HttpGetBaseUrl()) ? 1 : 0;
            }
            if (action == u"showlastlog") {
                ShellExecuteW(nullptr, L"open", logPath_.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
                return SetVariantBool(pvarRetValue, true) ? 1 : 0;
            }
            SetLastError(u"Unsupported action");
            return SetVariantBool(pvarRetValue, false) ? 1 : 0;
        }
        case MethodId::GetLastError:
            return SetVariantString(pvarRetValue, lastError_) ? 1 : 0;
        default:
            return 0;
    }
}

long ADDIN_STDCALL KaspiKassaDriver::GetNEvents() { return 0; }
const WCHAR_T* ADDIN_STDCALL KaspiKassaDriver::GetEventName(long, long) { return nullptr; }
void ADDIN_STDCALL KaspiKassaDriver::ExternalEvent(const WCHAR_T*, const WCHAR_T*, const WCHAR_T*) {}
void ADDIN_STDCALL KaspiKassaDriver::SetLocale(const WCHAR_T* loc) {
    locale_ = FromWcharT(loc);
}

bool_t ADDIN_STDCALL KaspiKassaDriver::RegisterProfileAs(WCHAR_T** wsProfileName) {
    if (wsProfileName == nullptr || memMgr_ == nullptr) {
        return 0;
    }
    void* mem = nullptr;
    const std::u16string value = u"KaspiKassaAPIv3";
    const std::uint32_t bytes = static_cast<std::uint32_t>((value.size() + 1) * sizeof(WCHAR_T));
    if (!memMgr_->AllocMemory(&mem, bytes) || mem == nullptr) {
        return 0;
    }
    auto* out = static_cast<WCHAR_T*>(mem);
    for (size_t i = 0; i < value.size(); ++i) {
        out[i] = static_cast<WCHAR_T>(value[i]);
    }
    out[value.size()] = 0;
    *wsProfileName = out;
    return 1;
}

bool_t ADDIN_STDCALL KaspiKassaDriver::GetPropertyProfile(WCHAR_T** wsData) {
    if (wsData == nullptr || memMgr_ == nullptr) {
        return 0;
    }
    const std::u16string value = u"<Profile/>";
    void* mem = nullptr;
    const std::uint32_t bytes = static_cast<std::uint32_t>((value.size() + 1) * sizeof(WCHAR_T));
    if (!memMgr_->AllocMemory(&mem, bytes) || mem == nullptr) {
        return 0;
    }
    auto* out = static_cast<WCHAR_T*>(mem);
    for (size_t i = 0; i < value.size(); ++i) {
        out[i] = static_cast<WCHAR_T>(value[i]);
    }
    out[value.size()] = 0;
    *wsData = out;
    return 1;
}

bool_t ADDIN_STDCALL KaspiKassaDriver::SetPropertyProfile(const WCHAR_T*) {
    return 1;
}

bool KaspiKassaDriver::HttpGetBaseUrl() {
    const std::wstring url = U16ToUtf16(baseUrl_);
    URL_COMPONENTS uc{};
    uc.dwStructSize = sizeof(uc);
    wchar_t host[256] = {};
    wchar_t path[1024] = {};
    uc.lpszHostName = host;
    uc.dwHostNameLength = static_cast<DWORD>(std::size(host));
    uc.lpszUrlPath = path;
    uc.dwUrlPathLength = static_cast<DWORD>(std::size(path));

    if (!WinHttpCrackUrl(url.c_str(), 0, 0, &uc)) {
        SetLastError(u"WinHttpCrackUrl failed");
        Log(L"DeviceTest: WinHttpCrackUrl failed");
        return false;
    }

    HINTERNET hSession = WinHttpOpen(L"KaspiKassaAPIv3/1.0",
                                     WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                                     WINHTTP_NO_PROXY_NAME,
                                     WINHTTP_NO_PROXY_BYPASS,
                                     0);
    if (hSession == nullptr) {
        SetLastError(u"WinHttpOpen failed");
        Log(L"DeviceTest: WinHttpOpen failed");
        return false;
    }

    WinHttpSetTimeouts(hSession, timeoutMs_, timeoutMs_, timeoutMs_, timeoutMs_);

    HINTERNET hConnect = WinHttpConnect(hSession, host, uc.nPort, 0);
    if (hConnect == nullptr) {
        WinHttpCloseHandle(hSession);
        SetLastError(u"WinHttpConnect failed");
        Log(L"DeviceTest: WinHttpConnect failed");
        return false;
    }

    const DWORD flags = (uc.nScheme == INTERNET_SCHEME_HTTPS) ? WINHTTP_FLAG_SECURE : 0;
    HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"GET", path, nullptr, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, flags);

    bool ok = false;
    if (hRequest != nullptr && WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0) &&
        WinHttpReceiveResponse(hRequest, nullptr)) {
        DWORD code = 0;
        DWORD codeSize = sizeof(code);
        if (WinHttpQueryHeaders(hRequest,
                                WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
                                WINHTTP_HEADER_NAME_BY_INDEX,
                                &code,
                                &codeSize,
                                WINHTTP_NO_HEADER_INDEX)) {
            ok = (code >= 200 && code < 300);
            if (ok) {
                SetLastError(u"");
                Log(L"DeviceTest: HTTP status OK", true);
            } else {
                SetLastError(u"HTTP status not OK");
                Log(L"DeviceTest: HTTP status not OK");
            }
        }
    } else {
        SetLastError(u"WinHTTP request failed");
        Log(L"DeviceTest: request failed");
    }

    if (hRequest != nullptr) {
        WinHttpCloseHandle(hRequest);
    }
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);
    return ok;
}

void KaspiKassaDriver::Log(const std::wstring& message, bool verboseOnly) const {
    if (verboseOnly && !verboseLog_) {
        return;
    }
    std::wofstream out(logPath_, std::ios::app);
    if (!out.is_open()) {
        return;
    }
    SYSTEMTIME st{};
    GetLocalTime(&st);
    out << st.wYear << L'-' << st.wMonth << L'-' << st.wDay << L' '
        << st.wHour << L':' << st.wMinute << L':' << st.wSecond << L" | " << message << L"\n";
}

bool KaspiKassaDriver::SetVariantBool(tVariant* var, bool value) const {
    if (var == nullptr) {
        return false;
    }
    var->value.bVal = value ? 1 : 0;
    var->vt = VTYPE_BOOL;
    var->wReserved1 = 0;
    var->wReserved2 = 0;
    var->wReserved3 = 0;
    return true;
}

bool KaspiKassaDriver::SetVariantString(tVariant* var, const std::u16string& value) const {
    if (memMgr_ == nullptr) {
        return false;
    }
    void* mem = nullptr;
    const std::uint32_t bytes = static_cast<std::uint32_t>((value.size() + 1) * sizeof(WCHAR_T));
    if (!memMgr_->AllocMemory(&mem, bytes) || mem == nullptr) {
        return false;
    }
    auto* out = static_cast<WCHAR_T*>(mem);
    for (size_t i = 0; i < value.size(); ++i) {
        out[i] = static_cast<WCHAR_T>(value[i]);
    }
    out[value.size()] = 0;

    if (var != nullptr) {
        var->value.str.ptrVal = out;
        var->value.str.strLen = static_cast<std::uint32_t>(value.size());
        var->vt = VTYPE_LPWSTR;
        var->wReserved1 = 0;
        var->wReserved2 = 0;
        var->wReserved3 = 0;
    }
    return true;
}

bool KaspiKassaDriver::ReadVariantString(const tVariant& var, std::u16string* out, const char16_t* argName) {
    if (out == nullptr) {
        SetLastError(std::u16string(argName) + u" output is null");
        return false;
    }
    if (var.vt != VTYPE_LPWSTR && var.vt != VTYPE_PWSTR) {
        SetLastError(std::u16string(argName) + u" must be string");
        return false;
    }
    *out = FromWcharT(var.value.str.ptrVal);
    return true;
}

std::u16string KaspiKassaDriver::Utf16ToU16(const std::wstring& src) const {
    std::u16string out;
    out.reserve(src.size());
    for (wchar_t c : src) {
        out.push_back(static_cast<char16_t>(c));
    }
    return out;
}

std::wstring KaspiKassaDriver::U16ToUtf16(const std::u16string& src) const {
    std::wstring out;
    out.reserve(src.size());
    for (char16_t c : src) {
        out.push_back(static_cast<wchar_t>(c));
    }
    return out;
}

std::u16string KaspiKassaDriver::ToLower(std::u16string value) const {
    std::transform(value.begin(), value.end(), value.begin(), [](char16_t c) {
        if (c >= u'A' && c <= u'Z') {
            return static_cast<char16_t>(c + 32);
        }
        if (c >= u'А' && c <= u'Я') {
            return static_cast<char16_t>(c + 32);
        }
        return c;
    });
    return value;
}

bool KaspiKassaDriver::SetParameterInternal(const std::u16string& name, const tVariant& value) {
    const std::u16string low = ToLower(name);

    const bool isBase = (low == u"baseurl" || low == u"базовыйurl");
    const bool isToken = (low == u"token" || low == u"токен");
    const bool isPoint = (low == u"pointid" || low == u"идентификаторточки");
    const bool isTimeout = (low == u"timeoutms" || low == u"таймаутмс");
    const bool isPoll = (low == u"pollintervalms" || low == u"интервалполлингамс");
    const bool isVerbose = (low == u"verboselog" || low == u"подробныйлог");

    if (isBase || isToken || isPoint) {
        std::u16string text;
        if (!ReadVariantString(value, &text, u"SetParameter.value")) {
            return false;
        }
        if (isBase) {
            baseUrl_ = text;
        } else if (isToken) {
            token_ = text;
        } else {
            pointId_ = text;
        }
        Log(L"SetParameter string value applied", true);
        return true;
    }

    if (isTimeout || isPoll) {
        if (value.vt != VTYPE_I4 && value.vt != VTYPE_INT) {
            SetLastError(u"Numeric parameter expects integer");
            return false;
        }
        if (isTimeout) {
            timeoutMs_ = value.value.lVal;
        } else {
            pollIntervalMs_ = value.value.lVal;
        }
        return true;
    }

    if (isVerbose) {
        if (value.vt != VTYPE_BOOL) {
            SetLastError(u"VerboseLog expects boolean");
            return false;
        }
        verboseLog_ = (value.value.bVal != 0);
        return true;
    }

    SetLastError(u"Unknown parameter");
    return false;
}

void KaspiKassaDriver::SetLastError(const std::u16string& value) {
    lastError_ = value;
    Log(U16ToUtf16(value));
}
