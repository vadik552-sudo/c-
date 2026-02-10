#include "Component.h"

#include "WinHttpClient.h"

#include <algorithm>
#include <cwctype>
#include <sstream>

namespace {

IMemoryManager* g_memoryManager = nullptr;

const KaspiKassaComponent::MethodInfo kMethods[] = {
    {L"ПолучитьВерсию", L"GetVersion", 0, true},
    {L"ПолучитьОписание", L"GetDescription", 0, true},
    {L"ПолучитьПараметры", L"GetParameters", 0, true},
    {L"УстановитьПараметр", L"SetParameter", 2, false},
    {L"Открыть", L"Open", 0, true},
    {L"Закрыть", L"Close", 0, false},
    {L"ТестУстройства", L"DeviceTest", 0, true},
    {L"ПолучитьДополнительныеДействия", L"GetAdditionalActions", 0, true},
    {L"ВыполнитьДополнительноеДействие", L"DoAdditionalAction", 1, true},
    {L"ПолучитьПоследнююОшибку", L"GetLastError", 0, true},
};

const KaspiKassaComponent::PropInfo kProps[] = {
    {L"Открыт", L"IsOpened", true, false},
    {L"ПоследняяОшибка", L"LastError", true, false},
};

constexpr wchar_t kExtensionName[] = L"KaspiKassaAPIv3";

std::wstring ToLower(const std::wstring& value) {
    std::wstring output(value);
    std::transform(output.begin(), output.end(), output.begin(), [](wchar_t ch) {
        return static_cast<wchar_t>(std::towlower(ch));
    });
    return output;
}

}  // namespace

IMemoryManager* GetMemoryManager() {
    return g_memoryManager;
}

void SetMemoryManager(IMemoryManager* memoryManager) {
    g_memoryManager = memoryManager;
}

KaspiKassaComponent::KaspiKassaComponent()
    : connection_(nullptr),
      opened_(false),
      requestTimeoutSec_(30),
      pollIntervalSec_(3),
      verboseLog_(false) {
    baseUrl_ = L"https://kaspi.kz";
}

KaspiKassaComponent::~KaspiKassaComponent() = default;

bool_t KaspiKassaComponent::Init(void* pConnection) {
    connection_ = pConnection;
    return 1;
}

bool_t KaspiKassaComponent::setMemManager(void* pMemManager) {
    SetMemoryManager(static_cast<IMemoryManager*>(pMemManager));
    return (GetMemoryManager() != nullptr) ? 1 : 0;
}

long KaspiKassaComponent::GetInfo() {
    return eAppCapabilities1;
}

void KaspiKassaComponent::Done() {
    opened_ = false;
    connection_ = nullptr;
}

bool_t KaspiKassaComponent::RegisterExtensionAs(WCHAR_T** wsExtensionName) {
    if (wsExtensionName == nullptr || GetMemoryManager() == nullptr) {
        return 0;
    }
    const std::wstring name(kExtensionName);
    void* memory = nullptr;
    const unsigned long bytes = static_cast<unsigned long>((name.size() + 1) * sizeof(WCHAR_T));
    if (!GetMemoryManager()->AllocMemory(&memory, bytes) || memory == nullptr) {
        return 0;
    }
    WCHAR_T* output = static_cast<WCHAR_T*>(memory);
    for (size_t i = 0; i < name.size(); ++i) {
        output[i] = static_cast<WCHAR_T>(name[i]);
    }
    output[name.size()] = 0;
    *wsExtensionName = output;
    return 1;
}

long KaspiKassaComponent::GetNProps() {
    return static_cast<long>(PropId::Count);
}

long KaspiKassaComponent::FindProp(const WCHAR_T* wsPropName) {
    if (wsPropName == nullptr) {
        return -1;
    }
    const std::wstring lookup = ToLower(ToWString(wsPropName));
    for (long i = 0; i < static_cast<long>(PropId::Count); ++i) {
        if (lookup == ToLower(kProps[i].ru) || lookup == ToLower(kProps[i].en)) {
            return i;
        }
    }
    return -1;
}

