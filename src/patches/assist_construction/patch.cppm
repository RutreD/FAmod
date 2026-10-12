module;
#include <xbyak/xbyak.h>

export module patch.assist_construction;
import fa;
import core;
import std;

using namespace fa;

export class AssistConstructionOrdersPatch : public IPatch {
public:
  inline static bool enabled_{true};

  [[nodiscard]] std::string_view Name() const noexcept override {
    return tr("Assist Construction Orders",
              {{Language::Russian, "Ассист запланированных построек"},
               {Language::Chinese, "协助规划建造"}});
  }

  [[nodiscard]] std::string_view Description() const noexcept override {
    return tr(
        "Allows engineers to assist queued or unbuilt building blueprints "
        "(ghosts) by right-clicking them directly, or by issuing explicit "
        "Repair orders.\nConsole command: ui_AssistConstructionOrders",
        {{Language::Russian,
          "Позволяет инженерам ассистировать запланированные призраки построек "
          "(чертежи в очереди) кликом ПКМ напрямую или через приказ починки "
          "(Repair).\n"
          "Консольная команда: ui_AssistConstructionOrders"},
         {Language::Chinese, "允许工程单位通过直接右键点击规划中的建筑蓝图（幽"
                             "灵虚影）或使用修理指令（Repair）协助建造。\n"
                             "控制台指令：ui_AssistConstructionOrders"}});
  }

  void Apply() override;
  void RenderUi() override;

  void BindSettings(SettingsBinder &b) override {
    b.Bind("AssistConstructionOrders.enabled", enabled_, true);
  }
};

// Helper function: issues an assist command for a queued or under-construction
// building blueprint. In the Moho engine, field construction by engineers/ACUs
// (buildings, factories, etc.) is represented internally by
// EUnitCommandType::UNITCOMMAND_BuildMobile. Returns true if the order was
// successfully issued, false if the target is not a construction blueprint.
static bool
TryIssueAssistConstructionOrder(const SCommandModeData *command_mode,
                                CWldSession *session) {
  if (!AssistConstructionOrdersPatch::enabled_ || !command_mode ||
      !command_mode->HasDraggedCommand()) {
    return false;
  }

  CommandIssueHelper *command_under_mouse =
      EngineRuntime::FindCommandIssueHelper(session->mCommandManager,
                                            command_mode->mIsDragged);
  if (!command_under_mouse || command_under_mouse->GetCommandType() !=
                                  EUnitCommandType::UNITCOMMAND_BuildMobile) {
    return false;
  }

  SSTICommandIssueData data{EUnitCommandType::UNITCOMMAND_BuildMobile};
  data.mTarget.mType = AITARGET_Ground;
  data.mTarget.mEnt = 0xF0000000u;
  data.mTarget.mPos = command_under_mouse->GetPosition();
  data.mBlueprint = command_under_mouse->GetBlueprint();
  data.mOri = {1.0f, 0.0f, 0.0f, 0.0f};

  const bool clear_queue = (command_mode->mModifiers & 1) == 0;
  EngineRuntime::IssueCmd(&session->mSelection, &data, clear_queue);

  return true;
}

// C++ hook logic for Ground / Move / Guard path (0x00820847)
void __stdcall
OnCheckCanAssistConstructionOrder(SSTICommandIssueData *cmd,
                                  const SCommandModeData *command_mode,
                                  CommandIssueHelper *command_under_mouse) {
  if (!AssistConstructionOrdersPatch::enabled_ || !cmd || !command_mode ||
      !command_mode->HasDraggedCommand() || !command_under_mouse) {
    return;
  }

  if (command_under_mouse->GetCommandType() ==
      EUnitCommandType::UNITCOMMAND_BuildMobile) {
    cmd->mCommandType = EUnitCommandType::UNITCOMMAND_BuildMobile;
    cmd->mBlueprint = command_under_mouse->GetBlueprint();
    cmd->mTarget.mType = AITARGET_Ground;
    cmd->mTarget.mEnt = 0xF0000000u;
    cmd->mTarget.mPos = command_under_mouse->GetPosition();
    cmd->mOri = {1.0f, 0.0f, 0.0f, 0.0f};
  }
}

// C++ hook logic for Reclaim path (0x008215CD).
// In vanilla FA, right-clicking on empty ground over a construction ghost
// issues Move, but right-clicking when an obstacle (tree or wreck) is
// underneath triggers an implicit Reclaim order. We intercept implicit Reclaim
// (context RMB) so it assists the construction order instead. If the player
// EXPLICITLY selected Reclaim from the command card or via keybind
// (executed via LMB), we do NOT intercept it, allowing them to intentionally
// reclaim the obstacle.
bool __stdcall OnCheckReclaimPath(const SCommandModeData *command_mode,
                                  CWldSession *session) {
  // Only intercept implicit right-click commands; leave explicit command card
  // reclaim intact
  if (!IsImplicitRightClickCommand(command_mode, session)) {
    return false;
  }

  return TryIssueAssistConstructionOrder(command_mode, session);
}

// C++ hook logic for Repair path (0x00821844).
// In vanilla FA, clicking Repair on empty terrain or on a queued construction
// ghost drops the order because ebp (target unit) is null. This intercepts that
// case and redirects it into assisting the construction order if a queued
// blueprint command exists under the cursor.
bool __stdcall OnCheckRepairPath(const SCommandModeData *command_mode,
                                 CWldSession *session) {
  return TryIssueAssistConstructionOrder(command_mode, session);
}

