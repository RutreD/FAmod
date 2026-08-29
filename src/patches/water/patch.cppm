module;
#include <Windows.h>
#include <xbyak/xbyak.h>

export module patch.water;

import std;
import core;
import fa;
import :shader;

export class WaterPatch : public IPatch {
public:
  inline static bool enabled_{false};
  inline static bool override_shader_{true};
  inline static bool reflect_projectiles_{true};

  [[nodiscard]] std::string_view Name() const noexcept override {
    return tr("Mirror Water Reflections",
              {{Language::Russian, "Зеркальные отражения в воде"},
               {Language::Chinese, "水面镜像反射"}});
  }

  [[nodiscard]] std::string_view Description() const noexcept override {
    return tr(
        "Configures screen-space water mirror reflections (SSR) and "
        "projectile/laser reflection rendering pipeline.",
        {{Language::Russian,
          "Настройка зеркальных экранных отражений в воде (SSR) и конвейера "
          "отражения снарядов, лазеров и эффектов."},
         {Language::Chinese,
          "配置屏幕空间水面镜像反射 (SSR) 以及弹道、激光和特效的反射渲染管线。"}});
  }

  void Apply() override;
  void RenderUi() override;

  void BindSettings(SettingsBinder &b) override {
    b.Bind("Water.enabled", enabled_, false);
    b.Bind("Water.overrideShader", override_shader_, true);
    b.Bind("Water.reflectProjectiles", reflect_projectiles_, true);
  }
};

namespace patch::water {

// -------------------------------------------------------------------
// 1. Hook RenderCopyForRefraction at 0x007F94B3
// Render projectiles and weapon effects right before Refraction copy
// -------------------------------------------------------------------
struct RefractionCopyHookStub : public Xbyak::CodeGenerator {
  explicit RefractionCopyHookStub() {
    pushad();

    // 1. RenderMeshes(viewport, 0x24) -> retn 4 pops 0x24
    push(0x24);
    mov(eax, ebp);
    mov(ecx, 0x007F8290);
    call(ecx);

    // 2. RenderEffects(viewport, 0) -> retn 4 pops 0
    push(0);
    mov(edi, ebp);
    mov(ecx, 0x007F8560);
    call(ecx);

    // 3. RenderMeshes(viewport, 0x28) -> retn 4 pops 0x28
    push(0x28);
    mov(eax, ebp);
    mov(ecx, 0x007F8290);
    call(ecx);

    popad();

    // Original Moho::WRenViewport::RenderCopyForRefraction(0, viewport)
    xor_(cl, cl);
    mov(edx, ebp);
    mov(eax, 0x007F83F0);
    call(eax);

    ret();
  }
};

struct PatchRefractionCopyCall : public Xbyak::CodeGenerator {
  explicit PatchRefractionCopyCall(const void *trampoline_address)
      : Xbyak::CodeGenerator(5, reinterpret_cast<void *>(0x007F94B3)) {
    setProtectMode(PROTECT_RWE);
    call(trampoline_address);
    setProtectModeRE();
  }
};

// -------------------------------------------------------------------
// 2. Skip original late projectile pass at 0x007F9536
// -------------------------------------------------------------------
struct PatchOriginalProjectilesSkip : public Xbyak::CodeGenerator {
  explicit PatchOriginalProjectilesSkip()
      : Xbyak::CodeGenerator(27, reinterpret_cast<void *>(0x007F9536)) {
    setProtectMode(PROTECT_RWE);
    jmp(reinterpret_cast<const void *>(0x007F9551));
    for (int i = 0; i < 22; ++i) {
      nop();
    }
    setProtectModeRE();
  }
};

} // namespace patch::water

void WaterPatch::Apply() {
  if (!enabled_) {
    return;
  }

  // 1. Substitute water2.fx when requested by VFS (if override_shader_ is enabled)
  if (override_shader_) {
    core::events::OnMemoryMapFile += [](fa::ConstMemBuffer *buffer,
                                        const char *fname) {
      if (enabled_ && override_shader_ && fname &&
          std::string_view(fname).ends_with("water2.fx")) {
        buffer->assign(patch::water::kWater2ShaderSource);
      }
    };
  }

  // 2. Pre-render projectiles and weapon effects before refraction copy
  if (reflect_projectiles_) {
    static patch::water::RefractionCopyHookStub refraction_hook_stub;
    patch::water::PatchRefractionCopyCall patch_refraction_call(
        refraction_hook_stub.getCode<const void *>());

    static patch::water::PatchOriginalProjectilesSkip patch_skip_projectiles;
  }
}
