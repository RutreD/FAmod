module;
#include <Windows.h>
#include <imgui.h>
#include <rcmp.hpp>

export module core.app;

import std;
import core;
import imgui_hook;

import patch.allied_range_rings;
import patch.range_ring_stencil;
import patch.minimap_ranges;
import patch.network_stats;
import patch.health_bars;

export namespace core::app {

void Initialize() {
#if defined(_DEBUG)
  ImGuiHook::OnFrame += [] { ImGui::ShowDemoWindow(); };
#endif

  auto &registry = PatchRegistry::Instance();
  registry.RegisterPatch<AlliedRangeRingsPatch>();
  registry.RegisterPatch<RangeRingStencilPatch>();
  registry.RegisterPatch<HealthBarsPatch>();
  registry.RegisterPatch<NetworkStatsPatch>();
  registry.RegisterPatch<MinimapRangesPatch>();

  registry.LoadAll(GetSettings());
  registry.ApplyAll();

  ImGuiHook::OnInitialize += [&] {
    // Address 0x0096D090 is Forged Alliance's WndProc function
    rcmp::hook_function<0x0096D090,
                        rcmp::stdcall_t<LRESULT(HWND, UINT, WPARAM, LPARAM)>>(
        [](auto original, HWND hWnd, UINT uMsg, WPARAM wParam,
           LPARAM lParam) -> LRESULT {
          auto signal_results =
              ImGuiHook::OnWndProc.emit_iterate(hWnd, uMsg, wParam, lParam);
          if (std::ranges::contains(signal_results, true))
            return 0;
          if (ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
            return 0;
          if ((uMsg >= WM_MOUSEFIRST && uMsg <= WM_MOUSELAST) ||
              (uMsg >= WM_KEYFIRST && uMsg <= WM_KEYLAST)) {
            const auto &io = ImGui::GetIO();
            if (io.WantCaptureMouse || io.WantCaptureKeyboard)
              return 0;
          }
          return original(hWnd, uMsg, wParam, lParam);
        });

    core::ui::Initialize();
  };

  ImGuiHook::OnFrame += [] { core::ui::Render(); };
  ImGuiHook::D3D9::Install(ImGuiHook::D3D9::Method::EndScene);
}

} // namespace core::app
