module;
#include <cctype>
#include <string_view>
#include <xbyak/xbyak.h>

export module patch.bomber_fire_clock;
import fa;
import core;
import std;

using namespace fa;

export class BomberFireClockPatch : public IPatch {
public:
  inline static bool enabled_{true};

  [[nodiscard]] std::string_view Name() const noexcept override {
    return tr("Bomber Reload Bar",
              {{Language::Russian, "Полоса перезарядки бомбардировщиков"},
               {Language::Chinese, "轰炸机装填进度条"}});
  }

  [[nodiscard]] std::string_view Description() const noexcept override {
    return tr(
        "Displays a yellow reload progress bar (fire clock) below bombers' "
        "health bars for their primary bomb weapon in 3D world and UI.\n"
        "Console command: ui_BomberFireClock",
        {{Language::Russian,
          "Отображает жёлтую полосу перезарядки (fire clock) под полосой "
          "здоровья бомбардировщиков в 3D-мире и в панели интерфейса.\n"
          "Консольная команда: ui_BomberFireClock"},
         {Language::Chinese,
          "在3D战场生命条下方及界面中为轰炸机的主武器显示黄色装填进度条（fire clock）。\n"
          "控制台指令：ui_BomberFireClock"}});
  }

  void Apply() override;
  void RenderUi() override;

  void BindSettings(SettingsBinder &b) override {
    b.Bind("BomberFireClock.enabled", enabled_, true);
  }
};

namespace {

[[nodiscard]] bool MatchFileName(std::string_view path,
                                 std::string_view target) noexcept {
  if (path.size() < target.size())
    return false;

  const auto suffix = path.substr(path.size() - target.size());
  for (std::size_t i = 0; i < target.size(); ++i) {
    char a = suffix[i];
    char b = target[i];
    if (a == '/')
      a = '\\';
    if (b == '/')
      b = '\\';
    if (std::tolower(static_cast<unsigned char>(a)) !=
        std::tolower(static_cast<unsigned char>(b))) {
      return false;
    }
  }
  return true;
}

// Hook for non-shielded unit secondary status bar (0x0085D15A)
// Originally:
// 0x0085d15a: comiss xmm0, xmm2   ; fuelRatio vs workProgress
// 0x0085d15d: jbe 0x0085D171      ; if fuelRatio <= workProgress -> choose workProgress
//             jmp 0x0085D15F      ; choose fuelRatio
struct UnshieldedBarHook : public Xbyak::CodeGenerator {
  explicit UnshieldedBarHook(const bool *p_enabled) {
    Xbyak::Label l_orig;
    Xbyak::Label l_choose_work;
    Xbyak::Label l_choose_fuel;

    cmp(byte[reinterpret_cast<const void *>(p_enabled)], 1);
    jne(l_orig);

    // If workProgress > 0.0f, prioritize workProgress!
    // xmm2 is workProgress, xmm3 is 0.0f
    comiss(xmm2, xmm3);
    ja(l_choose_work);

    // Otherwise choose fuelRatio
    jmp(l_choose_fuel);

    L(l_orig);
    comiss(xmm0, xmm2);
    jbe(l_choose_work);

    L(l_choose_fuel);
    jmp(reinterpret_cast<const void *>(0x0085D15F));

    L(l_choose_work);
    jmp(reinterpret_cast<const void *>(0x0085D171));
  }
};

struct PatchUnshieldedBarJump : public Xbyak::CodeGenerator {
  explicit PatchUnshieldedBarJump(const void *trampoline_address)
      : Xbyak::CodeGenerator(5, reinterpret_cast<void *>(0x0085D15A)) {
    setProtectMode(PROTECT_RWE);
    jmp(trampoline_address);
    setProtectModeRE();
  }
};

// Hook for shielded unit 3rd-row bar (0x0085D0D3)
// Originally:
// 0x0085d0d3: comiss xmm0, dword ptr [0xe4f6e8] ; fuelRatio vs -1.0f
// ...
// 0x0085d0e9: jbe 0x0085D14B                    ; if fuelRatio <= -1.0f -> choose workProgress
struct ShieldedBarHook : public Xbyak::CodeGenerator {
  explicit ShieldedBarHook(const bool *p_enabled) {
    Xbyak::Label l_orig;
    Xbyak::Label l_done;

    cmp(byte[reinterpret_cast<const void *>(p_enabled)], 1);
    jne(l_orig);

    // If workProgress > 0.0f, force jbe at 0x0085D0E9 by setting ZF=1 (comiss xmm0, xmm0)
    comiss(xmm2, xmm3);
    jbe(l_orig);

    comiss(xmm0, xmm0); // ZF=1, so jbe at 0x0085D0E9 is taken
    jmp(l_done);

    L(l_orig);
    comiss(xmm0, dword[reinterpret_cast<const void *>(0x00E4F6E8)]);

    L(l_done);
    jmp(reinterpret_cast<const void *>(0x0085D0DA));
  }
};

struct PatchShieldedBarJump : public Xbyak::CodeGenerator {
  explicit PatchShieldedBarJump(const void *trampoline_address)
      : Xbyak::CodeGenerator(7, reinterpret_cast<void *>(0x0085D0D3)) {
    setProtectMode(PROTECT_RWE);
    jmp(trampoline_address);
    nop(2);
    setProtectModeRE();
  }
};

} // namespace

