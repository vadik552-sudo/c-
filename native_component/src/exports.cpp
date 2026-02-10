#include "driver.h"

#include <new>
#include <cstddef>
#include <cstdint>

namespace {

IMemoryManager* gMemMgr = nullptr;

int CopyStringToMemoryManager(const char16_t* src, WCHAR_T** out) {
    if (gMemMgr == nullptr || out == nullptr || src == nullptr) {
        return -1;
    }

    std::size_t len = 0;
    while (src[len] != 0) {
        ++len;
    }

    void* mem = nullptr;
    const std::uint32_t bytes = static_cast<std::uint32_t>((len + 1) * sizeof(WCHAR_T));
    if (!gMemMgr->AllocMemory(&mem, bytes) || mem == nullptr) {
        return -1;
    }

    auto* dst = static_cast<WCHAR_T*>(mem);
    for (std::size_t i = 0; i < len; ++i) {
        dst[i] = static_cast<WCHAR_T>(src[i]);
    }
    dst[len] = 0;
    *out = dst;
    return 0;
}

bool MatchClass(const WCHAR_T* clsName) {
    if (clsName == nullptr || *clsName == 0) {
        return true;
    }
    const char16_t* expected = KaspiKassaDriver::ClassName();
    std::size_t i = 0;
    while (expected[i] != 0 && clsName[i] != 0) {
        if (static_cast<WCHAR_T>(expected[i]) != clsName[i]) {
            return false;
        }
        ++i;
    }
    return expected[i] == 0 && clsName[i] == 0;
}

}  // namespace

extern "C" __declspec(dllexport) long ADDIN_STDCALL GetClassObject(const WCHAR_T* clsName, IComponentBase** pIntf) {
    if (pIntf == nullptr) {
        return -1;
    }
    *pIntf = nullptr;

    if (!MatchClass(clsName)) {
        return -1;
    }

    auto* driver = new (std::nothrow) KaspiKassaDriver();
    if (driver == nullptr) {
        return -1;
    }

    if (!driver->setMemManager(gMemMgr)) {
        delete driver;
        return -1;
    }

    *pIntf = driver;
    return 0;
}

extern "C" __declspec(dllexport) long ADDIN_STDCALL DestroyObject(IComponentBase** pIntf) {
    if (pIntf == nullptr || *pIntf == nullptr) {
        return -1;
    }
    delete *pIntf;
    *pIntf = nullptr;
    return 0;
}

extern "C" __declspec(dllexport) long ADDIN_STDCALL GetClassNames(WCHAR_T** names, long* count) {
    if (count == nullptr) {
        return -1;
    }
    *count = 0;

    if (CopyStringToMemoryManager(KaspiKassaDriver::ClassName(), names) != 0) {
        return -1;
    }

    *count = 1;
    return 0;
}

extern "C" __declspec(dllexport) void ADDIN_STDCALL SetGlobalMemoryManager(void* pMemMgr) {
    gMemMgr = static_cast<IMemoryManager*>(pMemMgr);
}

extern "C" __declspec(dllexport) void ADDIN_STDCALL SetMemManager(void* pMemMgr) {
    SetGlobalMemoryManager(pMemMgr);
}
