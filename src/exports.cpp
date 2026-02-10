#include "driver.h"

#include <new>
#include <cstddef>

namespace {

IMemoryManager* gMemoryManager = nullptr;

const WCHAR_T* GetClassName() {
    return GetComponentClassName();
}

bool CopyClassName(WCHAR_T** out) {
    if (out == nullptr || gMemoryManager == nullptr) {
        return false;
    }

    const WCHAR_T* src = GetClassName();
    size_t len = 0;
    while (src[len] != 0) {
        ++len;
    }

    void* raw = nullptr;
    const auto bytes = static_cast<unsigned long>((len + 1) * sizeof(WCHAR_T));
    if (!gMemoryManager->AllocMemory(&raw, bytes) || raw == nullptr) {
        return false;
    }

    auto* dst = static_cast<WCHAR_T*>(raw);
    for (size_t i = 0; i < len; ++i) {
        dst[i] = src[i];
    }
    dst[len] = 0;
    *out = dst;
    return true;
}

bool ClassNameMatches(const WCHAR_T* requested) {
    if (requested == nullptr || *requested == 0) {
        return true;
    }

    const WCHAR_T* expected = GetClassName();
    while (*requested != 0 && *expected != 0) {
        if (*requested != *expected) {
            return false;
        }
        ++requested;
        ++expected;
    }
    return *requested == 0 && *expected == 0;
}

}  // namespace

extern "C" __declspec(dllexport) long ADDIN_STDCALL SetGlobalMemoryManager(void* memoryManager) {
    gMemoryManager = static_cast<IMemoryManager*>(memoryManager);
    return gMemoryManager != nullptr ? 1 : 0;
}

extern "C" __declspec(dllexport) long ADDIN_STDCALL SetMemManager(void* memoryManager) {
    return SetGlobalMemoryManager(memoryManager);
}

extern "C" __declspec(dllexport) long ADDIN_STDCALL GetClassNames(WCHAR_T** wsNames) {
    return CopyClassName(wsNames) ? 1 : 0;
}

extern "C" __declspec(dllexport) long ADDIN_STDCALL GetClassObject(const WCHAR_T* wsName, IComponentBase** pInterface) {
    if (pInterface == nullptr) {
        return 0;
    }
    *pInterface = nullptr;

    if (!ClassNameMatches(wsName)) {
        return 0;
    }

    auto* instance = new (std::nothrow) KaspiKassaDriver();
    if (instance == nullptr) {
        return 0;
    }

    if (!instance->setMemManager(gMemoryManager)) {
        delete instance;
        return 0;
    }

    *pInterface = instance;
    return 1;
}

extern "C" __declspec(dllexport) long ADDIN_STDCALL DestroyObject(IComponentBase** pInterface) {
    if (pInterface == nullptr || *pInterface == nullptr) {
        return 0;
    }

    delete *pInterface;
    *pInterface = nullptr;
    return 1;
}
