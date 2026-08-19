module;
#include <Windows.h>
#include <d3d9.h>

#include "imgui.h"
#include "imgui_hook_impl_win32.hpp"
#include "imgui_impl_dx9.h"
#include "imgui_impl_win32.h"

export module imgui_hook;
export import :signal;

import std;
export {
  using ::ImGui_ImplWin32_WndProcHandler;
}

export namespace ImGuiHook {

inline signal<void()> OnInitialize;
inline signal<void()> OnFrame;
inline signal<void()> OnReset;
inline signal<void()> OnAfterReset;

inline signal<bool(HWND, UINT, WPARAM, LPARAM)> OnWndProc;

namespace D3D9 {
enum class Method { Present, EndScene };
inline IDirect3DDevice9 *Device = nullptr;
void Install(Method method = Method::Present,
             std::uintptr_t *vtable_ = nullptr);
} // namespace D3D9

namespace Input {
inline HWND Window = nullptr;
void Install(HWND hwnd);
} // namespace Input

} // namespace ImGuiHook

namespace ImGuiHook::Input {

inline WNDPROC originalWndProc = nullptr;

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
  auto signalEmits = OnWndProc.emit_iterate(hWnd, uMsg, wParam, lParam);
  if (std::ranges::contains(signalEmits, true))
    return 0;
  if (ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
    return 0;
  if ((uMsg >= WM_MOUSEFIRST && uMsg <= WM_MOUSELAST) ||
      (uMsg >= WM_KEYFIRST && uMsg <= WM_KEYLAST)) {
    const auto &io = ImGui::GetIO();
    if (io.WantCaptureMouse || io.WantCaptureKeyboard)
      return 0;
  }
  return CallWindowProcA(originalWndProc, hWnd, uMsg, wParam, lParam);
}

void Install(HWND hwnd) {
  originalWndProc = reinterpret_cast<WNDPROC>(SetWindowLongPtrA(
      hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(&WndProc)));
}

} // namespace ImGuiHook::Input

namespace ImGuiHook::D3D9 {

constexpr std::size_t kVTableIndexReset = 16;
constexpr std::size_t kVTableIndexPresent = 17;
constexpr std::size_t kVTableIndexEndScene = 42;
constexpr std::size_t kVTableCount = 43;

struct ScopedMemoryProtect {
  void *address{nullptr};
  std::size_t size{0};
  DWORD old_protect{0};

  ScopedMemoryProtect(void *addr, std::size_t sz, DWORD new_protect)
      : address(addr), size(sz) {
    VirtualProtect(address, size, new_protect, &old_protect);
  }

  ~ScopedMemoryProtect() {
    if (address) {
      VirtualProtect(address, size, old_protect, &old_protect);
    }
  }