const WCHAR_T* KaspiKassaComponent::GetPropName(long lPropNum, long lPropAlias) {
    if (lPropNum < 0 || lPropNum >= static_cast<long>(PropId::Count)) {
        return nullptr;
    }
    const wchar_t* name = (lPropAlias == 0) ? kProps[lPropNum].ru : kProps[lPropNum].en;
    return reinterpret_cast<const WCHAR_T*>(name);
}

bool_t KaspiKassaComponent::GetPropVal(const long lPropNum, tVariant* pvarPropVal) {
    if (pvarPropVal == nullptr) {
        return 0;
    }
    switch (static_cast<PropId>(lPropNum)) {
        case PropId::IsOpened:
            return SetVariantBool(pvarPropVal, opened_);
        case PropId::LastError:
            return SetVariantString(pvarPropVal, lastError_);
        default:
            return 0;
    }
}

bool_t KaspiKassaComponent::SetPropVal(const long, tVariant*) {
    return 0;
}

bool_t KaspiKassaComponent::IsPropReadable(const long lPropNum) {
    if (lPropNum < 0 || lPropNum >= static_cast<long>(PropId::Count)) {
        return 0;
    }
    return kProps[lPropNum].readable ? 1 : 0;
}

bool_t KaspiKassaComponent::IsPropWritable(const long lPropNum) {
    if (lPropNum < 0 || lPropNum >= static_cast<long>(PropId::Count)) {
        return 0;
    }
    return kProps[lPropNum].writable ? 1 : 0;
}

long KaspiKassaComponent::GetNMethods() {
    return static_cast<long>(MethodId::Count);
}

long KaspiKassaComponent::FindMethod(const WCHAR_T* wsMethodName) {
    if (wsMethodName == nullptr) {
        return -1;
    }
    const std::wstring lookup = ToLower(ToWString(wsMethodName));
    for (long i = 0; i < static_cast<long>(MethodId::Count); ++i) {
        if (lookup == ToLower(kMethods[i].ru) || lookup == ToLower(kMethods[i].en)) {
            return i;
        }
    }
    return -1;
}

const WCHAR_T* KaspiKassaComponent::GetMethodName(const long lMethodNum, const long lMethodAlias) {
    if (lMethodNum < 0 || lMethodNum >= static_cast<long>(MethodId::Count)) {
        return nullptr;
    }
    const wchar_t* name = (lMethodAlias == 0) ? kMethods[lMethodNum].ru : kMethods[lMethodNum].en;
    return reinterpret_cast<const WCHAR_T*>(name);
}

long KaspiKassaComponent::GetNParams(const long lMethodNum) {
    if (lMethodNum < 0 || lMethodNum >= static_cast<long>(MethodId::Count)) {
        return 0;
    }
    return kMethods[lMethodNum].params;
}

bool_t KaspiKassaComponent::GetParamDefValue(const long, const long, tVariant*) {
    return 0;
}

bool_t KaspiKassaComponent::HasRetVal(const long lMethodNum) {
    if (lMethodNum < 0 || lMethodNum >= static_cast<long>(MethodId::Count)) {
        return 0;
    }
    return kMethods[lMethodNum].hasRet ? 1 : 0;
}

bool_t KaspiKassaComponent::CallAsProc(const long lMethodNum, tVariant* paParams, const long lSizeArray) {
    if (lMethodNum == static_cast<long>(MethodId::SetParameter)) {
        if (lSizeArray != 2 || paParams == nullptr) {
            SetLastError(L"SetParameter expects 2 parameters");
            return 0;
        }
        std::wstring key;
        if (!TryReadVariantString(paParams[0], &key, L"SetParameter key")) {
            return 0;
        }
        return SetParameterValue(key, paParams[1]) ? 1 : 0;
    }
    if (lMethodNum == static_cast<long>(MethodId::Close)) {
        opened_ = false;
        return 1;
    }
    return 0;
}

