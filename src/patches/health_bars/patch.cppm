module;
#include <xbyak/xbyak.h>

export module patch.health_bars;
import fa;
import core;

using namespace fa;

export class HealthBarsPatch : public IPatch {
public:
  inline static bool render_full_health_{true};

  [[nodiscard]] std::string_view Name() const noexcept override {
    return tr("Health Bars", {{Language::Russian, "Полосы здоровья"},
                              {Language::Chinese, "生命值条"}});
  }

  [[nodiscard]] std::string_view Description() const noexcept override {
    return tr(
        "Allows hiding health bars for undamaged units (100% HP). Minor FPS "
        "improvement.",
        {{Language::Russian, "Позволяет скрыть полосы здоровья для "
                             "неповреждённых юнитов (100% HP). "
                             "Даёт незначительный прирост FPS."},
         {Language::Chinese,
          "允许隐藏满血单位（100%生命值）的生命条。对帧率有微小提升。"}});
  }

  void Apply() override;
  void RenderUi() override;

  void BindSettings(SettingsBinder &b) override {
    b.Bind("Health.renderFullHealth", render_full_health_, true);
  }
};

struct HealthBarRenderHookBody : public Xbyak::CodeGenerator {
  explicit HealthBarRenderHookBody(const bool *p_render_full_health) {
    Xbyak::Label l_execute_original;
    cmp(byte[reinterpret_cast<const void *>(p_render_full_health)], 1);
    je(l_execute_original);

    movss(xmm0, dword[edi + 0x68]);
    comiss(xmm0, dword[edi + 0x6C]);
    jb(l_execute_original);
    ret();

    L(l_execute_original);
    jmp(reinterpret_cast<const void *>(0x0085EED0));
  }
};

struct PatchHealthBarCall : public Xbyak::CodeGenerator {
  explicit PatchHealthBarCall(const void *trampoline_address)
      : Xbyak::CodeGenerator(5, reinterpret_cast<void *>(0x0085C09F)) {
    setProtectMode(PROTECT_RWE);
    call(trampoline_address);
    setProtectModeRE();
  }
};

void HealthBarsPatch::Apply() {
  static HealthBarRenderHookBody hook_body(&render_full_health_); // Keep
                                                                  // generated
                                                                  // JIT code
                                                                  // buffer
                                                                  // alive in
                                                                  // memory
                                                                  // across the
                                                                  // process
                                                                  // lifetime
  PatchHealthBarCall patch_call(hook_body.getCode());
  static ConDescReg ren_full_health_var(
      "ren_FullHealth",
      "Render health bars for units at full health (true/false)",
      &render_full_health_); // Keep registered console variable alive in memory
                             // across the process lifetime
}