  ScopedMemoryProtect(const ScopedMemoryProtect &) = delete;
  ScopedMemoryProtect &operator=(const ScopedMemoryProtect &) = delete;
};

std::uintptr_t *GetVirtualTable(std::uintptr_t *default_vtable = nullptr) {
  static std::uintptr_t *vt = [&]() -> std::uintptr_t * {
    auto scan_d3d9_dll = []() -> std::uintptr_t * {
      std::wstring sys_dir;
      sys_dir.resize_and_overwrite(
          MAX_PATH, [](wchar_t *buf, std::size_t count) {
            return static_cast<std::size_t>(
                GetSystemDirectoryW(buf, static_cast<UINT>(count)));
          });
      if (sys_dir.empty())
        return nullptr;
      sys_dir += L"\\d3d9.dll";

      HMODULE hD3D9 = LoadLibraryW(sys_dir.c_str());
      if (!hD3D9)
        return nullptr;

      auto dos = reinterpret_cast<const IMAGE_DOS_HEADER *>(hD3D9);
      auto nt = reinterpret_cast<const IMAGE_NT_HEADERS *>(
          reinterpret_cast<const std::uint8_t *>(hD3D9) + dos->e_lfanew);
      auto base = reinterpret_cast<const std::uint8_t *>(hD3D9);
      std::span<const std::uint8_t> bytes(base, nt->OptionalHeader.SizeOfImage);

      if (bytes.size() < 16)
        return nullptr;

      for (std::size_t i = 0; i + 14 < bytes.size(); ++i) {
        auto p16 = reinterpret_cast<const std::uint16_t *>(&bytes[i]);
        if (p16[0] == 0x06C7 && p16[3] == 0x8689 && p16[6] == 0x8689) {
          return *reinterpret_cast<std::uintptr_t *const *>(&bytes[i + 2]);
        }
      }
      return nullptr;
    };

    if (auto found = scan_d3d9_dll())
      return found;

    if (default_vtable == nullptr)
      throw std::runtime_error(
          "ImGuiHook::D3D9: Failed to find Direct3D9 virtual table");
    return default_vtable;
  }();

  return vt;
}

void Init(IDirect3DDevice9 *pDevice) {
  Device = pDevice;
  D3DDEVICE_CREATION_PARAMETERS params{};
  pDevice->GetCreationParameters(&params);
  ImGui::CreateContext();
  ImGui_ImplWin32_Init(params.hFocusWindow);
  ImGui_ImplDX9_Init(pDevice);
  Input::Window = params.hFocusWindow;
  OnInitialize.emit();
}

void RenderFrame() {
  ImGui_ImplDX9_NewFrame();
  ImGui_ImplWin32_NewFrame();
  ImGui::NewFrame();
  OnFrame.emit();
  ImGui::EndFrame();
  ImGui::Render();
  ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
}

using PresentFn = HRESULT(__stdcall *)(IDirect3DDevice9 *, const RECT *,
                                       const RECT *, HWND, const RGNDATA *);
inline PresentFn originalPresent = nullptr;

HRESULT __stdcall PresentHooked(IDirect3DDevice9 *pDevice,
                                const RECT *pSourceRect, const RECT *pDestRect,
                                HWND hDestWindowOverride,
                                const RGNDATA *pDirtyRegion) {
  if (Device == nullptr) {
    Init(pDevice);
  }

  RenderFrame();

  return originalPresent(pDevice, pSourceRect, pDestRect, hDestWindowOverride,
                         pDirtyRegion);
}

using EndSceneFn = HRESULT(__stdcall *)(IDirect3DDevice9 *);
inline EndSceneFn originalEndScene = nullptr;

HRESULT __stdcall EndSceneHooked(IDirect3DDevice9 *pDevice) {
  if (Device == nullptr) {
    Init(pDevice);
  }
  RenderFrame();
  return originalEndScene(pDevice);
}

using ResetFn = HRESULT(__stdcall *)(IDirect3DDevice9 *,
                                     D3DPRESENT_PARAMETERS *);
inline ResetFn originalReset = nullptr;

HRESULT __stdcall ResetHooked(IDirect3DDevice9 *pDevice,
                              D3DPRESENT_PARAMETERS *pPresentationParameters) {
  OnReset.emit();
  ImGui_ImplDX9_InvalidateDeviceObjects();
  auto result = originalReset(pDevice, pPresentationParameters);
  OnAfterReset.emit();
  return result;
}

void Install(Method method, std::uintptr_t *vtable_) {
  std::uintptr_t *vtable = GetVirtualTable(vtable_);
  ScopedMemoryProtect protect(vtable, sizeof(std::uintptr_t) * kVTableCount,
                              PAGE_READWRITE);

  if (method == Method::Present) {
    originalPresent = reinterpret_cast<PresentFn>(vtable[kVTableIndexPresent]);
    vtable[kVTableIndexPresent] =
        reinterpret_cast<std::uintptr_t>(&PresentHooked);
  } else if (method == Method::EndScene) {
    originalEndScene =
        reinterpret_cast<EndSceneFn>(vtable[kVTableIndexEndScene]);
    vtable[kVTableIndexEndScene] =
        reinterpret_cast<std::uintptr_t>(&EndSceneHooked);
  }
  originalReset = reinterpret_cast<ResetFn>(vtable[kVTableIndexReset]);
  vtable[kVTableIndexReset] = reinterpret_cast<std::uintptr_t>(&ResetHooked);
}

} // namespace ImGuiHook::D3D9
