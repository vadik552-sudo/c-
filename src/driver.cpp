#include "driver.h"

#include <algorithm>
#include <cstddef>

namespace {

#define W16(lit) reinterpret_cast<const WCHAR_T*>(u##lit)

constexpr const WCHAR_T* kClassName = W16("KaspiKassaAPIv3");

struct MethodDef {
    const WCHAR_T* ru;
    const WCHAR_T* en;
    long params;
    bool hasRet;
};

constexpr std::array<MethodDef, static_cast<size_t>(MethodId::Count)> kMethods = {{
    {W16("ПолучитьВерсию"), W16("GetVersion"), 0, true},
    {W16("ПолучитьОписание"), W16("GetDescription"), 0, true},
    {W16("ПолучитьПараметры"), W16("GetParameters"), 0, true},
    {W16("УстановитьПараметр"), W16("SetParameter"), 2, true},
    {W16("Открыть"), W16("Open"), 0, true},
    {W16("Закрыть"), W16("Close"), 0, true},
    {W16("ТестУстройства"), W16("DeviceTest"), 0, true},
    {W16("ПолучитьДополнительныеДействия"), W16("GetAdditionalActions"), 0, true},
    {W16("ВыполнитьДополнительноеДействие"), W16("DoAdditionalAction"), 1, true},
    {W16("ПолучитьПоследнююОшибку"), W16("GetLastError"), 0, true},
}};

std::u16string ToU16(const WCHAR_T* src) {
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

std::u16string ToLower(std::u16string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](char16_t ch) {
        if (ch >= u'A' && ch <= u'Z') {
            return static_cast<char16_t>(ch + 32);
        }
        return ch;
    });
    return value;
}

}  // namespace

KaspiKassaDriver::KaspiKassaDriver() = default;
KaspiKassaDriver::~KaspiKassaDriver() = default;

bool_t ADDIN_STDCALL KaspiKassaDriver::Init(void* pConnection) {
    connection_ = pConnection;
    return 1;
}

bool_t ADDIN_STDCALL KaspiKassaDriver::setMemManager(void* pMemManager) {
    mem_ = static_cast<IMemoryManager*>(pMemManager);
    return mem_ != nullptr ? 1 : 0;
}

long ADDIN_STDCALL KaspiKassaDriver::GetInfo() {
    return eAppCapabilities1;
}

void ADDIN_STDCALL KaspiKassaDriver::Done() {
    opened_ = false;
    connection_ = nullptr;
}

bool_t ADDIN_STDCALL KaspiKassaDriver::RegisterExtensionAs(WCHAR_T** wsExtensionName) {
    if (wsExtensionName == nullptr || mem_ == nullptr) {
        return 0;
    }
    void* raw = nullptr;
    constexpr unsigned long byteCount = sizeof(char16_t) * 16;
    if (!mem_->AllocMemory(&raw, byteCount) || raw == nullptr) {
        return 0;
    }
    auto* dst = static_cast<WCHAR_T*>(raw);
    constexpr char16_t kName[] = u"KaspiKassaAPIv3";
    for (size_t i = 0; i < 16; ++i) {
        dst[i] = static_cast<WCHAR_T>(kName[i]);
    }
    *wsExtensionName = dst;
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
    const auto lookup = ToLower(ToU16(wsMethodName));
    for (long i = 0; i < static_cast<long>(MethodId::Count); ++i) {
        if (lookup == ToLower(ToU16(kMethods[static_cast<size_t>(i)].ru)) ||
            lookup == ToLower(ToU16(kMethods[static_cast<size_t>(i)].en))) {
            return i;
        }
    }
    return -1;
}

const WCHAR_T* ADDIN_STDCALL KaspiKassaDriver::GetMethodName(long lMethodNum, long lMethodAlias) {
    if (lMethodNum < 0 || lMethodNum >= static_cast<long>(MethodId::Count)) {
        return nullptr;
    }
    const auto& method = kMethods[static_cast<size_t>(lMethodNum)];
    return lMethodAlias == 0 ? method.ru : method.en;
}

