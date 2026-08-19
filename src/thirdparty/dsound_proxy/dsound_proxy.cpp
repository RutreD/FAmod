// clang-format off
#include <Windows.h>
#include <mmsystem.h>
#include <dsound.h>
// clang-format on
import std;

namespace proxy {

struct OriginalFunctions {
  decltype(&::DirectSoundCreate) direct_sound_create{nullptr};
  decltype(&::DirectSoundEnumerateA) direct_sound_enumerate_a{nullptr};
  decltype(&::DirectSoundEnumerateW) direct_sound_enumerate_w{nullptr};
  decltype(&::DirectSoundCaptureCreate) direct_sound_capture_create{nullptr};
  decltype(&::DirectSoundCaptureEnumerateA) direct_sound_capture_enumerate_a{
      nullptr};
  decltype(&::DirectSoundCaptureEnumerateW) direct_sound_capture_enumerate_w{
      nullptr};
  decltype(&::DirectSoundCreate8) direct_sound_create8{nullptr};
  decltype(&::DirectSoundCaptureCreate8) direct_sound_capture_create8{nullptr};
  decltype(&::DirectSoundFullDuplexCreate) direct_sound_full_duplex_create{
      nullptr};
  decltype(&::GetDeviceID) get_device_id{nullptr};

  HRESULT(WINAPI *dll_can_unload_now)() { nullptr };
  HRESULT(WINAPI *dll_get_class_object)(REFCLSID, REFIID, LPVOID *) { nullptr };

  [[nodiscard]] static OriginalFunctions &get() noexcept {
    static OriginalFunctions instance;
    return instance;
  }

private:
  HMODULE original_dll{nullptr};

  OriginalFunctions() {
    std::wstring path;
    path.resize_and_overwrite(MAX_PATH, [](wchar_t *buf, std::size_t buf_size) {
      const UINT len = GetSystemDirectoryW(buf, static_cast<UINT>(buf_size));
      if (len == 0 || len >= buf_size)
        return 0u;
      return len;
    });

    if (path.empty())
      return;
    path += L"\\dsound.dll";

    original_dll = LoadLibraryW(path.c_str());
    if (!original_dll)
      return;

    auto load = [this]<typename Fn>(Fn &fn, const char *name) {
      fn = reinterpret_cast<Fn>(GetProcAddress(original_dll, name));
    };

    load(direct_sound_create, "DirectSoundCreate");
    load(direct_sound_enumerate_a, "DirectSoundEnumerateA");
    load(direct_sound_enumerate_w, "DirectSoundEnumerateW");
    load(direct_sound_capture_create, "DirectSoundCaptureCreate");
    load(direct_sound_capture_enumerate_a, "DirectSoundCaptureEnumerateA");
    load(direct_sound_capture_enumerate_w, "DirectSoundCaptureEnumerateW");
    load(direct_sound_create8, "DirectSoundCreate8");
    load(direct_sound_capture_create8, "DirectSoundCaptureCreate8");
    load(direct_sound_full_duplex_create, "DirectSoundFullDuplexCreate");
    load(get_device_id, "GetDeviceID");
    load(dll_can_unload_now, "DllCanUnloadNow");
    load(dll_get_class_object, "DllGetClassObject");
  }

  ~OriginalFunctions() {
    if (original_dll) {
      FreeLibrary(original_dll);
    }
  }

  OriginalFunctions(const OriginalFunctions &) = delete;
  OriginalFunctions &operator=(const OriginalFunctions &) = delete;
  OriginalFunctions(OriginalFunctions &&) = delete;
  OriginalFunctions &operator=(OriginalFunctions &&) = delete;
};

} // namespace proxy

