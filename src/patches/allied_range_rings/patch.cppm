module;
#include <xbyak/xbyak.h>

export module patch.allied_range_rings;
import fa;
import core;
import std;

using namespace fa;

export enum class IntelRangeBehavior : int {
  kOwnUnits = 0,
  kOwnUnitsAndAlliedBuildings = 1,
  kOwnUnitsAndAlliedUnits = 2,
};

export class AlliedRangeRingsPatch : public IPatch {
public:
  inline static IntelRangeBehavior behavior_{
      IntelRangeBehavior::kOwnUnitsAndAlliedBuildings};

  [[nodiscard]] std::string_view Name() const noexcept override {
    return tr("Allied Range Rings",
              {{Language::Russian, "Кольца радиуса союзников"},
               {Language::Chinese, "友军范围圈"}});
  }

  [[nodiscard]] std::string_view Description() const noexcept override {
    return tr(
        "Controls intel range rings for allied units. Also includes bug fixes "
        "and performance (FPS) optimizations.",
        {{Language::Russian,
          "Контролирует отображение колец радиуса для союзников. Так же "
          "содержит исправления ошибок и оптимизацию производительности "
          "(FPS)."},
         {Language::Chinese,
          "控制友军单位的侦察范围圈显示。还包含错误修复及帧率（FPS）优化。"}});
  }

  void Apply() override;
  void RenderUi() override;

  void BindSettings(SettingsBinder &b) override {
    b.Bind("AlliedRangeRings.behavior", behavior_,
           IntelRangeBehavior::kOwnUnitsAndAlliedBuildings);
  }
};

UserUnit *__fastcall RenderRangeFilter(UserUnit *self, uintptr_t esp) {
  auto session = g_CWldSession;
  if (self->mArmy->mConstDat.mIndex == session->focusArmyIndex) {
    return session->selectedUnits.contains(self) ? nullptr : self;
  }
  const auto &range_name_str = **reinterpret_cast<string **>(esp + 0x30);
  const std::string_view range_name = range_name_str.view();
  const bool is_intel =
      range_name == "Radar" || range_name == "Omni" || range_name == "Sonar";
  return is_intel ? self : nullptr;
}

void __fastcall SyncVisionRange(ReconBlip *recon_blip, Unit *unit) {
  auto &recon_attr = recon_blip->Entity::mVarDat.mAttributes;
  auto &unit_attr = unit->mVarDat.mAttributes;
  recon_attr.SetIntelRadius(ENTATTR_Vision,
                            unit_attr.GetIntelRadius(ENTATTR_Vision));
  if (unit->mArmy->IsAlly(g_CWldSession->focusArmyIndex)) {
    recon_attr.SetIntelRadius(ENTATTR_Radar,
                              unit_attr.GetIntelRadius(ENTATTR_Radar));
    recon_attr.SetIntelRadius(ENTATTR_Sonar,
                              unit_attr.GetIntelRadius(ENTATTR_Sonar));
    recon_attr.SetIntelRadius(ENTATTR_Omni,
                              unit_attr.GetIntelRadius(ENTATTR_Omni));
  }
}

bool __cdecl ShouldAddUnit(UserArmy *focus_army, UserUnit *user_unit) {
  if (user_unit->mArmy == focus_army)
    return true;
  if (AlliedRangeRingsPatch::behavior_ == IntelRangeBehavior::kOwnUnits)
    return false;
  if (!user_unit->mArmy->IsAlly(g_CWldSession->focusArmyIndex))
    return false;

  auto blueprint = static_cast<RUnitBlueprint *>(user_unit->mParams.mBlueprint);
  if (AlliedRangeRingsPatch::behavior_ ==
          IntelRangeBehavior::kOwnUnitsAndAlliedBuildings &&
      blueprint->mPhysics.mMotionType != RULEUMT_None)
    return false;

  auto &unit_attr = user_unit->mVarDat.mAttributes;
  const bool has_intel = (unit_attr.GetIntelRadius(ENTATTR_Radar) |
                          unit_attr.GetIntelRadius(ENTATTR_Sonar) |
                          unit_attr.GetIntelRadius(ENTATTR_Omni)) > 0;
  return has_intel;
}

struct RenderRangeFilterCall : public Xbyak::CodeGenerator {
  RenderRangeFilterCall()
      : Xbyak::CodeGenerator(9, reinterpret_cast<void *>(0x007EF12B)) {
    setProtectMode(PROTECT_RWE);
    mov(edx, esp);
    mov(ecx, esi);
    call(reinterpret_cast<const void *>(RenderRangeFilter));
    setProtectModeRE();
  }
};

struct SyncVisionRangeCall : public Xbyak::CodeGenerator {
  SyncVisionRangeCall()
      : Xbyak::CodeGenerator(14, reinterpret_cast<void *>(0x005BF011)) {
    setProtectMode(PROTECT_RWE);
    mov(ecx, edi);
    mov(edx, eax);
    call(reinterpret_cast<const void *>(SyncVisionRange));
    jmp(reinterpret_cast<const void *>(0x005BF02C));
    setProtectModeRE();
  }
};

struct ShouldAddUnitTrampoline : public Xbyak::CodeGenerator {
  ShouldAddUnitTrampoline() {
    push(ecx);
    push(edx);
    push(edi);
    push(eax);
    call(ShouldAddUnit);
    add(esp, 8);
    pop(edx);
    pop(ecx);
    test(al, al);
    je(reinterpret_cast<const void *>(0x007A789F));
    jmp(reinterpret_cast<const void *>(0x007A7860));
  }
};

struct ShouldAddUnitJmp : public Xbyak::CodeGenerator {
  ShouldAddUnitJmp(const void *trampoline_address)
      : Xbyak::CodeGenerator(6, reinterpret_cast<void *>(0x007A7858)) {
    setProtectMode(PROTECT_RWE);
    jmp(trampoline_address);
    nop();
    setProtectModeRE();
  }
};

struct RevertOldPatch : public Xbyak::CodeGenerator {
  RevertOldPatch()
      : Xbyak::CodeGenerator(5, reinterpret_cast<void *>(0x007F944D)) {
    setProtectMode(PROTECT_RWE);
    call(reinterpret_cast<const void *>(0x007EEA00));
    setProtectModeRE();
  }
};

void AlliedRangeRingsPatch::Apply() {
  static ConDescReg con_intel_range_behavior{
      "ren_IntelRangeBehavior",
      "Intel range rings behavior: 0 = Own units only; 1 = Own units and "
      "allied buildings; 2 = Own and all allied units",
      reinterpret_cast<int *>(&behavior_)}; // Keep registered console variable
                                            // alive in memory across the
                                            // process lifetime

  static ShouldAddUnitTrampoline tramp; // Keep generated JIT code buffer alive
                                        // in memory across the process lifetime

  ShouldAddUnitJmp saj(tramp.getCode<const void *>());
  SyncVisionRangeCall svrc;
  RenderRangeFilterCall ruufc;
  RevertOldPatch rop;
}