long ADDIN_STDCALL KaspiKassaDriver::GetNParams(long lMethodNum) {
    if (lMethodNum < 0 || lMethodNum >= static_cast<long>(MethodId::Count)) {
        return 0;
    }
    return kMethods[static_cast<size_t>(lMethodNum)].params;
}

bool_t ADDIN_STDCALL KaspiKassaDriver::GetParamDefValue(long, long, tVariant*) {
    return 0;
}

bool_t ADDIN_STDCALL KaspiKassaDriver::HasRetVal(long lMethodNum) {
    if (lMethodNum < 0 || lMethodNum >= static_cast<long>(MethodId::Count)) {
        return 0;
    }
    return kMethods[static_cast<size_t>(lMethodNum)].hasRet ? 1 : 0;
}

bool_t ADDIN_STDCALL KaspiKassaDriver::CallAsProc(long lMethodNum, tVariant* paParams, long lSizeArray) {
    if (lMethodNum == static_cast<long>(MethodId::SetParameter)) {
        if (lSizeArray != 2 || paParams == nullptr) {
            SetLastError(u"SetParameter expects 2 parameters");
            return 0;
        }
        std::u16string key;
        if (!ReadStringParam(paParams[0], &key, u"SetParameter key")) {
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

bool_t ADDIN_STDCALL KaspiKassaDriver::CallAsFunc(long lMethodNum, tVariant* pvarRetValue, tVariant* paParams, long lSizeArray) {
    if (pvarRetValue == nullptr) {
        return 0;
    }

    switch (static_cast<MethodId>(lMethodNum)) {
        case MethodId::GetVersion:
            return SetStringResult(pvarRetValue, u"3.0.1") ? 1 : 0;
        case MethodId::GetDescription:
            return SetStringResult(pvarRetValue, u"KaspiKassa API v3 Native driver") ? 1 : 0;
        case MethodId::GetParameters:
            return SetStringResult(pvarRetValue, u"BaseUrl;Token;PointId;MaxWaitSeconds;PollIntervalSeconds;Verbose") ? 1 : 0;
        case MethodId::SetParameter: {
            if (lSizeArray != 2 || paParams == nullptr) {
                SetLastError(u"SetParameter expects 2 parameters");
                return SetBoolResult(pvarRetValue, false) ? 1 : 0;
            }
            std::u16string key;
            if (!ReadStringParam(paParams[0], &key, u"SetParameter key")) {
                return SetBoolResult(pvarRetValue, false) ? 1 : 0;
            }
            return SetBoolResult(pvarRetValue, SetParameterValue(key, paParams[1])) ? 1 : 0;
        }
        case MethodId::Open:
            opened_ = true;
            SetLastError(u"");
            return SetBoolResult(pvarRetValue, true) ? 1 : 0;
        case MethodId::Close:
            opened_ = false;
            return SetBoolResult(pvarRetValue, true) ? 1 : 0;
        case MethodId::DeviceTest:
            return SetStringResult(pvarRetValue, opened_ ? u"OK" : u"CLOSED") ? 1 : 0;
        case MethodId::GetAdditionalActions:
            return SetStringResult(pvarRetValue, u"TestConnection;ShowLastLog") ? 1 : 0;
        case MethodId::DoAdditionalAction: {
            if (lSizeArray != 1 || paParams == nullptr) {
                SetLastError(u"DoAdditionalAction expects one parameter");
                return SetStringResult(pvarRetValue, lastError_) ? 1 : 0;
            }
            std::u16string action;
            if (!ReadStringParam(paParams[0], &action, u"DoAdditionalAction action")) {
                return SetStringResult(pvarRetValue, lastError_) ? 1 : 0;
            }
            action = ToLower(action);
            if (action == u"testconnection") {
                return SetStringResult(pvarRetValue, opened_ ? u"OK" : u"CLOSED") ? 1 : 0;
            }
            if (action == u"showlastlog") {
                return SetStringResult(pvarRetValue, lastLog_) ? 1 : 0;
            }
            SetLastError(u"Unsupported action");
            return SetStringResult(pvarRetValue, lastError_) ? 1 : 0;
        }
        case MethodId::GetLastError:
            return SetStringResult(pvarRetValue, lastError_) ? 1 : 0;
        default:
            return 0;
    }
}

long ADDIN_STDCALL KaspiKassaDriver::GetNEvents() { return 0; }
const WCHAR_T* ADDIN_STDCALL KaspiKassaDriver::GetEventName(long, long) { return nullptr; }
void ADDIN_STDCALL KaspiKassaDriver::ExternalEvent(const WCHAR_T*, const WCHAR_T*, const WCHAR_T*) {}
void ADDIN_STDCALL KaspiKassaDriver::SetLocale(const WCHAR_T* loc) { locale_ = ToU16(loc); }

bool KaspiKassaDriver::SetStringResult(tVariant* variant, const std::u16string& value) const {
    if (variant == nullptr || mem_ == nullptr) {
        return false;
    }
    void* raw = nullptr;
    const auto bytes = static_cast<unsigned long>((value.size() + 1) * sizeof(WCHAR_T));
    if (!mem_->AllocMemory(&raw, bytes) || raw == nullptr) {
        return false;
    }
    auto* dst = static_cast<WCHAR_T*>(raw);
    for (size_t i = 0; i < value.size(); ++i) {
        dst[i] = static_cast<WCHAR_T>(value[i]);
    }
    dst[value.size()] = 0;
    variant->value.str.ptrVal = dst;
    variant->value.str.strLen = static_cast<std::uint32_t>(value.size());
    variant->vt = VTYPE_LPWSTR;
    variant->wReserved1 = 0;
    variant->wReserved2 = 0;
    variant->wReserved3 = 0;
    return true;
}

bool KaspiKassaDriver::SetBoolResult(tVariant* variant, bool value) const {
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

bool KaspiKassaDriver::ReadStringParam(const tVariant& value, std::u16string* out, const char16_t* name) {
    if (out == nullptr) {
        SetLastError(std::u16string(name) + u" output is null");
        return false;
    }
    if (value.vt != VTYPE_LPWSTR && value.vt != VTYPE_PWSTR) {
        SetLastError(std::u16string(name) + u" must be string");
        return false;
    }
    *out = ToU16(value.value.str.ptrVal);
    return true;
}

bool KaspiKassaDriver::SetParameterValue(const std::u16string& key, const tVariant& value) {
    const auto lowKey = ToLower(key);
    auto readString = [&](std::u16string* dst) -> bool {
        if (value.vt != VTYPE_LPWSTR && value.vt != VTYPE_PWSTR) {
            SetLastError(u"Parameter must be string");
            return false;
        }
        *dst = ToU16(value.value.str.ptrVal);
        return true;
    };

    if (lowKey == u"baseurl") {
        return readString(&baseUrl_);
    }
    if (lowKey == u"token") {
        return readString(&token_);
    }
    if (lowKey == u"pointid") {
        return readString(&pointId_);
    }
    if (lowKey == u"maxwaitseconds") {
        if (value.vt == VTYPE_I4 || value.vt == VTYPE_INT) {
            requestTimeoutSec_ = value.value.lVal;
            return true;
        }
        SetLastError(u"maxwaitseconds must be integer");
        return false;
    }
    if (lowKey == u"pollintervalseconds") {
        if (value.vt == VTYPE_I4 || value.vt == VTYPE_INT) {
            pollIntervalSec_ = value.value.lVal;
            return true;
        }
        SetLastError(u"pollintervalseconds must be integer");
        return false;
    }
    if (lowKey == u"verbose") {
        if (value.vt == VTYPE_BOOL) {
            verbose_ = value.value.bVal != 0;
            return true;
        }
        SetLastError(u"verbose must be boolean");
        return false;
    }

    SetLastError(u"Unknown parameter");
    return false;
}

void KaspiKassaDriver::SetLastError(const std::u16string& text) {
    lastError_ = text;
}

extern "C" const WCHAR_T* GetComponentClassName() {
    return kClassName;
}