bool_t KaspiKassaComponent::CallAsFunc(const long lMethodNum, tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) {
    if (pvarRetValue == nullptr) {
        return 0;
    }

    switch (static_cast<MethodId>(lMethodNum)) {
        case MethodId::GetVersion:
            return SetVariantString(pvarRetValue, L"3.0.0");

        case MethodId::GetDescription:
            return SetVariantString(pvarRetValue, L"KaspiKassa API v3 Native driver");

        case MethodId::GetParameters:
            return SetVariantString(
                pvarRetValue,
                L"БазовыйURL;Токен;ИдентификаторТочки;МаксимальноеВремяОжидания;ИнтервалПаузыПоллинга;ВключитьПодробныйЛог");

        case MethodId::SetParameter: {
            if (lSizeArray != 2 || paParams == nullptr) {
                SetLastError(L"SetParameter expects 2 parameters");
                return 0;
            }
            std::wstring key;
            if (!TryReadVariantString(paParams[0], &key, L"SetParameter key")) {
                return SetVariantBool(pvarRetValue, false);
            }
            return SetVariantBool(pvarRetValue, SetParameterValue(key, paParams[1]));
        }

        case MethodId::Open:
            opened_ = true;
            SetLastError(L"");
            return SetVariantBool(pvarRetValue, true);

        case MethodId::Close:
            opened_ = false;
            return SetVariantBool(pvarRetValue, true);

        case MethodId::DeviceTest: {
            std::wstring response;
            const bool ok = DoHttpTest(&response);
            if (ok) {
                return SetVariantString(pvarRetValue, response);
            }
            return SetVariantString(pvarRetValue, lastError_);
        }

        case MethodId::GetAdditionalActions:
            return SetVariantString(pvarRetValue, L"TestConnection;ShowLastLog");

        case MethodId::DoAdditionalAction: {
            if (lSizeArray != 1 || paParams == nullptr) {
                SetLastError(L"DoAdditionalAction expects one parameter");
                return 0;
            }
            std::wstring action;
            if (!TryReadVariantString(paParams[0], &action, L"DoAdditionalAction action")) {
                return SetVariantString(pvarRetValue, lastError_);
            }
            action = ToLower(action);
            if (action == L"testconnection") {
                std::wstring response;
                const bool ok = DoHttpTest(&response);
                return SetVariantString(pvarRetValue, ok ? response : lastError_);
            }
            if (action == L"showlastlog") {
                return SetVariantString(pvarRetValue, lastLog_);
            }
            SetLastError(L"Unsupported action");
            return SetVariantString(pvarRetValue, lastError_);
        }

        case MethodId::GetLastError:
            return SetVariantString(pvarRetValue, lastError_);

        default:
            return 0;
    }
}

long KaspiKassaComponent::GetNEvents() {
    return 0;
}

const WCHAR_T* KaspiKassaComponent::GetEventName(const long, const long) {
    return nullptr;
}

void KaspiKassaComponent::ExternalEvent(const WCHAR_T*, const WCHAR_T*, const WCHAR_T*) {
}

void KaspiKassaComponent::SetLocale(const WCHAR_T* loc) {
    locale_ = ToWString(loc);
}

bool KaspiKassaComponent::SetVariantString(tVariant* variant, const std::wstring& value) const {
    if (variant == nullptr || GetMemoryManager() == nullptr) {
        return false;
    }
    void* memory = nullptr;
    const size_t length = value.size();
    const unsigned long bytes = static_cast<unsigned long>((length + 1) * sizeof(WCHAR_T));
    if (!GetMemoryManager()->AllocMemory(&memory, bytes) || memory == nullptr) {
        return false;
    }

    auto* out = static_cast<WCHAR_T*>(memory);
    for (size_t i = 0; i < length; ++i) {
        out[i] = static_cast<WCHAR_T>(value[i]);
    }
    out[length] = 0;

    variant->value.str.ptrVal = out;
    variant->value.str.strLen = static_cast<std::uint32_t>(length);
    variant->vt = VTYPE_LPWSTR;
    variant->wReserved1 = 0;
    variant->wReserved2 = 0;
    variant->wReserved3 = 0;
    return true;
}

bool KaspiKassaComponent::SetVariantBool(tVariant* variant, bool value) const {
    if (variant == nullptr) {
        return false;
    }
    variant->value.bVal = value ? 1 : 0;
    variant->vt = VTYPE_BOOL;
    variant->wReserved1 = 0;
    variant->wReserved2 = 0;
    variant->wReserved3 = 0;
    return true;
}

