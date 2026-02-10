#pragma once

#include <cstdint>

using WCHAR_T = std::uint16_t;
using bool_t = long;

constexpr bool_t eAppCapabilitiesInvalid = 0;
constexpr bool_t eAppCapabilities1 = 1;
constexpr bool_t eAppCapabilities2 = 2;

constexpr std::uint16_t VTYPE_EMPTY = 0;
constexpr std::uint16_t VTYPE_NULL = 1;
constexpr std::uint16_t VTYPE_I2 = 2;
constexpr std::uint16_t VTYPE_I4 = 3;
constexpr std::uint16_t VTYPE_R4 = 4;
constexpr std::uint16_t VTYPE_R8 = 5;
constexpr std::uint16_t VTYPE_DATE = 7;
constexpr std::uint16_t VTYPE_BSTR = 8;
constexpr std::uint16_t VTYPE_BOOL = 11;
constexpr std::uint16_t VTYPE_VARIANT = 12;
constexpr std::uint16_t VTYPE_I1 = 16;
constexpr std::uint16_t VTYPE_UI1 = 17;
constexpr std::uint16_t VTYPE_UI2 = 18;
constexpr std::uint16_t VTYPE_UI4 = 19;
constexpr std::uint16_t VTYPE_I8 = 20;
constexpr std::uint16_t VTYPE_UI8 = 21;
constexpr std::uint16_t VTYPE_INT = 22;
constexpr std::uint16_t VTYPE_UINT = 23;
constexpr std::uint16_t VTYPE_PWSTR = 26;
constexpr std::uint16_t VTYPE_BLOB = 0x1000;
constexpr std::uint16_t VTYPE_LPWSTR = 31;

#pragma pack(push, 8)
struct tVariant {
    union {
        bool_t bVal;
        std::int32_t lVal;
        double dblVal;
        std::int64_t hVal;
        struct {
            WCHAR_T* ptrVal;
            std::uint32_t strLen;
        } str;
        void* pvarVal;
    } value;
    std::uint16_t vt;
    std::uint16_t wReserved1;
    std::uint16_t wReserved2;
    std::uint16_t wReserved3;
};
#pragma pack(pop)

static_assert(sizeof(tVariant) == 24, "tVariant size must be 24 bytes on x64");

class IMemoryManager {
public:
    virtual bool_t AllocMemory(void** pMemory, unsigned long ulCountByte) = 0;
    virtual void FreeMemory(void* pMemory) = 0;

protected:
    virtual ~IMemoryManager() = default;
};

class IAddInDefBase {
public:
    virtual bool_t Init(void* pConnection) = 0;
    virtual bool_t setMemManager(void* pMemManager) = 0;
    virtual long GetInfo() = 0;
    virtual void Done() = 0;
    virtual bool_t RegisterExtensionAs(WCHAR_T** wsExtensionName) = 0;

protected:
    virtual ~IAddInDefBase() = default;
};

class ILanguageExtenderBase : public IAddInDefBase {
public:
    virtual long GetNProps() = 0;
    virtual long FindProp(const WCHAR_T* wsPropName) = 0;
    virtual const WCHAR_T* GetPropName(long lPropNum, long lPropAlias) = 0;
    virtual bool_t GetPropVal(const long lPropNum, tVariant* pvarPropVal) = 0;
    virtual bool_t SetPropVal(const long lPropNum, tVariant* pvarPropVal) = 0;
    virtual bool_t IsPropReadable(const long lPropNum) = 0;
    virtual bool_t IsPropWritable(const long lPropNum) = 0;

    virtual long GetNMethods() = 0;
    virtual long FindMethod(const WCHAR_T* wsMethodName) = 0;
    virtual const WCHAR_T* GetMethodName(const long lMethodNum, const long lMethodAlias) = 0;
    virtual long GetNParams(const long lMethodNum) = 0;
    virtual bool_t GetParamDefValue(const long lMethodNum, const long lParamNum, tVariant* pvarParamDefValue) = 0;
    virtual bool_t HasRetVal(const long lMethodNum) = 0;
    virtual bool_t CallAsProc(const long lMethodNum, tVariant* paParams, const long lSizeArray) = 0;
    virtual bool_t CallAsFunc(const long lMethodNum, tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) = 0;

    virtual long GetNEvents() = 0;
    virtual const WCHAR_T* GetEventName(const long lEventNum, const long lEventAlias) = 0;
    virtual void ExternalEvent(const WCHAR_T* wsSource, const WCHAR_T* wsMessage, const WCHAR_T* wsData) = 0;
    virtual void SetLocale(const WCHAR_T* loc) = 0;

protected:
    ~ILanguageExtenderBase() override = default;
};

using IComponentBase = ILanguageExtenderBase;
