module;
#include <xbyak/xbyak.h>

export module patch.share_factory_assist;
import fa;
import core;
import std;

using namespace fa;

export class ShareFactoryAssistPatch : public IPatch {
public:
  inline static bool enabled_{false};
  inline static bool is_factory_requested_{false};

  [[nodiscard]] std::string_view Name() const noexcept override {
    return tr("Factory Assist Orders",
              {{Language::Russian, "Приказы ассиста фабрик"},
               {Language::Chinese, "工厂协助指令"}});
  }

  [[nodiscard]] std::string_view Description() const noexcept override {
    return tr(
        "Allows factories to issue assist orders for produced units (target "
        "or ground rally). Assisting another factory preserves standard "
        "build queue sharing.",
        {{Language::Russian,
          "Позволяет фабрикам отдавать приказ ассиста для произведённых юнитов "
          "(включая цель или приказ на землю). При ассисте другой фабрики "
          "сохраняется стандартное распределение очереди постройки."},
         {Language::Chinese,
          "允许工厂为其生产的单位下达协助指令（包括目标协助或地面集结）。"
          "协助另一座工厂时仍保持原版共享建造队列的行为。"}});
  }

  void Apply() override;
  void RenderUi() override;

  void BindSettings(SettingsBinder &b) override {
    b.Bind("ShareFactoryAssist.enabled", enabled_, true);
  }
};

void __stdcall OnIssueCommand(WeakSet<UserEntity> *units,
                              SSTICommandIssueData *cmd, uint32_t clear_queue);

struct EngineThunks : public Xbyak::CodeGenerator {

  using WeakSetCtorFn = void(__cdecl *)(WeakSet<UserEntity> *set);
  using WeakSetDtorFn = void(__cdecl *)(WeakSet<UserEntity> *set);
  using IssueCommandFn = void(__cdecl *)(WeakSet<UserEntity> *units,
                                         const SSTICommandIssueData *cmd,
                                         uint32_t clear_queue);

  WeakSetCtorFn ws_ctor{nullptr};
  WeakSetDtorFn ws_dtor{nullptr};
  IssueCommandFn issue_cmd_orig{nullptr};
  IssueCommandFn issue_factory{nullptr};
  const void *issue_bridge{nullptr};

  EngineThunks() {
    // 1. WeakSet constructor (0x007AE180: esi = this)
    ws_ctor = getCurr<WeakSetCtorFn>();
    push(esi);
    mov(esi, dword[esp + 8]);
    mov(eax, 0x007AE180);
    call(eax);
    pop(esi);
    ret();

    // 2. WeakSet destructor (0x007ABDE0: eax = this)
    ws_dtor = getCurr<WeakSetDtorFn>();
    mov(eax, dword[esp + 4]);
    mov(ecx, 0x007ABDE0);
    call(ecx);
    ret();

    // 3. Caller for Moho::ISSUE_FactoryCommand (0x008B0B30: ebx = factories)
    issue_factory = getCurr<IssueCommandFn>();
    push(ebx);
    mov(ebx, dword[esp + 8]);
    push(dword[esp + 16]); // clear_queue
    push(dword[esp + 16]); // cmd
    mov(eax, 0x008B0B30);
    call(eax);
    add(esp, 8);
    pop(ebx);
    ret();

    // 4. Hook bridge for Moho::ISSUE_Command -> OnIssueCommand
    issue_bridge = getCurr<const void *>();
    push(dword[esp + 8]); // clear_queue
    push(dword[esp + 8]); // cmd
    push(ebx);            // units
    call(reinterpret_cast<const void *>(OnIssueCommand));
    ret();

    // 5. Caller for original Moho::ISSUE_Command (executes preserved prologue)
    Xbyak::Label l_orig_prologue;
    issue_cmd_orig = getCurr<IssueCommandFn>();
    push(ebx);
    mov(ebx, dword[esp + 8]);
    push(dword[esp + 16]); // clear_queue
    push(dword[esp + 16]); // cmd
    call(l_orig_prologue);
    add(esp, 8);
    pop(ebx);
    ret();

    L(l_orig_prologue);
    push(0xFFFFFFFF);
    push(0x00BB3958);
    jmp(reinterpret_cast<const void *>(0x008B05E7));
  }

  static EngineThunks &Instance() {
    static EngineThunks instance;
    return instance;
  }
};

struct ScopedWeakSet : WeakSet<UserEntity> {
  ScopedWeakSet() { EngineThunks::Instance().ws_ctor(this); }
  ~ScopedWeakSet() { EngineThunks::Instance().ws_dtor(this); }
};

