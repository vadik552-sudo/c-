#pragma once

#include "../include_1c/bpo_methods.h"
#include "../include_1c/component_base.h"

#include <array>
#include <string>

class KaspiKassaDriver final : public IComponentBase {
public:
    KaspiKassaDriver();
    ~KaspiKassaDriver() override;

    bool_t ADDIN_STDCALL Init(void* pConnection) override;
    bool_t ADDIN_STDCALL setMemManager(void* pMemManager) override;
    long ADDIN_STDCALL GetInfo() override;
    void ADDIN_STDCALL Done() override;
    bool_t ADDIN_STDCALL RegisterExtensionAs(WCHAR_T** wsExtensionName) override;

    long ADDIN_STDCALL GetNProps() override;
    long ADDIN_STDCALL FindProp(const WCHAR_T* wsPropName) override;
    const WCHAR_T* ADDIN_STDCALL GetPropName(long lPropNum, long lPropAlias) override;
    bool_t ADDIN_STDCALL GetPropVal(long lPropNum, tVariant* pvarPropVal) override;
    bool_t ADDIN_STDCALL SetPropVal(long lPropNum, tVariant* pvarPropVal) override;
    bool_t ADDIN_STDCALL IsPropReadable(long lPropNum) override;
    bool_t ADDIN_STDCALL IsPropWritable(long lPropNum) override;

    long ADDIN_STDCALL GetNMethods() override;
    long ADDIN_STDCALL FindMethod(const WCHAR_T* wsMethodName) override;
    const WCHAR_T* ADDIN_STDCALL GetMethodName(long lMethodNum, long lMethodAlias) override;
    long ADDIN_STDCALL GetNParams(long lMethodNum) override;
    bool_t ADDIN_STDCALL GetParamDefValue(long lMethodNum, long lParamNum, tVariant* pvarParamDefValue) override;
    bool_t ADDIN_STDCALL HasRetVal(long lMethodNum) override;
    bool_t ADDIN_STDCALL CallAsProc(long lMethodNum, tVariant* paParams, long lSizeArray) override;
    bool_t ADDIN_STDCALL CallAsFunc(long lMethodNum, tVariant* pvarRetValue, tVariant* paParams, long lSizeArray) override;

    long ADDIN_STDCALL GetNEvents() override;
    const WCHAR_T* ADDIN_STDCALL GetEventName(long lEventNum, long lEventAlias) override;
    void ADDIN_STDCALL ExternalEvent(const WCHAR_T* wsSource, const WCHAR_T* wsMessage, const WCHAR_T* wsData) override;
    void ADDIN_STDCALL SetLocale(const WCHAR_T* loc) override;

private:
    bool SetStringResult(tVariant* variant, const std::u16string& value) const;
    bool SetBoolResult(tVariant* variant, bool value) const;
    bool ReadStringParam(const tVariant& value, std::u16string* out, const char16_t* name);
    bool SetParameterValue(const std::u16string& key, const tVariant& value);
    void SetLastError(const std::u16string& text);

    IMemoryManager* mem_{nullptr};
    void* connection_{nullptr};
    std::u16string baseUrl_{u"https://kaspi.kz"};
    std::u16string token_;
    std::u16string pointId_;
    std::u16string lastError_;
    std::u16string lastLog_{u""};
    std::u16string locale_{u"ru"};
    std::int32_t requestTimeoutSec_{30};
    std::int32_t pollIntervalSec_{3};
    bool verbose_{false};
    bool opened_{false};
};

extern "C" const WCHAR_T* GetComponentClassName();
