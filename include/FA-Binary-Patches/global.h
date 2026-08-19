#pragma once

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <string_view>
#include <type_traits>

#define SHARED extern "C"

#define GPtr(addr, type) \
  (*(type*)addr)

#define WDecl(addr, type) \
  ((type)*(uintptr_t*)addr)

#define VALIDATE_SIZE(struc, size) \
  static_assert(sizeof(struc) == size, "Invalid structure size of " #struc)

#if defined(__clang__)
#pragma clang diagnostic ignored "-Winvalid-offsetof"
#elif defined(__GNUC__)
#pragma GCC diagnostic ignored "-Winvalid-offsetof"
#endif

#define VALIDATE_OFFSET(struc, member, offset) \
  static_assert(offsetof(struc, member) == offset, "Invalid offset of " #member " in " #struc)

// Forward declarations for global engine singletons
struct CSimDriver;
struct SWldSessionInfo;
struct CWldSession;
struct Sim;
struct EntityCategoryTypeInfo;
struct CAiBrainTypeInfo;
struct CUIManager;
struct EngineStats;
struct WRenViewport;
struct LuaState;
struct Device;

template <typename T>
[[nodiscard]] constexpr T& MemoryRef(uintptr_t addr) noexcept {
  return *reinterpret_cast<T*>(addr);
}

template <typename T>
[[nodiscard]] constexpr T* MemoryPtr(uintptr_t addr) noexcept {
  return reinterpret_cast<T*>(addr);
}

template <typename T, uintptr_t Addr>
struct GlobalRef {
  [[nodiscard]] constexpr operator T&() const noexcept {
    return *reinterpret_cast<T*>(Addr);
  }

  [[nodiscard]] constexpr T& get() const noexcept {
    return *reinterpret_cast<T*>(Addr);
  }

  [[nodiscard]] constexpr auto operator->() const noexcept {
    return *reinterpret_cast<T*>(Addr);
  }

  [[nodiscard]] constexpr auto& operator*() const noexcept {
    return **reinterpret_cast<T*>(Addr);
  }

  [[nodiscard]] constexpr T* operator&() const noexcept {
    return reinterpret_cast<T*>(Addr);
  }

  template <typename U>
  constexpr T& operator=(U&& val) const noexcept {
    return (*reinterpret_cast<T*>(Addr) = static_cast<U&&>(val));
  }
};

// Global engine instances as zero-overhead constexpr proxies
inline constexpr GlobalRef<CSimDriver*, 0x10C4F50>             g_CSimDriver{};
inline constexpr GlobalRef<SWldSessionInfo*, 0x10C4F58>        g_SWldSessionInfo{};
inline constexpr GlobalRef<CWldSession*, 0x10A6470>            g_CWldSession{};
inline constexpr GlobalRef<Sim*, 0x10A63F0>                    g_Sim{};
inline constexpr GlobalRef<EntityCategoryTypeInfo*, 0x10C6E70> g_EntityCategoryTypeInfo{};
inline constexpr GlobalRef<CAiBrainTypeInfo*, 0x10C6FA0>       g_CAiBrainTypeInfo{};
inline constexpr GlobalRef<CUIManager*, 0x10A6450>             g_CUIManager{};
inline constexpr GlobalRef<EngineStats*, 0x10A67B8>            g_EngineStats{};
inline constexpr GlobalRef<WRenViewport*, 0x10C7C28>           g_WRenViewport{};
inline constexpr GlobalRef<LuaState*, 0x10A6478>               g_ConsoleLuaState{};
inline constexpr GlobalRef<Device*, 0x0F8E284>                 g_Device{};

// Engine functions mapped to game binary addresses
inline const auto AbortF          = reinterpret_cast<void(__cdecl *)(const wchar_t *fmt, ...)>(0x9C4940);
inline const auto LogF            = reinterpret_cast<int(__cdecl *)(const char *fmt, ...)>(0x937CB0);
inline const auto WarningF        = reinterpret_cast<int(__cdecl *)(const char *fmt, ...)>(0x937D30);
inline const auto SpewF           = reinterpret_cast<int(__cdecl *)(const char *fmt, ...)>(0x937C30);
inline const auto ConsoleLogF     = reinterpret_cast<int(__cdecl *)(const char *fmt, ...)>(0x41C990);
inline const auto FileWrite       = reinterpret_cast<int(__cdecl *)(int fileIndex, const char *str, int strlen)>(0xA9B4E6);
inline const auto CopyToClipboard = reinterpret_cast<bool(__cdecl *)(const wchar_t *str)>(0x4F2730);
inline const auto shi_new         = reinterpret_cast<void*(__cdecl *)(size_t size)>(0xA825B9);

// Game-specific CRT functions (to ensure game heap ABI compatibility)
inline const auto realloc   = reinterpret_cast<void*(__cdecl *)(void *ptr, size_t new_size)>(0x957B00);
inline const auto malloc    = reinterpret_cast<void*(__cdecl *)(size_t size)>(0x958B20);
inline const auto free      = reinterpret_cast<void(__cdecl *)(void *ptr)>(0x958C40);
inline const auto msize     = reinterpret_cast<size_t(__cdecl *)(void *memblock)>(0x957EA0);
inline const auto calloc    = reinterpret_cast<void*(__cdecl *)(size_t num, size_t size)>(0x957AB0);
inline const auto memset    = reinterpret_cast<void*(__cdecl *)(void *dest, int ch, size_t count)>(0xA89110);
inline const auto memcpy    = reinterpret_cast<void*(__cdecl *)(void *dest, const void *src, size_t count)>(0xA89190);
inline const auto strlen    = reinterpret_cast<size_t(__cdecl *)(const char *str)>(0xA94450);
inline const auto strcmp    = reinterpret_cast<int(__cdecl *)(const char *str1, const char *str2)>(0xAA549E);
inline const auto sprintf_s = reinterpret_cast<int(__cdecl *)(char *Buffer, size_t BufferCount, const char *Format, ...)>(0xA82F32);
inline const auto sqrtf     = reinterpret_cast<float(__cdecl *)(float)>(0x452FC0);

#define DebugLog(_s) LogF("%s", (_s))

template<typename T>
struct basic_string;
using string = basic_string<char>;
using wstring = basic_string<wchar_t>;

#define SSO_bytes 0x10ul
template <typename T> struct basic_string {
  static constexpr uint32_t sso_size = SSO_bytes / sizeof(T);
  uint32_t ptr;
  union {
    T str[sso_size];
    T *_data;
  };
  uint32_t strLen;
  uint32_t capacity;

  basic_string() noexcept {
    ptr = 0;
    str[0] = T(0);
    strLen = 0;
    capacity = sso_size - 1;
  }

  basic_string(const T *s) {
    if constexpr (std::is_same_v<char, T>)
      reinterpret_cast<void(__thiscall *)(void *, const char *)>(0x405520)(this, s);
    else if constexpr (std::is_same_v<wchar_t, T>)
      reinterpret_cast<void(__thiscall *)(void *, const wchar_t *)>(0x938FD0)(this, s);
    else
      static_assert(false, "Unknown type T.");
  }

  [[nodiscard]] inline const T *data() const noexcept { return capacity < sso_size ? static_cast<const T*>(str) : _data; }
  [[nodiscard]] inline T *data() noexcept { return capacity < sso_size ? static_cast<T*>(str) : _data; }
  [[nodiscard]] inline uint32_t size() const noexcept { return strLen; }
  [[nodiscard]] inline uint32_t length() const noexcept { return strLen; }
  [[nodiscard]] inline bool empty() const noexcept { return strLen == 0; }

  [[nodiscard]] inline std::basic_string_view<T> view() const noexcept {
    return {data(), static_cast<std::size_t>(strLen)};
  }

  constexpr operator std::basic_string_view<T>() const noexcept {
    return view();
  }

  inline void clear() noexcept {
    if (capacity >= sso_size) {
      free(data());
    }
    ptr = 0;
    str[0] = T(0);
    strLen = 0;
    capacity = sso_size - 1;
  }

  ~basic_string() { clear(); }
};

VALIDATE_SIZE(string, 0x1C);
static_assert(string::sso_size == 0x10);
VALIDATE_SIZE(wstring, 0x1C);
static_assert(wstring::sso_size == 0x8);

template <typename T>
struct Result {
  T *object = nullptr;
  const char *reason = nullptr;

  constexpr static Result<T> Fail(const char *reason) noexcept { return {nullptr, reason}; }
  constexpr static Result<T> Success(void *data) noexcept { return {static_cast<T*>(data), nullptr}; }

  [[nodiscard]] inline bool IsFail() const noexcept { return reason != nullptr; }
  [[nodiscard]] inline bool IsSuccess() const noexcept { return reason == nullptr; }
};

template <typename T = void*>
[[nodiscard]] inline T Offset(void *ptr, size_t offset) noexcept {
  return reinterpret_cast<T>(reinterpret_cast<char*>(ptr) + offset);
}

template <typename T = void*>
[[nodiscard]] inline const T Offset(const void *ptr, size_t offset) noexcept {
  return reinterpret_cast<const T>(reinterpret_cast<const char*>(ptr) + offset);
}

template <typename T>
[[nodiscard]] inline T &GetField(void *ptr, size_t offset) noexcept {
  return *Offset<T *>(ptr, offset);
}

template <typename T>
[[nodiscard]] inline const T &GetField(const void *ptr, size_t offset) noexcept {
  return *Offset<const T *>(ptr, offset);
}

