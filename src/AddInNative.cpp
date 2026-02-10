#include "Component.h"

#include <new>
#include <string>

namespace {

std::wstring FromWcharT(const WCHAR_T* src) {
    if (src == nullptr) {
        return std::wstring();
    }
    std::wstring out;
    while (*src != 0) {
        out.push_back(static_cast<wchar_t>(*src));
        ++src;
    }
    return out;
}

bool CopyToMemoryManager(const std::wstring& source, WCHAR_T** out) {
    if (out == nullptr || GetMemoryManager() == nullptr) {
        return false;
    }
    void* mem = nullptr;
    const unsigned long bytes = static_cast<unsigned long>((source.size() + 1) * sizeof(WCHAR_T));
    if (!GetMemoryManager()->AllocMemory(&mem, bytes) || mem == nullptr) {
        return false;
    }

    WCHAR_T* dst = static_cast<WCHAR_T*>(mem);
    for (size_t i = 0; i < source.size(); ++i) {
        dst[i] = static_cast<WCHAR_T>(source[i]);
    }
    dst[source.size()] = 0;
    *out = dst;
    return true;
}

}  // namespace

extern "C" __declspec(dllexport) long SetGlobalMemoryManager(void* memoryManager) {
    SetMemoryManager(static_cast<IMemoryManager*>(memoryManager));
    return (GetMemoryManager() != nullptr) ? 1 : 0;
}

extern "C" __declspec(dllexport) long SetMemManager(void* memoryManager) {
    return SetGlobalMemoryManager(memoryManager);
}

extern "C" __declspec(dllexport) long GetClassNames(WCHAR_T** wsNames) {
    if (!CopyToMemoryManager(L"KaspiKassaAPIv3", wsNames)) {
        return 0;
    }
    return 1;
}

extern "C" __declspec(dllexport) long GetClassObject(const WCHAR_T* wsName, IComponentBase** pInterface) {
    if (pInterface == nullptr) {
        return 0;
    }

    const std::wstring requested = FromWcharT(wsName);
    if (!requested.empty() && requested != L"KaspiKassaAPIv3") {
        *pInterface = nullptr;
        return 0;
    }

    auto* component = new (std::nothrow) KaspiKassaComponent();
    if (component == nullptr) {
        *pInterface = nullptr;
        return 0;
    }

    *pInterface = component;
    return 1;
}

extern "C" __declspec(dllexport) long DestroyObject(IComponentBase** pInterface) {
    if (pInterface == nullptr || *pInterface == nullptr) {
        return 0;
    }

    delete *pInterface;
    *pInterface = nullptr;
    return 1;
}
