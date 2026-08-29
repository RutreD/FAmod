module;
#include <Windows.h>
#include <imgui.h>
#include <rcmp.hpp>

export module core.app;

import std;
import core;
import imgui_hook;

import patch.range_rings;
import patch.range_ring_stencil;
import patch.minimap_ranges;
import patch.network_stats;
import patch.health_bars;
import patch.share_factory_assist;
import patch.post_process;
import patch.water;
import patch.strategic_icons;

export namespace core::app {

void Initialize() {
#if defined(_DEBUG)
  ImGuiHook::OnFrame += [] { ImGui::ShowDemoWindow(); };
#endif

  core::events::Initialize();

  auto &registry = PatchRegistry::Instance();
  registry.RegisterPatch<RangeRingsPatch>();
  registry.RegisterPatch<RangeRingStencilPatch>();
  registry.RegisterPatch<HealthBarsPatch>();
  registry.RegisterPatch<NetworkStatsPatch>();
  registry.RegisterPatch<MinimapRangesPatch>();
  registry.RegisterPatch<ShareFactoryAssistPatch>();
  registry.RegisterPatch<PostProcessPatch>();
  registry.RegisterPatch<WaterPatch>();
  // registry.RegisterPatch<StrategicIconsPatch>();

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

          if (core::ui::IsOpen()) {
            if (ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
              return 0;
            auto &io = ImGui::GetIO();
            if (uMsg >= WM_KEYFIRST && uMsg <= WM_KEYLAST) {
              if (io.WantCaptureKeyboard)
                return 0;
            }
            if (uMsg >= WM_MOUSEFIRST && uMsg <= WM_MOUSELAST) {
              if (io.WantCaptureMouse)
                return 0;
            }
          }

          return original(hWnd, uMsg, wParam, lParam);
        });

    core::ui::Initialize();
  };

  ImGuiHook::OnFrame += [] { core::ui::Render(); };
  ImGuiHook::D3D9::Install(ImGuiHook::D3D9::Method::EndScene);
}

} // namespace core::app
