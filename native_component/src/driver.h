#pragma once

#include "../include_1c/bpo_methods.h"
#include "../include_1c/component_base.h"

#include <string>
#include <vector>

class KaspiKassaDriver final : public IComponentBase, public IPropertyProfile {
public:
    KaspiKassaDriver();
    ~KaspiKassaDriver() override;

    bool_t ADDIN_STDCALL Init(void* pConnection) override;
    bool_t ADDIN_STDCALL setMemManager(void* pMemMgr) override;
    long ADDIN_STDCALL GetInfo() override;
    void ADDIN_STDCALL Done() override;
    bool_t ADDIN_STDCALL RegisterExtensionAs(WCHAR_T** wsExtensionName) override;

    long ADDIN_STDCALL GetNProps() override;
    long ADDIN_STDCALL FindProp(const WCHAR_T* wsPropName) override;
    const WCHAR_T* ADDIN_STDCALL GetPropName(long propNum, long propAlias) override;
    bool_t ADDIN_STDCALL GetPropVal(long propNum, tVariant* pvarPropVal) override;
    bool_t ADDIN_STDCALL SetPropVal(long propNum, tVariant* pvarPropVal) override;
    bool_t ADDIN_STDCALL IsPropReadable(long propNum) override;
    bool_t ADDIN_STDCALL IsPropWritable(long propNum) override;

    long ADDIN_STDCALL GetNMethods() override;
    long ADDIN_STDCALL FindMethod(const WCHAR_T* wsMethodName) override;
    const WCHAR_T* ADDIN_STDCALL GetMethodName(long methodNum, long methodAlias) override;
    long ADDIN_STDCALL GetNParams(long methodNum) override;
    bool_t ADDIN_STDCALL GetParamDefValue(long methodNum, long paramNum, tVariant* pvarParamDefValue) override;
    bool_t ADDIN_STDCALL HasRetVal(long methodNum) override;
    bool_t ADDIN_STDCALL CallAsProc(long methodNum, tVariant* paParams, long sizeArray) override;
    bool_t ADDIN_STDCALL CallAsFunc(long methodNum, tVariant* pvarRetValue, tVariant* paParams, long sizeArray) override;

    long ADDIN_STDCALL GetNEvents() override;
    const WCHAR_T* ADDIN_STDCALL GetEventName(long eventNum, long eventAlias) override;
    void ADDIN_STDCALL ExternalEvent(const WCHAR_T* wsSource, const WCHAR_T* wsMessage, const WCHAR_T* wsData) override;
    void ADDIN_STDCALL SetLocale(const WCHAR_T* loc) override;

    bool_t ADDIN_STDCALL RegisterProfileAs(WCHAR_T** wsProfileName) override;
    bool_t ADDIN_STDCALL GetPropertyProfile(WCHAR_T** wsData) override;
    bool_t ADDIN_STDCALL SetPropertyProfile(const WCHAR_T* wsData) override;

    static const char16_t* ClassName();

private:
    bool HttpGetBaseUrl();
    void Log(const std::wstring& message, bool verboseOnly = false) const;
    bool SetVariantBool(tVariant* var, bool value) const;
    bool SetVariantString(tVariant* var, const std::u16string& value) const;
    bool ReadVariantString(const tVariant& var, std::u16string* out, const char16_t* argName);
    std::u16string Utf16ToU16(const std::wstring& src) const;
    std::wstring U16ToUtf16(const std::u16string& src) const;
    std::u16string ToLower(std::u16string value) const;
    bool SetParameterInternal(const std::u16string& name, const tVariant& value);
    void SetLastError(const std::u16string& value);

private:
    IMemoryManager* memMgr_{nullptr};
    void* connection_{nullptr};
    std::u16string baseUrl_{u"https://kaspi.kz/api/v3"};
    std::u16string token_;
    std::u16string pointId_;
    long timeoutMs_{30000};
    long pollIntervalMs_{1000};
    bool verboseLog_{false};
    bool opened_{false};
    std::u16string locale_{u"en"};
    std::u16string lastError_;
    mutable std::wstring logPath_;
};