bool KaspiKassaComponent::SetVariantInt(tVariant* variant, std::int32_t value) const {
    if (variant == nullptr) {
        return false;
    }
    variant->value.lVal = value;
    variant->vt = VTYPE_I4;
    variant->wReserved1 = 0;
    variant->wReserved2 = 0;
    variant->wReserved3 = 0;
    return true;
}

std::wstring KaspiKassaComponent::ToWString(const WCHAR_T* value) {
    if (value == nullptr) {
        return std::wstring();
    }
    std::wstring output;
    while (*value != 0) {
        output.push_back(static_cast<wchar_t>(*value));
        ++value;
    }
    return output;
}

bool KaspiKassaComponent::EqualsNoCase(const std::wstring& left, const wchar_t* right) {
    return ToLower(left) == ToLower(std::wstring(right));
}

bool KaspiKassaComponent::TryReadVariantString(const tVariant& value, std::wstring* out, const wchar_t* argName) {
    if (value.vt != VTYPE_LPWSTR && value.vt != VTYPE_PWSTR) {
        SetLastError(std::wstring(argName) + L" must be a string");
        return false;
    }
    if (out == nullptr) {
        SetLastError(std::wstring(argName) + L" output is null");
        return false;
    }
    *out = ToWString(value.value.str.ptrVal);
    return true;
}

bool KaspiKassaComponent::SetParameterValue(const std::wstring& key, const tVariant& value) {
    const std::wstring lowKey = ToLower(key);
    auto asString = [this, &value, &key]() -> std::wstring {
        if (value.vt != VTYPE_LPWSTR && value.vt != VTYPE_PWSTR) {
            SetLastError(L"Parameter '" + key + L"' expects a string value");
            SetLastError(L"Parameter '" + key + L"' expects a string value");
            return std::wstring();
        }
        return ToWString(value.value.str.ptrVal);
    };

    if (EqualsNoCase(lowKey, L"базовыйurl") || EqualsNoCase(lowKey, L"baseurl")) {
        baseUrl_ = asString();
        return !baseUrl_.empty();
    }
    if (EqualsNoCase(lowKey, L"токен") || EqualsNoCase(lowKey, L"token")) {
        token_ = asString();
        return true;
    }
    if (EqualsNoCase(lowKey, L"идентификаторточки") || EqualsNoCase(lowKey, L"pointid")) {
        pointId_ = asString();
        return true;
    }
    if (EqualsNoCase(lowKey, L"максимальноевремяожидания") || EqualsNoCase(lowKey, L"maxwaitseconds")) {
        if (value.vt == VTYPE_I4 || value.vt == VTYPE_INT) {
            requestTimeoutSec_ = value.value.lVal;
            return true;
        }
        return false;
    }
    if (EqualsNoCase(lowKey, L"интервалпаузыполлинга") || EqualsNoCase(lowKey, L"pollintervalseconds")) {
        if (value.vt == VTYPE_I4 || value.vt == VTYPE_INT) {
            pollIntervalSec_ = value.value.lVal;
            return true;
        }
        return false;
    }
    if (EqualsNoCase(lowKey, L"включитьподробныйлог") || EqualsNoCase(lowKey, L"verbose")) {
        if (value.vt == VTYPE_BOOL) {
            verboseLog_ = (value.value.bVal != 0);
            return true;
        }
        return false;
    }

    SetLastError(L"Unknown parameter: " + key);
    return false;
}

bool KaspiKassaComponent::GetParameterValue(const std::wstring&, std::wstring*) const {
    return false;
}

    const HttpResult result = PerformHttpGet(baseUrl_, static_cast<unsigned int>(requestTimeoutSec_));
    if (!result.success) {
        SetLastError(result.error);
        lastLog_ = L"HTTP GET failed: " + result.error;
        if (responseText != nullptr) {
            *responseText = lastError_;
        }
        return false;
    }

    std::wstringstream stream;
    stream << L"HTTP " << result.statusCode << L"; " << result.body;
    const std::wstring text = stream.str();
    lastLog_ = text;
    SetLastError(L"");
    if (responseText != nullptr) {
        *responseText = text;
    }
    return true;
}

void KaspiKassaComponent::SetLastError(const std::wstring& error) {
    lastError_ = error;
}