void BomberFireClockPatch::Apply() {
  static ConDescReg<bool> cvar(
      "ui_BomberFireClock",
      "Render reload progress bar for bombers (true/false)",
      &enabled_);

  // 1. Binary hooks in ForgedAlliance.exe for 3D world lifebar rendering
  static UnshieldedBarHook unshielded_hook(&enabled_);
  static PatchUnshieldedBarJump patch_unshielded(unshielded_hook.getCode());

  static ShieldedBarHook shielded_hook(&enabled_);
  static PatchShieldedBarJump patch_shielded(shielded_hook.getCode());

  // 2. VFS Lua injections
  core::events::OnMemoryMapFile +=
      [](fa::ConstMemBuffer *buffer, const char *fname) {
        if (!fname || !BomberFireClockPatch::enabled_)
          return;

        // Blueprint hook: enable RenderFireClock for bomb/strike weapons
        if (MatchFileName(fname, "system\\blueprints.lua")) {
          buffer->append(R"(
local oldModBlueprints = ModBlueprints
function ModBlueprints(all_bps)
    oldModBlueprints(all_bps)
    if all_bps and all_bps.Unit then
        local count = 0
        for id, bp in all_bps.Unit do
            local isBomber = false
            if bp.CategoriesHash and bp.CategoriesHash.BOMBER then
                isBomber = true
            elseif bp.Categories then
                for _, cat in bp.Categories do
                    if cat == 'BOMBER' then
                        isBomber = true
                        break
                    end
                end
            end
            if isBomber and bp.Weapon then
                local unitHooked = false
                for _, w in bp.Weapon do
                    local cat = w.WeaponCategory or ''
                    local lbl = w.Label or ''
                    if not w.DummyWeapon and cat ~= 'Death' and cat ~= 'Kamikaze' and cat ~= 'Anti Air' then
                        if cat == 'Bomb' or cat == 'Anti Navy' or w.NeedToComputeBombDrop or lbl == 'Bomb' or lbl == 'GroundMissile' or lbl == 'Torpedo' then
                            w.RenderFireClock = true
                            unitHooked = true
                        end
                    end
                end
                if unitHooked then
                    count = count + 1
                end
            end
        end
        LOG("FAMOD: >>> ModBlueprints set RenderFireClock for " .. tostring(count) .. " bombers! <<<")
    end
end
)");
        }

        // Weapon hook: ensure RenderClockThread uses at least 0.01 progress so it never sits at 0.0
        if (MatchFileName(fname, "weapons\\DefaultProjectileWeapon.lua")) {
          buffer->append(R"(

local dpw = rawget(_G, "DefaultProjectileWeapon")
if dpw and dpw.RackSalvoFiringState then
    dpw.RackSalvoFiringState.RenderClockThread = function(self, rateOfFire)
        local unit = self.unit
        local clockTime = math.round(10 * rateOfFire)
        local totalTime = clockTime
        while clockTime >= 0 and not self:BeenDestroyed() and not unit.Dead do
            if not unit:IsUnitState("SiloBuildingAmmo") then
                unit:SetWorkProgress(math.max(0.01, 1 - clockTime / totalTime))
            end
            clockTime = clockTime - 1
            WaitSeconds(0.1)
        end
    end
end
)");
        }
      };
}