static void SplitFactories(WeakSet<UserEntity> *all_units,
                           WeakSet<UserEntity> *normal_units,
                           WeakSet<UserEntity> *factory_units) {
  ShareFactoryAssistPatch::is_factory_requested_ = true;
  using SplitSelectedUnitsFn = void(__cdecl *)(
      WeakSet<UserEntity> * all_units, WeakSet<UserEntity> * normal_units,
      WeakSet<UserEntity> * factory_units);
  reinterpret_cast<SplitSelectedUnitsFn>(0x0081E9E0)(all_units, normal_units,
                                                     factory_units);
  ShareFactoryAssistPatch::is_factory_requested_ = false;
}

struct PatchJumpGenerator : public Xbyak::CodeGenerator {
  PatchJumpGenerator(uintptr_t target_site, const void *dest,
                     size_t total_len = 5)
      : Xbyak::CodeGenerator(total_len, reinterpret_cast<void *>(target_site)) {
    setProtectMode(PROTECT_RWE);
    jmp(dest);
    if (total_len > 5) {
      nop(total_len - 5);
    }
    setProtectModeRE();
  }
};

// 1. Hook inside Moho::SplitSelectedUnitsByGivenCategory (0x0081EA44)
// Switches category from "REBUILDER" to "RALLYPOINT" when requested
struct CategoryHookTrampoline : public Xbyak::CodeGenerator {
  explicit CategoryHookTrampoline(const bool *p_is_factory_requested) {
    Xbyak::Label l_default;
    cmp(byte[reinterpret_cast<const void *>(p_is_factory_requested)], 0);
    je(l_default);

    push(10); // strlen("RALLYPOINT")
    static constexpr char kRallypointStr[] = "RALLYPOINT";
    push(reinterpret_cast<uintptr_t>(kRallypointStr));
    jmp(reinterpret_cast<const void *>(0x0081EA4B));

    L(l_default);
    push(9); // strlen("REBUILDER")
    push(0x00E20498);
    jmp(reinterpret_cast<const void *>(0x0081EA4B));
  }
};

// 2. Hook inside Moho::SCommandModeData::HandleEvent (0x00820426, 11 bytes)
// Prevents factories from being filtered out during ground Assist orders
struct GroundFilterTrampoline : public Xbyak::CodeGenerator {
  GroundFilterTrampoline() {
    Xbyak::Label l_allow;
    Xbyak::Label l_skip;

    mov(edx, dword[ecx]);
    mov(eax, dword[edx + 0x10]);
    call(eax); // CanMoveOrGuardGround()
    test(al, al);
    jnz(l_allow); // Mobile unit -> allowed

    // Unit cannot move - allow if ShareFactoryAssist is enabled and command is
    // Guard
    cmp(byte[reinterpret_cast<const void *>(
            &ShareFactoryAssistPatch::enabled_)],
        0);
    je(l_skip);
    cmp(dword[edi + 4], RULEUCC_Guard); // [edi+4] = mCommandCaps
    jne(l_skip);

    L(l_allow);
    jmp(reinterpret_cast<const void *>(
        0x00820431)); // Add unit to selection list

    L(l_skip);
    jmp(reinterpret_cast<const void *>(0x0082047A)); // Skip unit
  }
};

// Handles ALL Assist targets uniformly: Units, Structures, Ground Rally Points
void __stdcall OnIssueCommand(WeakSet<UserEntity> *units,
                              SSTICommandIssueData *cmd, uint32_t clear_queue) {
  if (!ShareFactoryAssistPatch::enabled_ || !units || units->Empty() || !cmd ||
      cmd->mCommandType != UNITCOMMAND_Guard) {
    EngineThunks::Instance().issue_cmd_orig(units, cmd, clear_queue);
    return;
  }

  ScopedWeakSet normal_units;
  ScopedWeakSet factory_units;
  SplitFactories(units, &normal_units, &factory_units);

  if (factory_units.Empty()) {
    EngineThunks::Instance().issue_cmd_orig(units, cmd, clear_queue);
    return;
  }

  // Normal units receive standard assist command
  if (!normal_units.Empty()) {
    EngineThunks::Instance().issue_cmd_orig(&normal_units, cmd, clear_queue);
  }

  // Factories receive factory command (sharing build queue or setting ground
  // rally point)
  EngineThunks::Instance().issue_factory(&factory_units, cmd, clear_queue);
}

void ShareFactoryAssistPatch::Apply() {
  static ConDescReg share_factory_assist_cmd{
      "ui_ShareFactoryAssistCommandToUnits",
      "Share factory assist command to units (true/false)", &enabled_};
  // Keep generated JIT code buffer alive in memory across the process lifetime
  static CategoryHookTrampoline split_tramp(&is_factory_requested_);
  // Keep generated JIT code buffer alive in memory across the process lifetime
  static GroundFilterTrampoline ground_filter_tramp;

  PatchJumpGenerator split_tramp_patch(0x0081EA44, split_tramp.getCode(), 7);
  PatchJumpGenerator ground_filter_tramp_patch(
      0x00820426, ground_filter_tramp.getCode(), 11);
  PatchJumpGenerator issue_bridge_patch(
      0x008B05E0, EngineThunks::Instance().issue_bridge, 7);
}
