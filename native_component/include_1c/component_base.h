#pragma once

#include "types.h"

#if defined(_MSC_VER)
#define ADDIN_STDCALL __stdcall
#else
#define ADDIN_STDCALL
#endif

constexpr long APP_CAP_NATIVE = 2000;

class IMemoryManager {
public:
    virtual bool_t ADDIN_STDCALL AllocMemory(void** pMemory, std::uint32_t sizeBytes) = 0;
    virtual bool_t ADDIN_STDCALL FreeMemory(void** pMemory) = 0;

protected:
    virtual ~IMemoryManager() = default;
};

class IAddInDefBase {
public:
    virtual bool_t ADDIN_STDCALL Init(void* pConnection) = 0;
    virtual bool_t ADDIN_STDCALL setMemManager(void* pMemMgr) = 0;
    virtual long ADDIN_STDCALL GetInfo() = 0;
    virtual void ADDIN_STDCALL Done() = 0;
    virtual bool_t ADDIN_STDCALL RegisterExtensionAs(WCHAR_T** wsExtensionName) = 0;

protected:
    virtual ~IAddInDefBase() = default;
};

class ILanguageExtenderBase : public IAddInDefBase {
public:
    virtual long ADDIN_STDCALL GetNProps() = 0;
    virtual long ADDIN_STDCALL FindProp(const WCHAR_T* wsPropName) = 0;
    virtual const WCHAR_T* ADDIN_STDCALL GetPropName(long propNum, long propAlias) = 0;
    virtual bool_t ADDIN_STDCALL GetPropVal(long propNum, tVariant* pvarPropVal) = 0;
    virtual bool_t ADDIN_STDCALL SetPropVal(long propNum, tVariant* pvarPropVal) = 0;
    virtual bool_t ADDIN_STDCALL IsPropReadable(long propNum) = 0;
    virtual bool_t ADDIN_STDCALL IsPropWritable(long propNum) = 0;

    virtual long ADDIN_STDCALL GetNMethods() = 0;
    virtual long ADDIN_STDCALL FindMethod(const WCHAR_T* wsMethodName) = 0;
    virtual const WCHAR_T* ADDIN_STDCALL GetMethodName(long methodNum, long methodAlias) = 0;
    virtual long ADDIN_STDCALL GetNParams(long methodNum) = 0;
    virtual bool_t ADDIN_STDCALL GetParamDefValue(long methodNum, long paramNum, tVariant* pvarParamDefValue) = 0;
    virtual bool_t ADDIN_STDCALL HasRetVal(long methodNum) = 0;
    virtual bool_t ADDIN_STDCALL CallAsProc(long methodNum, tVariant* paParams, long sizeArray) = 0;
    virtual bool_t ADDIN_STDCALL CallAsFunc(long methodNum, tVariant* pvarRetValue, tVariant* paParams, long sizeArray) = 0;

    virtual long ADDIN_STDCALL GetNEvents() = 0;
    virtual const WCHAR_T* ADDIN_STDCALL GetEventName(long eventNum, long eventAlias) = 0;
    virtual void ADDIN_STDCALL ExternalEvent(const WCHAR_T* wsSource, const WCHAR_T* wsMessage, const WCHAR_T* wsData) = 0;
    virtual void ADDIN_STDCALL SetLocale(const WCHAR_T* loc) = 0;

    virtual ~ILanguageExtenderBase() override = default;
};

class IPropertyProfile {
public:
    virtual bool_t ADDIN_STDCALL RegisterProfileAs(WCHAR_T** wsProfileName) = 0;
    virtual bool_t ADDIN_STDCALL GetPropertyProfile(WCHAR_T** wsData) = 0;
    virtual bool_t ADDIN_STDCALL SetPropertyProfile(const WCHAR_T* wsData) = 0;

protected:
    virtual ~IPropertyProfile() = default;
};

using IComponentBase = ILanguageExtenderBase;
