module;
#include <Windows.h>
#include <imgui.h>
#include <rcmp.hpp>

export module core:game_events;

import std;
import fa;
import imgui_hook.signal;

using namespace fa;

export namespace core::events {

inline signal<void()> OnInit;
inline signal<void(CUIManager *)> OnInitUIManager;
inline signal<void(CWldSession *)> OnInitWldSession;
inline signal<void(fa::ConstMemBuffer *, const char *)> OnMemoryMapFile;

void Initialize();

} // namespace core::events

namespace core::events {

void Initialize() {
  // 1. Hook Moho::PLAT_Init (0x004A2150) - one-time engine platform startup
  rcmp::hook_function<0x004A2150, rcmp::cdecl_t<void()>>([](auto original) {
    original();
    OnInit.emit();
  });

  // 2. Hook Moho::WLD_CreateSession (0x00897220) - game session creation
  rcmp::hook_function<
      0x00897220, rcmp::cdecl_t<CWldSession *(void *, void *, void *, void *)>>(
      [](auto original, void *a1, void *a2, void *a3,
         void *a4) -> CWldSession * {
        CWldSession *session = original(a1, a2, a3, a4);
        if (session) {
          OnInitWldSession.emit(session);
        }
        return session;
      });

  // 3. Hook VFS func_MemoryMapFile (0x004596C0) - virtual file system memory
  // mapping
  rcmp::hook_function<0x004596C0,
                      rcmp::stdcall_t<void *(void *a1, fa::ConstMemBuffer *a2,
                                             const char *fname)>>(
      [](auto original, void *a1, fa::ConstMemBuffer *a2,
         const char *fname) -> void * {
        void *res = original(a1, a2, fname);
        if (fname && a2 && a2->mBegin && a2->mEnd) {
          OnMemoryMapFile.emit(a2, fname);
        }
        return res;
      });
}

} // namespace core::events