// Hook 1 trampoline: Moho::SCommandModeData::HandleEvent (0x00820847, 5 bytes)
// Replaces: cmp byte ptr [esp+11h], 0
struct AssistConstructionOrderTrampoline : public Xbyak::CodeGenerator {
  AssistConstructionOrderTrampoline() {
    push(eax);
    push(ecx);
    push(edx);

    push(esi); // command_under_mouse
    push(edi); // command_mode (SCommandModeData*)
    // cmd is at [esp + 0x64 + 20]
    lea(eax, dword[esp + 0x64 + 20]);
    push(eax);
    call(reinterpret_cast<const void *>(OnCheckCanAssistConstructionOrder));

    pop(edx);
    pop(ecx);
    pop(eax);

    // Original replaced instruction: cmp byte ptr [esp+11h], 0
    cmp(byte[esp + 0x11], 0);

    // Jump back to 0x0082084C
    jmp(reinterpret_cast<const void *>(0x0082084C));
  }
};

// Hook 2 trampoline: Moho::SCommandModeData::HandleEvent (0x008215CD, 6 bytes)
// Replaces: jz loc_8218A9
struct ReclaimCheckTrampoline : public Xbyak::CodeGenerator {
  ReclaimCheckTrampoline() {
    Xbyak::Label l_not_reclaim;
    Xbyak::Label l_to_reclaim_order;

    // Check ZF flag set by: cmp ecx, 100000h at 0x008215C1
    jnz(l_not_reclaim);

    // Reclaim command capability detected - check if cursor is over a
    // construction blueprint
    push(eax);
    push(ecx);
    push(edx);

    push(edx); // session (CWldSession*)
    push(edi); // command_mode (SCommandModeData*)
    call(reinterpret_cast<const void *>(OnCheckReclaimPath));

    test(al, al);
    pop(edx);
    pop(ecx);
    pop(eax);

    jz(l_to_reclaim_order);

    // Handled: skip remaining event handling and exit function cleanly
    jmp(reinterpret_cast<const void *>(0x00821EEA));

    L(l_to_reclaim_order);
    // Not construction blueprint: jump to engine reclaim handling
    jmp(reinterpret_cast<const void *>(0x008218A9));

    L(l_not_reclaim);
    // Continue with next capability check in engine (0x008215D3: cmp ecx,
    // 10000h)
    jmp(reinterpret_cast<const void *>(0x008215D3));
  }
};

// Hook 3 trampoline: Moho::SCommandModeData::HandleEvent (0x00821844, 6 bytes).
// Replaces: jz def_81FD5C
// For RULEUCC_Repair, the engine executes: test ebp, ebp (0x00821842).
// In vanilla FA, ebp holds the entity under the cursor (including wrecks!).
// If the player clicks Repair over a queued construction blueprint ghost that
// is sitting on top of a wreck, vanilla FA sees ebp != 0 and repairs the wreck.
// To fix this, we ALWAYS check for a construction blueprint ghost first:
// - If a construction blueprint ghost exists under the cursor: issue assist
// order and exit.
// - If no construction ghost exists: fallback to vanilla logic (repair living
// unit if ebp != 0,
//   or silently drop the order if ebp == 0).
struct RepairCheckTrampoline : public Xbyak::CodeGenerator {
  RepairCheckTrampoline() {
    Xbyak::Label l_has_unit;
    Xbyak::Label l_exit;

    // Check if cursor is over a construction blueprint ghost first
    push(eax);
    push(ecx);
    push(edx);

    push(edx); // session (CWldSession*)
    push(edi); // command_mode (SCommandModeData*)
    call(reinterpret_cast<const void *>(OnCheckRepairPath));

    test(al, al);
    pop(edx);
    pop(ecx);
    pop(eax);

    // If assist command was successfully issued, exit HandleEvent
    jnz(l_exit);

    // Not a construction blueprint ghost: check if a target entity exists under
    // cursor (ebp != 0)
    test(ebp, ebp);
    jnz(l_has_unit);

    L(l_exit);
    // Exit HandleEvent (def_81FD5C at 0x00821EEA)
    jmp(reinterpret_cast<const void *>(0x00821EEA));

    L(l_has_unit);
    // Entity exists under cursor: proceed with original repair handling
    jmp(reinterpret_cast<const void *>(0x0082184A));
  }
};

void AssistConstructionOrdersPatch::Apply() {
  static ConDescReg assist_construction_orders_cmd{
      "ui_AssistConstructionOrders",
      "Assist construction orders on blueprints (true/false)", &enabled_};

  static AssistConstructionOrderTrampoline assist_con_tramp;
  static ReclaimCheckTrampoline reclaim_check_tramp;
  static RepairCheckTrampoline repair_check_tramp;

  PatchJump assist_con_patch(0x00820847, assist_con_tramp.getCode(), 5);
  PatchJump reclaim_check_patch(0x008215CD, reclaim_check_tramp.getCode(), 6);
  PatchJump repair_check_patch(0x00821844, repair_check_tramp.getCode(), 6);
}
