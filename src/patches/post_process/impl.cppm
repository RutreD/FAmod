module;
#include <xbyak/xbyak.h>

module patch.post_process;

import :settings;
import :utils;
import :shader;
import :renderer;
import :ui;

import std;
import core;
import imgui_hook;

namespace patch::post_process {

void OnBeforeRenderUI() {
  PostProcessRenderer::Instance().Render(ImGuiHook::D3D9::Device);
}

// Hook placed at 0x007F95AE right after 3D world rendering (terrain, meshes,
// water, effects, fog of war, refracting particles) and BEFORE strategic icons,
// unit lifebars/health bars, projectile arcs, and UI are drawn.
struct PostProcessHookStub : public Xbyak::CodeGenerator {
  explicit PostProcessHookStub(void (*callback)()) {
    // 1. Call original Moho::WRenViewport::RenderRefractingEffects(esi)
    call(reinterpret_cast<const void *>(0x007F8600));

    // 2. Run our custom post-processing on the 3D world render target
    jmp(reinterpret_cast<const void *>(callback));
  }
};

struct PatchRefractingEffectsCall : public Xbyak::CodeGenerator {
  explicit PatchRefractingEffectsCall(const void *trampoline_address)
      : Xbyak::CodeGenerator(5, reinterpret_cast<void *>(0x007F95AE)) {
    setProtectMode(PROTECT_RWE);
    call(trampoline_address);
    setProtectModeRE();
  }
};

} // namespace patch::post_process

void PostProcessPatch::Apply() {
  ImGuiHook::OnReset += [] {
    patch::post_process::PostProcessRenderer::Instance().OnReset();
  };

  static patch::post_process::PostProcessHookStub hook_stub(&patch::post_process::OnBeforeRenderUI);
  patch::post_process::PatchRefractingEffectsCall patch_call(hook_stub.getCode<const void *>());
}

void PostProcessPatch::RenderUi() {
  patch::post_process::RenderPostProcessUi();
}

void PostProcessPatch::BindSettings(SettingsBinder &b) {
  using namespace patch::post_process;
  b.Bind("PostProcess.enabled", g_settings.enabled, true);
  b.Bind("PostProcess.preset", g_settings.preset, Preset::Default);
  b.Bind("PostProcess.fxaaEnabled", g_settings.fxaa_enabled, true);
  b.Bind("PostProcess.brightness", g_settings.brightness, 0.0f);
  b.Bind("PostProcess.contrast", g_settings.contrast, 1.0f);
  b.Bind("PostProcess.saturation", g_settings.saturation, 1.0f);
  b.Bind("PostProcess.vibrance", g_settings.vibrance, 0.0f);
  b.Bind("PostProcess.gamma", g_settings.gamma, 1.0f);
  b.Bind("PostProcess.exposure", g_settings.exposure, 0.0f);
  b.Bind("PostProcess.colorTemp", g_settings.color_temp, 0.0f);
  b.Bind("PostProcess.tint", g_settings.tint, 0.0f);
  b.Bind("PostProcess.tonemapMode", g_settings.tonemap_mode, TonemapMode::None);
  b.Bind("PostProcess.splitIntensity", g_settings.split_intensity, 0.0f);
  b.Bind("PostProcess.splitBalance", g_settings.split_balance, 0.0f);
  b.Bind("PostProcess.sharpening", g_settings.sharpening, 0.0f);
  b.Bind("PostProcess.vignetteIntensity", g_settings.vignette_intensity, 0.0f);
  b.Bind("PostProcess.filmGrain", g_settings.film_grain, 0.0f);
  b.Bind("PostProcess.bleachBypass", g_settings.bleach_bypass, 0.0f);
  b.Bind("PostProcess.blackLevel", g_settings.black_level, 0.0f);
  b.Bind("PostProcess.sCurve", g_settings.s_curve, 0.0f);
  b.Bind("PostProcess.technicolor", g_settings.technicolor, 0.0f);
  b.Bind("PostProcess.dpxFilm", g_settings.dpx_film, 0.0f);
}
