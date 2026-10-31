module;
#include <xbyak/xbyak.h>

export module patch.horizontal_plus_camera;
import core;
import std;

export class HorizontalPlusCameraPatch : public IPatch {
public:
  inline static bool enabled_{true};

  [[nodiscard]] std::string_view Name() const noexcept override {
    return tr("Hor+ World Camera");
  }

  [[nodiscard]] std::string_view Description() const noexcept override {
    return tr("Preserves the vanilla 4:3 vertical view while revealing more "
              "world horizontally on widescreen and ultrawide viewports.");
  }

  void Apply() override;
  void RenderUi() override;

  void BindSettings(SettingsBinder &b) override {
    b.Bind("HorizontalPlusCamera.enabled", enabled_, true);
  }
};

namespace {

constexpr float kReferenceAspect = 4.0f / 3.0f;

// CameraImpl::UpdateCoords has just built its perspective matrix at
// [caller ESP+0x84]. The retail projection keeps horizontal FOV fixed, making
// wider viewports Vert-. Multiply both projection scale terms by
// (4:3/currentAspect) to keep the 4:3 vertical framing fixed and let horizontal
// FOV grow with the viewport. On hook entry the CALL return address shifts the
// caller's locals by four bytes, so the matrix begins at [ESP+0x88].
//
// The overwritten instruction is replayed first. XMM1/XMM2 are saved because
// this is an inline call-site hook rather than a normal C++ call boundary.
struct HorizontalPlusProjectionHook : public Xbyak::CodeGenerator {
  HorizontalPlusProjectionHook(const bool *enabled,
                               const float *reference_aspect) {
    Xbyak::Label done;

    // Replays 0x007AA898: movss xmm0, [esp+0x4C].
    movss(xmm0, dword[esp + 0x50]);

    cmp(byte[reinterpret_cast<const void *>(enabled)], 0);
    je(done);

    sub(esp, 8);
    movss(dword[esp], xmm1);
    movss(dword[esp + 4], xmm2);

    movss(xmm1, dword[reinterpret_cast<const void *>(reference_aspect)]);
    divss(xmm1, dword[ebx + 0x338]);

    // Projection matrix r[0].x. The eight-byte save moves its hook-relative
    // address from ESP+0x88 to ESP+0x90.
    movss(xmm2, dword[esp + 0x90]);
    mulss(xmm2, xmm1);
    movss(dword[esp + 0x90], xmm2);

    // Projection matrix r[1].y is 0x14 bytes after r[0].x.
    movss(xmm2, dword[esp + 0xA4]);
    mulss(xmm2, xmm1);
    movss(dword[esp + 0xA4], xmm2);

    movss(xmm1, dword[esp]);
    movss(xmm2, dword[esp + 4]);
    add(esp, 8);

    L(done);
    ret();
  }
};

struct HorizontalPlusProjectionCallPatch : public Xbyak::CodeGenerator {
  explicit HorizontalPlusProjectionCallPatch(const void *hook_address)
      : Xbyak::CodeGenerator(6, reinterpret_cast<void *>(0x007AA898)) {
    setProtectMode(PROTECT_RWE);
    call(hook_address);
    nop();
    setProtectModeRE();
  }
};

// CameraImpl::GetMaxZoom normally multiplies the vertical map extent by the
// live viewport aspect at 0x007A7399. The value must match the effective
// aspect used by UpdateCoords to build this camera's projection:
//
//   * Hor+ perspective projection: fixed 4:3 reference aspect.
//   * Original/orthographic projection: live viewport aspect.
//
// This keeps the projection and its zoom bound as one policy. In particular,
// orthographic views such as the minimap retain their original projection and
// therefore retain their original maximum zoom without camera-name checks.
struct HorizontalPlusMaxZoomHook : public Xbyak::CodeGenerator {
  HorizontalPlusMaxZoomHook(const bool *enabled,
                            const float *reference_aspect) {
    Xbyak::Label use_live_aspect;

    cmp(byte[reinterpret_cast<const void *>(enabled)], 0);
    je(use_live_aspect);

    // CameraImpl::mIsOrtho (+0x33C). UpdateCoords only applies the Hor+
    // projection adjustment in its perspective path, so orthographic cameras
    // must continue using their live viewport aspect here as well.
    cmp(byte[ecx + 0x33C], 0);
    jne(use_live_aspect);

    mulss(xmm0, dword[reinterpret_cast<const void *>(reference_aspect)]);
    ret();

    L(use_live_aspect);
    mulss(xmm0, dword[ecx + 0x338]);
    ret();
  }
};

struct HorizontalPlusMaxZoomCallPatch : public Xbyak::CodeGenerator {
  explicit HorizontalPlusMaxZoomCallPatch(const void *hook_address)
      : Xbyak::CodeGenerator(8, reinterpret_cast<void *>(0x007A7399)) {
    setProtectMode(PROTECT_RWE);
    call(hook_address);
    nop(3);
    setProtectModeRE();
  }
};

} // namespace

void HorizontalPlusCameraPatch::Apply() {
  static HorizontalPlusProjectionHook hook(&enabled_, &kReferenceAspect);
  HorizontalPlusProjectionCallPatch patch(hook.getCode());
  static HorizontalPlusMaxZoomHook max_zoom_hook(&enabled_,
                                                  &kReferenceAspect);
  HorizontalPlusMaxZoomCallPatch max_zoom_patch(max_zoom_hook.getCode());
}
