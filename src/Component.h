#pragma once

#include "AddInDefBase.h"

#include <string>
#include <vector>

class KaspiKassaComponent final : public IComponentBase {
public:
    KaspiKassaComponent();
    ~KaspiKassaComponent() override;

    bool_t Init(void* pConnection) override;
    bool_t setMemManager(void* pMemManager) override;
    long GetInfo() override;
    void Done() override;
    bool_t RegisterExtensionAs(WCHAR_T** wsExtensionName) override;

    long GetNProps() override;
    long FindProp(const WCHAR_T* wsPropName) override;
    const WCHAR_T* GetPropName(long lPropNum, long lPropAlias) override;
    bool_t GetPropVal(const long lPropNum, tVariant* pvarPropVal) override;
    bool_t SetPropVal(const long lPropNum, tVariant* pvarPropVal) override;
    bool_t IsPropReadable(const long lPropNum) override;
    bool_t IsPropWritable(const long lPropNum) override;

    long GetNMethods() override;
    long FindMethod(const WCHAR_T* wsMethodName) override;
    const WCHAR_T* GetMethodName(const long lMethodNum, const long lMethodAlias) override;
    long GetNParams(const long lMethodNum) override;
    bool_t GetParamDefValue(const long lMethodNum, const long lParamNum, tVariant* pvarParamDefValue) override;
    bool_t HasRetVal(const long lMethodNum) override;
    bool_t CallAsProc(const long lMethodNum, tVariant* paParams, const long lSizeArray) override;
    bool_t CallAsFunc(const long lMethodNum, tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) override;

    long GetNEvents() override;
    const WCHAR_T* GetEventName(const long lEventNum, const long lEventAlias) override;
    void ExternalEvent(const WCHAR_T* wsSource, const WCHAR_T* wsMessage, const WCHAR_T* wsData) override;
    void SetLocale(const WCHAR_T* loc) override;

public:
    enum class MethodId : long {
        GetVersion = 0,
        GetDescription,
        GetParameters,
        SetParameter,
        Open,
        Close,
        DeviceTest,
        GetAdditionalActions,
        DoAdditionalAction,
        GetLastError,
        Count
    };

    enum class PropId : long {
        IsOpened = 0,
        LastError,
        Count
    };

    struct MethodInfo {
        const wchar_t* ru;
        const wchar_t* en;
        long params;
        bool hasRet;
    };

    struct PropInfo {
        const wchar_t* ru;
        const wchar_t* en;
        bool readable;
        bool writable;
    };

    bool SetVariantString(tVariant* variant, const std::wstring& value) const;
    bool SetVariantBool(tVariant* variant, bool value) const;
    bool SetVariantInt(tVariant* variant, std::int32_t value) const;
    static std::wstring ToWString(const WCHAR_T* value);
    static bool EqualsNoCase(const std::wstring& left, const wchar_t* right);
    bool TryReadVariantString(const tVariant& value, std::wstring* out, const wchar_t* argName);
    bool SetParameterValue(const std::wstring& key, const tVariant& value);
    bool GetParameterValue(const std::wstring& key, std::wstring* out) const;
    bool DoHttpTest(std::wstring* responseText);

    void SetLastError(const std::wstring& error);

private:
    void* connection_;
    bool opened_;
    std::wstring locale_;
    std::wstring baseUrl_;
    std::wstring token_;
    std::wstring pointId_;
    std::int32_t requestTimeoutSec_;
    std::int32_t pollIntervalSec_;
    bool verboseLog_;
    std::wstring lastError_;
    std::wstring lastLog_;
};

IMemoryManager* GetMemoryManager();
void SetMemoryManager(IMemoryManager* memoryManager);
