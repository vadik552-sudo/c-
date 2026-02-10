#pragma once

#include <cstddef>
#include <cstdint>

using WCHAR_T = std::uint16_t;
using bool_t = std::int32_t;

constexpr std::uint16_t VTYPE_EMPTY = 0;
constexpr std::uint16_t VTYPE_NULL = 1;
constexpr std::uint16_t VTYPE_I2 = 2;
constexpr std::uint16_t VTYPE_I4 = 3;
constexpr std::uint16_t VTYPE_R4 = 4;
constexpr std::uint16_t VTYPE_R8 = 5;
constexpr std::uint16_t VTYPE_DATE = 7;
constexpr std::uint16_t VTYPE_BSTR = 8;
constexpr std::uint16_t VTYPE_BOOL = 11;
constexpr std::uint16_t VTYPE_I1 = 16;
constexpr std::uint16_t VTYPE_UI1 = 17;
constexpr std::uint16_t VTYPE_UI2 = 18;
constexpr std::uint16_t VTYPE_UI4 = 19;
constexpr std::uint16_t VTYPE_I8 = 20;
constexpr std::uint16_t VTYPE_UI8 = 21;
constexpr std::uint16_t VTYPE_INT = 22;
constexpr std::uint16_t VTYPE_UINT = 23;
constexpr std::uint16_t VTYPE_PWSTR = 26;
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

static_assert(sizeof(tVariant) == 24, "tVariant must be 24 bytes on x64");
static_assert(offsetof(tVariant, vt) == 16, "vt offset must be 16");
static_assert(offsetof(tVariant, value) == 0, "value offset must be 0");
