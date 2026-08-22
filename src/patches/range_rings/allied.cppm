module;
#include <xbyak/xbyak.h>

module patch.range_rings:allied;

import :strategic_defense;
import fa;
import core;
import std;

using namespace fa;

namespace patch::range_rings {

enum class IntelRangeBehavior : int {
  kOwnUnits = 0,
  kOwnUnitsAndAlliedBuildings = 1,
  kOwnUnitsAndAlliedUnits = 2,
};

inline IntelRangeBehavior allied_behavior_{
    IntelRangeBehavior::kOwnUnitsAndAlliedBuildings};

UserUnit *__fastcall RenderRangeFilter(UserUnit *self, uintptr_t esp) {
  auto session = g_CWldSession;
  if (self->mArmy->mConstDat.mIndex == session->focusArmyIndex) {
    return session->selectedUnits.contains(self) ? nullptr : self;
  }

  const auto &range_name_str = **reinterpret_cast<string **>(esp + 0x30);
  const std::string_view range_name = range_name_str.view();

  const bool is_intel = (range_name == "Radar" || range_name == "Omni" ||
                         range_name == "Sonar") &&
                        (allied_behavior_ != IntelRangeBehavior::kOwnUnits);

  const bool is_smd =
      smd_enabled_ && (range_name == "StrategicDefense") &&
      (smd_behavior_ == StrategicDefenseBehavior::kOwnAndAlliedUnits);

  return (is_intel || is_smd) ? self : nullptr;
}

void __fastcall SyncVisionRange(ReconBlip *recon_blip, Unit *unit) {
  auto &recon_attr = recon_blip->Entity::mVarDat.mAttributes;
  auto &unit_attr = unit->mVarDat.mAttributes;

  recon_attr.SetIntelRadius(ENTATTR_Vision,
                            unit_attr.GetIntelRadius(ENTATTR_Vision));

  if (unit->mArmy && unit->mArmy->IsAlly(g_CWldSession->focusArmyIndex)) {
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

  if (!user_unit->mArmy->IsAlly(g_CWldSession->focusArmyIndex))
    return false;

  auto blueprint = static_cast<RUnitBlueprint *>(user_unit->mParams.mBlueprint);

  bool has_intel = false;
  if (allied_behavior_ != IntelRangeBehavior::kOwnUnits) {
    if (allied_behavior_ == IntelRangeBehavior::kOwnUnitsAndAlliedUnits ||
        blueprint->mPhysics.mMotionType == RULEUMT_None) {
      auto &unit_attr = user_unit->mVarDat.mAttributes;
      has_intel = (unit_attr.GetIntelRadius(ENTATTR_Radar) |
                   unit_attr.GetIntelRadius(ENTATTR_Sonar) |
                   unit_attr.GetIntelRadius(ENTATTR_Omni)) != 0;
    }
  }

  bool has_smd = false;
  if (smd_enabled_ &&
      smd_behavior_ == StrategicDefenseBehavior::kOwnAndAlliedUnits) {
    has_smd = FindStrategicDefenseWeapon(blueprint) != nullptr;
  }

  return has_intel || has_smd;
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

void ApplyAllied() {
  static ShouldAddUnitTrampoline tramp;
  ShouldAddUnitJmp saj(tramp.getCode<const void *>());
  SyncVisionRangeCall svrc;
  RenderRangeFilterCall ruufc;
  RevertOldPatch rop;
}

} // namespace patch::range_rings
