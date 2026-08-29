module;
#include <algorithm>
#include <cstdint>
#include <xbyak/xbyak.h>

export module patch.strategic_icons;
import fa;
import core;

using namespace fa;

namespace {
struct CD3DBatchTextureLayout {
  void *vftable;
  void *list_next;
  void *list_prev;
  uint32_t width;
  uint32_t height;
  uint32_t border;
};
} // namespace

export class StrategicIconsPatch : public IPatch {
public:
  inline static float scale_factor_{1.0f};
  inline static bool normalize_high_res_{true};
  inline static float base_size_{24.0f};

  [[nodiscard]] std::string_view Name() const noexcept override {
    return tr("Strategic Icons",
              {{Language::Russian, "Стратегические иконки"},
               {Language::Chinese, "战略图标"}});
  }

  [[nodiscard]] std::string_view Description() const noexcept override {
    return tr(
        "Decouples strategic icon screen size from texture resolution, allowing "
        "high-resolution icons with smooth lines and customizable scaling.",
        {{Language::Russian,
          "Отделяет размер стратегических иконок на экране от разрешения "
          "текстуры, обеспечивая поддержку четких HD-иконок и масштабирование."},
         {Language::Chinese,
          "将战略图标屏幕尺寸与纹理分辨率解耦，支持高清平滑图标及缩放。"}});
  }

  void Apply() override;
  void RenderUi() override;

  void BindSettings(SettingsBinder &b) override {
    b.Bind("StrategicIcons.scale", scale_factor_, 1.0f);
    b.Bind("StrategicIcons.normalizeHighRes", normalize_high_res_, true);
    b.Bind("StrategicIcons.baseSize", base_size_, 24.0f);
  }
};

extern "C" void __stdcall ComputeIconHalfExtents(
    const CD3DBatchTextureLayout *tex, float *out_half_w, float *out_half_h) {
  if (!tex || tex->width == 0 || tex->height == 0) {
    float fallback = (StrategicIconsPatch::base_size_ * 0.5f) *
                     StrategicIconsPatch::scale_factor_;
    *out_half_w = fallback;
    *out_half_h = fallback;
    return;
  }

  float w = static_cast<float>(tex->width);
  float h = static_cast<float>(tex->height);

  if (StrategicIconsPatch::normalize_high_res_ && (w > 32.0f || h > 32.0f)) {
    float max_dim = (std::max)(w, h);
    float base_ref = StrategicIconsPatch::base_size_;
    w = (w / max_dim) * base_ref;
    h = (h / max_dim) * base_ref;
  }

  float scale = (std::max)(0.1f, StrategicIconsPatch::scale_factor_);
  *out_half_w = (w * 0.5f) * scale;
  *out_half_h = (h * 0.5f) * scale;
}

// 1. Underlay patch at 0x0085DC9F (60 bytes: 0x0085DC9F - 0x0085DCDB)
struct PatchUnderlayExtents : public Xbyak::CodeGenerator {
  PatchUnderlayExtents()
      : Xbyak::CodeGenerator(60, reinterpret_cast<void *>(0x0085DC9F)) {
    setProtectMode(PROTECT_RWE);
    // Exact stack offsets: half_h at esp+0x38, half_w at esp+0x3C, center_x at esp+0x40
    lea(eax, dword[esp + 0x38]); // &out_half_h
    push(eax);
    lea(eax, dword[esp + 0x40]); // &out_half_w (was 0x3C + 4)
    push(eax);
    push(ecx);                   // tex
    call(reinterpret_cast<const void *>(&ComputeIconHalfExtents));
    movss(xmm6, dword[esp + 0x40]); // center_x
    while (getSize() < 60) {
      nop();
    }
    setProtectModeRE();
  }
};

// 2. Main unit icon patch at 0x0085DDAF (62 bytes: 0x0085DDAF - 0x0085DDED)
struct PatchMainIconExtents : public Xbyak::CodeGenerator {
  PatchMainIconExtents()
      : Xbyak::CodeGenerator(62, reinterpret_cast<void *>(0x0085DDAF)) {
    setProtectMode(PROTECT_RWE);
    mov(ebx, dword[ebx]);
    lea(eax, dword[esp + 0x38]); // &out_half_h
    push(eax);
    lea(eax, dword[esp + 0x40]); // &out_half_w (was 0x3C + 4)
    push(eax);
    push(ebx);                   // tex
    call(reinterpret_cast<const void *>(&ComputeIconHalfExtents));
    movss(xmm2, dword[esp + 0x40]); // center_x
    while (getSize() < 62) {
      nop();
    }
    setProtectModeRE();
  }
};

// 3. Overlay 1 (pause/stun/upgrade) patch at 0x0085DEAC (62 bytes: 0x0085DEAC - 0x0085DEEA)
struct PatchOverlay1Extents : public Xbyak::CodeGenerator {
  PatchOverlay1Extents()
      : Xbyak::CodeGenerator(62, reinterpret_cast<void *>(0x0085DEAC)) {
    setProtectMode(PROTECT_RWE);
    mov(ebx, dword[ebx]);
    lea(eax, dword[esp + 0x38]); // &out_half_h
    push(eax);
    lea(eax, dword[esp + 0x40]); // &out_half_w (was 0x3C + 4)
    push(eax);
    push(ebx);                   // tex
    call(reinterpret_cast<const void *>(&ComputeIconHalfExtents));
    movss(xmm2, dword[esp + 0x40]); // center_x
    while (getSize() < 62) {
      nop();
    }
    setProtectModeRE();
  }
};

// 4. Overlay 2 patch at 0x0085DFA8 (59 bytes: 0x0085DFA8 - 0x0085DFE3)
struct PatchOverlay2Extents : public Xbyak::CodeGenerator {
  PatchOverlay2Extents()
      : Xbyak::CodeGenerator(59, reinterpret_cast<void *>(0x0085DFA8)) {
    setProtectMode(PROTECT_RWE);
    mov(ebx, dword[ebx]);
    lea(eax, dword[esp + 0x38]); // &out_half_h
    push(eax);
    lea(eax, dword[esp + 0x40]); // &out_half_w (was 0x3C + 4)
    push(eax);
    push(ebx);                   // tex
    call(reinterpret_cast<const void *>(&ComputeIconHalfExtents));
    mov(eax, dword[esi + 0x14]); // batcher
    while (getSize() < 59) {
      nop();
    }
    setProtectModeRE();
  }
};

void StrategicIconsPatch::Apply() {
  PatchUnderlayExtents underlay_patch;
  PatchMainIconExtents main_patch;
  PatchOverlay1Extents overlay1_patch;
  PatchOverlay2Extents overlay2_patch;

  static ConDescReg ren_strat_icon_scale_var(
      "ren_StratIconScale", "Strategic icons display scale multiplier",
      &scale_factor_);
  static ConDescReg ren_strat_icon_normalize_var(
      "ren_StratIconNormalize",
      "Normalize high resolution strategic icon textures to base size",
      &normalize_high_res_);
  static ConDescReg ren_strat_icon_base_size_var(
      "ren_StratIconBaseSize",
      "Base pixel size for normalized high resolution strategic icons",
      &base_size_);
}
