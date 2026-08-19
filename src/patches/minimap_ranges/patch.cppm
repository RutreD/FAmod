module;
#include <xbyak/xbyak.h>

export module patch.minimap_ranges;
import fa;
import core;

using namespace fa;

export class MinimapRangesPatch : public IPatch {
public:
  inline static bool render_ranges_{true};

  [[nodiscard]] std::string_view Name() const noexcept override {
    return tr("Minimap Range Rings",
              {{Language::Russian, "Кольца радиуса на миникарте"},
               {Language::Chinese, "小地图范围圈"}});
  }

  [[nodiscard]] std::string_view Description() const noexcept override {
    return tr(
        "Allows disabling range rings on the minimap. Minor FPS improvement.",
        {{Language::Russian,
          "Позволяет отключить отрисовку колец радиуса на миникарте. Даёт "
          "незначительный прирост FPS."},
         {Language::Chinese,
          "允许关闭小地图上的范围圈渲染。对帧率有微小提升。"}});
  }

  void Apply() override;
  void RenderUi() override;

  void BindSettings(SettingsBinder &b) override {
    b.Bind("Minimap.renderRanges", render_ranges_, true);
  }
};

struct DisableMinimapDebugPatch : public Xbyak::CodeGenerator {
  DisableMinimapDebugPatch()
      : Xbyak::CodeGenerator(12, reinterpret_cast<void *>(0x007D1BD9)) {
    setProtectMode(PROTECT_RWE);
    nop(12);
    setProtectModeRE();
  }
};

struct MinimapRangeRenderTrampoline : public Xbyak::CodeGenerator {
  MinimapRangeRenderTrampoline() {
    mov(al, byte[reinterpret_cast<const void *>(
                &MinimapRangesPatch::render_ranges_)]);
    test(al, al);
    jz(reinterpret_cast<const void *>(0x007D1A3F));
    mov(ecx, dword[esp + 0xAC]);
    jmp(reinterpret_cast<const void *>(0x007D1A06));
  }
};

struct PatchMinimapRangeJump : public Xbyak::CodeGenerator {
  PatchMinimapRangeJump(const void *trampoline_address)
      : Xbyak::CodeGenerator(7, reinterpret_cast<void *>(0x007D19FF)) {
    setProtectMode(PROTECT_RWE);
    jmp(trampoline_address);
    nop(2);
    setProtectModeRE();
  }
};

void MinimapRangesPatch::Apply() {
  DisableMinimapDebugPatch debug_patch;
  static MinimapRangeRenderTrampoline
      tramp; // Keep the generated JIT code buffer alive in memory across the
  // process lifetime
  PatchMinimapRangeJump jump_patch(tramp.getCode<void *>());
  static ConDescReg ren_minimap_ranges_var(
      "ren_MinimapRanges", "Render range rings on the minimap (true/false)",
      &render_ranges_); // Keep registered console variable alive in memory
  // across the process lifetime
}
