#pragma once

#ifndef FA_MODULE_IMPORT
#error "Do not include runtime.h directly. Use 'import fa;' instead."
#endif

#include <cstdint>
#include <xbyak/xbyak.h>

struct PatchJump : public Xbyak::CodeGenerator {
  PatchJump(uintptr_t target_site, const void *dest, size_t total_len = 5)
      : Xbyak::CodeGenerator(total_len, reinterpret_cast<void *>(target_site)) {
    setProtectMode(PROTECT_RWE);
    jmp(dest);
    if (total_len > 5) {
      nop(total_len - 5);
    }
    setProtectModeRE();
  }
};

inline void SplitSelectionByRebuilderCategory(
    const WeakSet<UserEntity> *all_units,
    WeakSet<UserEntity> *normal_units,
    WeakSet<UserEntity> *rebuilder_units) {
  using SplitFn = void(__cdecl *)(const WeakSet<UserEntity> *,
                                  WeakSet<UserEntity> *,
                                  WeakSet<UserEntity> *);
  reinterpret_cast<SplitFn>(0x0081E9E0)(all_units, normal_units, rebuilder_units);
}

struct EngineThunks : public Xbyak::CodeGenerator {
  using WeakSetCtorFn = void(__cdecl *)(void *set);
  using WeakSetDtorFn = void(__cdecl *)(void *set);
  using LookupEntityFn = UserEntity *(__cdecl *)(void *session, uint32_t ent_id);
  using FindCmdHelperFn = CommandIssueHelper *(__cdecl *)(void *cmd_mgr, uint32_t cmd_id);
  using IssueCmdFn = void(__cdecl *)(const WeakSet<UserEntity> *units,
                                     const SSTICommandIssueData *cmd,
                                     bool clear_queue);

  inline static WeakSetCtorFn ws_ctor{nullptr};
  inline static WeakSetDtorFn ws_dtor{nullptr};
  inline static LookupEntityFn lookup_entity{nullptr};
  inline static FindCmdHelperFn find_cmd_helper{nullptr};
  inline static IssueCmdFn issue_cmd{nullptr};
  inline static IssueCmdFn issue_factory{nullptr};

  static void Initialize() {
    static EngineThunks instance;
  }

private:
  EngineThunks() {
    // 1. WeakSet constructor (0x007AE180: esi = this)
    ws_ctor = getCurr<WeakSetCtorFn>();
    push(esi);
    mov(esi, dword[esp + 8]); // this
    call(reinterpret_cast<const void *>(0x007AE180));
    pop(esi);
    ret();

    // 2. WeakSet destructor (0x007ABDE0: eax = this)
    ws_dtor = getCurr<WeakSetDtorFn>();
    mov(eax, dword[esp + 4]); // this
    jmp(reinterpret_cast<const void *>(0x007ABDE0));

    // 3. LookupEntity (0x00894280: esi = session, push ent_id)
    lookup_entity = getCurr<LookupEntityFn>();
    push(esi);
    mov(esi, dword[esp + 8]);  // session
    push(dword[esp + 12]);     // ent_id
    call(reinterpret_cast<const void *>(0x00894280));
    pop(esi);
    ret();

    // 4. FindCommandIssueHelper (0x008B5BB0: esi = cmd_mgr, push cmd_id)
    find_cmd_helper = getCurr<FindCmdHelperFn>();
    push(esi);
    mov(esi, dword[esp + 8]);  // cmd_mgr
    push(dword[esp + 12]);     // cmd_id
    call(reinterpret_cast<const void *>(0x008B5BB0));
    pop(esi);
    ret();

    // 5. IssueCmd (0x008B05E0: ebx = units, push clear_queue, push cmd)
    issue_cmd = getCurr<IssueCmdFn>();
    push(ebx);
    mov(ebx, dword[esp + 8]);   // units
    movzx(eax, byte[esp + 16]); // clear_queue
    push(eax);
    push(dword[esp + 16]);      // cmd
    call(reinterpret_cast<const void *>(0x008B05E0));
    add(esp, 8);
    pop(ebx);
    ret();

    // 6. IssueFactoryCmd (0x008B0B30: ebx = factories, push clear_queue, push cmd)
    issue_factory = getCurr<IssueCmdFn>();
    push(ebx);
    mov(ebx, dword[esp + 8]);   // factories
    movzx(eax, byte[esp + 16]); // clear_queue
    push(eax);
    push(dword[esp + 16]);      // cmd
    call(reinterpret_cast<const void *>(0x008B0B30));
    add(esp, 8);
    pop(ebx);
    ret();
  }
};

template <class T>
inline WeakSet<T>::WeakSet() {
  EngineThunks::ws_ctor(this);
}

template <class T>
inline WeakSet<T>::~WeakSet() {
  if (mHead) {
    EngineThunks::ws_dtor(this);
  }
}

namespace EngineRuntime {

inline void Initialize() {
  EngineThunks::Initialize();
}

inline UserEntity *LookupEntity(void *session, uint32_t ent_id) {
  if (!session) return nullptr;
  return EngineThunks::lookup_entity(session, ent_id);
}

inline void IssueFactoryCmd(const WeakSet<UserEntity> *factories,
                            const SSTICommandIssueData *cmd,
                            bool clear_queue) {
  EngineThunks::issue_factory(factories, cmd, clear_queue);
}

inline void IssueCmd(const WeakSet<UserEntity> *units,
                     const SSTICommandIssueData *cmd,
                     bool clear_queue) {
  EngineThunks::issue_cmd(units, cmd, clear_queue);
}

inline CommandIssueHelper *FindCommandIssueHelper(void *command_manager,
                                                  uint32_t command_id) {
  if (!command_manager) return nullptr;
  return EngineThunks::find_cmd_helper(command_manager, command_id);
}

} // namespace EngineRuntime
