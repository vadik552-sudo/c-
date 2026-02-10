#pragma once

#include "types.h"

#if defined(_MSC_VER)
#define ADDIN_STDCALL __stdcall
#else
#define ADDIN_STDCALL
#endif

class IMemoryManager {
public:
    virtual bool_t ADDIN_STDCALL AllocMemory(void** pMemory, unsigned long ulCountByte) = 0;
    virtual void ADDIN_STDCALL FreeMemory(void* pMemory) = 0;

protected:
    virtual ~IMemoryManager() = default;
};

class IAddInDefBase {
public:
    virtual bool_t ADDIN_STDCALL Init(void* pConnection) = 0;
    virtual bool_t ADDIN_STDCALL setMemManager(void* pMemManager) = 0;
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
    virtual const WCHAR_T* ADDIN_STDCALL GetPropName(long lPropNum, long lPropAlias) = 0;
    virtual bool_t ADDIN_STDCALL GetPropVal(long lPropNum, tVariant* pvarPropVal) = 0;
    virtual bool_t ADDIN_STDCALL SetPropVal(long lPropNum, tVariant* pvarPropVal) = 0;
    virtual bool_t ADDIN_STDCALL IsPropReadable(long lPropNum) = 0;
    virtual bool_t ADDIN_STDCALL IsPropWritable(long lPropNum) = 0;

    virtual long ADDIN_STDCALL GetNMethods() = 0;
    virtual long ADDIN_STDCALL FindMethod(const WCHAR_T* wsMethodName) = 0;
    virtual const WCHAR_T* ADDIN_STDCALL GetMethodName(long lMethodNum, long lMethodAlias) = 0;
    virtual long ADDIN_STDCALL GetNParams(long lMethodNum) = 0;
    virtual bool_t ADDIN_STDCALL GetParamDefValue(long lMethodNum, long lParamNum, tVariant* pvarParamDefValue) = 0;
    virtual bool_t ADDIN_STDCALL HasRetVal(long lMethodNum) = 0;
    virtual bool_t ADDIN_STDCALL CallAsProc(long lMethodNum, tVariant* paParams, long lSizeArray) = 0;
    virtual bool_t ADDIN_STDCALL CallAsFunc(long lMethodNum, tVariant* pvarRetValue, tVariant* paParams, long lSizeArray) = 0;

    virtual long ADDIN_STDCALL GetNEvents() = 0;
    virtual const WCHAR_T* ADDIN_STDCALL GetEventName(long lEventNum, long lEventAlias) = 0;
    virtual void ADDIN_STDCALL ExternalEvent(const WCHAR_T* wsSource, const WCHAR_T* wsMessage, const WCHAR_T* wsData) = 0;
    virtual void ADDIN_STDCALL SetLocale(const WCHAR_T* loc) = 0;

    virtual ~ILanguageExtenderBase() override = default;
};

using IComponentBase = ILanguageExtenderBase;
