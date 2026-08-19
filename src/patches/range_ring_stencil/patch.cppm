module;
#include <xbyak/xbyak.h>

export module patch.range_ring_stencil;
import core;

struct PatchLoop1CeilingA : public Xbyak::CodeGenerator {
  PatchLoop1CeilingA()
      : Xbyak::CodeGenerator(4, reinterpret_cast<void *>(0x007EF77E)) {
    setProtectMode(PROTECT_RWE);
    dd(127);
    setProtectModeRE();
  }
};

struct PatchLoop1CeilingB : public Xbyak::CodeGenerator {
  PatchLoop1CeilingB()
      : Xbyak::CodeGenerator(4, reinterpret_cast<void *>(0x007EF785)) {
    setProtectMode(PROTECT_RWE);
    dd(127);
    setProtectModeRE();
  }
};

struct RangeRenderBodyPatch : public Xbyak::CodeGenerator {
  const void *exit_address{nullptr};

  RangeRenderBodyPatch()
      : Xbyak::CodeGenerator(256, reinterpret_cast<void *>(0x007EF7EA)) {
    setProtectMode(PROTECT_RWE);

    constexpr int kRangeRenderer = 0x4;
    constexpr int kIdx = 0x8;
    constexpr int kFrame = 0x10;
    constexpr int kI = 0x14;

    mov(eax, dword[esp + 0x70 + kIdx]);
    mov(edi, dword[esp + 0x70 + kRangeRenderer]);
    add(edi, 0x4C);
    mov(dword[esp + 0x70 + kFrame], edi);

    cvtsi2ss(xmm0, dword[eax + 0x14]);
    movd(ebx, xmm0);
    push(ebx);

    cvtsi2ss(xmm0, dword[eax + 0x10]);
    movd(ebx, xmm0);
    push(ebx);

    mov(ebx, edi);
    call(reinterpret_cast<const void *>(0x007F5DA0));

    push(9);
    push(0x00E3F8E8);
    mov(ecx, edi);
    call(reinterpret_cast<const void *>(0x004059E0));

    mov(eax, dword[esp + 0x70 + kIdx]);
    push(dword[eax + 0x14]);
    push(dword[eax + 0x10]);
    call(reinterpret_cast<const void *>(0x007F6030));

    mov(eax, dword[esp + 0x70 + kI]);
    add(eax, esi);
    cmp(eax, ebp);
    mov(dword[esp + 0x70 + kI], eax);

    jl(reinterpret_cast<const void *>(0x007EF774));

    exit_address = getCurr();
    nop();
    nop();

    setProtectModeRE();
  }
};

struct RangeRenderJumpPatch : public Xbyak::CodeGenerator {
  explicit RangeRenderJumpPatch(const void *target_address)
      : Xbyak::CodeGenerator(6, reinterpret_cast<void *>(0x007EF76E)) {
    setProtectMode(PROTECT_RWE);
    jle(target_address);
    setProtectModeRE();
  }
};

export class RangeRingStencilPatch : public IPatch {
public:
  [[nodiscard]] std::string_view Name() const noexcept override {
    return tr("Range Ring Stencil Fix",
              {{Language::Russian, "Исправление трафарета колец радиуса"},
               {Language::Chinese, "范围圈模板缓冲区修复"}});
  }

  [[nodiscard]] std::string_view Description() const noexcept override {
    return tr(
        "Fixes overlapping range ring stencil overflow, eliminating visual "
        "glitches and significantly boosting FPS when many range rings are "
        "displayed.",
        {{Language::Russian,
          "Исправляет переполнение буфера трафарета (stencil overflow) при "
          "наложении колец, устраняет визуальные глитчи и существенно "
          "повышает FPS при отображении большого количества колец радиуса."},
         {Language::Chinese,
          "修复范围圈重叠时的模板溢出问题，消除画面伪影，并在屏幕显示大量范围"
          "圈时显著提升 FPS。"}});
  }

  void Apply() override {
    PatchLoop1CeilingA patch_a;
    PatchLoop1CeilingB patch_b;
    RangeRenderBodyPatch body_patch;
    RangeRenderJumpPatch jump_patch(body_patch.exit_address);
  }
};