extern "C" {

HRESULT WINAPI DirectSoundCreate(LPCGUID pcGuidDevice, LPDIRECTSOUND *ppDS,
                                 LPUNKNOWN pUnkOuter) {
  const auto func = proxy::OriginalFunctions::get().direct_sound_create;
  return func ? func(pcGuidDevice, ppDS, pUnkOuter) : DSERR_GENERIC;
}

HRESULT WINAPI DirectSoundEnumerateA(LPDSENUMCALLBACKA pDSEnumCallback,
                                     LPVOID pContext) {
  const auto func = proxy::OriginalFunctions::get().direct_sound_enumerate_a;
  return func ? func(pDSEnumCallback, pContext) : DSERR_GENERIC;
}

HRESULT WINAPI DirectSoundEnumerateW(LPDSENUMCALLBACKW pDSEnumCallback,
                                     LPVOID pContext) {
  const auto func = proxy::OriginalFunctions::get().direct_sound_enumerate_w;
  return func ? func(pDSEnumCallback, pContext) : DSERR_GENERIC;
}

HRESULT WINAPI DirectSoundCaptureCreate(LPCGUID pcGuidDevice,
                                        LPDIRECTSOUNDCAPTURE *ppDSC,
                                        LPUNKNOWN pUnkOuter) {
  const auto func = proxy::OriginalFunctions::get().direct_sound_capture_create;
  return func ? func(pcGuidDevice, ppDSC, pUnkOuter) : DSERR_GENERIC;
}

HRESULT WINAPI DirectSoundCaptureEnumerateA(LPDSENUMCALLBACKA pDSEnumCallback,
                                            LPVOID pContext) {
  const auto func =
      proxy::OriginalFunctions::get().direct_sound_capture_enumerate_a;
  return func ? func(pDSEnumCallback, pContext) : DSERR_GENERIC;
}

HRESULT WINAPI DirectSoundCaptureEnumerateW(LPDSENUMCALLBACKW pDSEnumCallback,
                                            LPVOID pContext) {
  const auto func =
      proxy::OriginalFunctions::get().direct_sound_capture_enumerate_w;
  return func ? func(pDSEnumCallback, pContext) : DSERR_GENERIC;
}

HRESULT WINAPI DirectSoundCreate8(LPCGUID pcGuidDevice, LPDIRECTSOUND8 *ppDS8,
                                  LPUNKNOWN pUnkOuter) {
  const auto func = proxy::OriginalFunctions::get().direct_sound_create8;
  return func ? func(pcGuidDevice, ppDS8, pUnkOuter) : DSERR_GENERIC;
}

HRESULT WINAPI DirectSoundCaptureCreate8(LPCGUID pcGuidDevice,
                                         LPDIRECTSOUNDCAPTURE8 *ppDSC8,
                                         LPUNKNOWN pUnkOuter) {
  const auto func =
      proxy::OriginalFunctions::get().direct_sound_capture_create8;
  return func ? func(pcGuidDevice, ppDSC8, pUnkOuter) : DSERR_GENERIC;
}

HRESULT WINAPI DirectSoundFullDuplexCreate(
    LPCGUID pcGuidCaptureDevice, LPCGUID pcGuidRenderDevice,
    LPCDSCBUFFERDESC pcDSCBufferDesc, LPCDSBUFFERDESC pcDSBufferDesc, HWND hWnd,
    DWORD dwLevel, LPDIRECTSOUNDFULLDUPLEX *ppDSFD,
    LPDIRECTSOUNDCAPTUREBUFFER8 *ppDSCBuffer8,
    LPDIRECTSOUNDBUFFER8 *ppDSBuffer8, LPUNKNOWN pUnkOuter) {
  const auto func =
      proxy::OriginalFunctions::get().direct_sound_full_duplex_create;
  return func ? func(pcGuidCaptureDevice, pcGuidRenderDevice, pcDSCBufferDesc,
                     pcDSBufferDesc, hWnd, dwLevel, ppDSFD, ppDSCBuffer8,
                     ppDSBuffer8, pUnkOuter)
              : DSERR_GENERIC;
}

HRESULT WINAPI GetDeviceID(LPCGUID pGuidSrc, LPGUID pGuidDest) {
  const auto func = proxy::OriginalFunctions::get().get_device_id;
  return func ? func(pGuidSrc, pGuidDest) : DSERR_GENERIC;
}

HRESULT WINAPI DllCanUnloadNow() {
  const auto func = proxy::OriginalFunctions::get().dll_can_unload_now;
  return func ? func() : S_FALSE;
}

HRESULT WINAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID *ppv) {
  const auto func = proxy::OriginalFunctions::get().dll_get_class_object;
  return func ? func(rclsid, riid, ppv) : CLASS_E_CLASSNOTAVAILABLE;
}

} // extern "C"
