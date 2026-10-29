module;
#include <Windows.h>
#include <imgui.h>

export module patch.xact_clock;
import std;
import core;

// xactengine2_9.dll (addresses at its preferred base 0x00400000) builds its
// clock as QueryPerformanceCounter * 1000 / frequency (0x00407C14) and callers
// keep only the low 32 bits. Its work loop (0x0040D34F) runs only when
// `(int)now >= (int)(nextDue - 5)`, and a new engine starts with
// nextDue = 0. While the low 32 bits of the uptime in
// milliseconds read as negative (uptime 24.9-49.7 days, repeating every 49.7
// days), a new engine never updates its cues, so each cue plays once and its
// instance limit blocks every replay. Rebasing XACT's QueryPerformanceCounter
// to the hook time keeps the clock small and positive.

namespace {

constexpr const wchar_t *kXactDll = L"xactengine2_9.dll";

enum class XactClockState {
  NotApplied,
  Hooked,
  SkippedHealthy,
  DllMissing,
  ImportMissing,
};

XactClockState g_state = XactClockState::NotApplied;
LONGLONG g_counter_base = 0;

BOOL WINAPI QueryPerformanceCounterRebased(LARGE_INTEGER *counter) {
  const BOOL ok = QueryPerformanceCounter(counter);
  if (ok) {
    counter->QuadPart -= g_counter_base;
  }
  return ok;
}

// Same value XACT's clock function produces for a counter reading.
[[nodiscard]] std::int32_t XactMilliseconds(LONGLONG counter) noexcept {
  LARGE_INTEGER frequency;
  QueryPerformanceFrequency(&frequency);
  return static_cast<std::int32_t>(counter * 1000 / frequency.QuadPart);
}

[[nodiscard]] void **FindImportSlot(HMODULE module, const char *dll_name,
                                    std::string_view func_name) noexcept {
  auto *base = reinterpret_cast<std::uint8_t *>(module);
  const auto *dos = reinterpret_cast<const IMAGE_DOS_HEADER *>(base);
  const auto *nt =
      reinterpret_cast<const IMAGE_NT_HEADERS *>(base + dos->e_lfanew);
  const auto &dir =
      nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
  if (dir.VirtualAddress == 0) {
    return nullptr;
  }

  for (auto *desc = reinterpret_cast<const IMAGE_IMPORT_DESCRIPTOR *>(
           base + dir.VirtualAddress);
       desc->Name != 0; ++desc) {
    if (lstrcmpiA(reinterpret_cast<const char *>(base + desc->Name),
                  dll_name) != 0 ||
        desc->OriginalFirstThunk == 0) {
      continue;
    }

    const auto *names = reinterpret_cast<const IMAGE_THUNK_DATA *>(
        base + desc->OriginalFirstThunk);
    auto *slots = reinterpret_cast<IMAGE_THUNK_DATA *>(base + desc->FirstThunk);
    for (; names->u1.AddressOfData != 0; ++names, ++slots) {
      if (IMAGE_SNAP_BY_ORDINAL(names->u1.Ordinal)) {
        continue;
      }
      const auto *by_name = reinterpret_cast<const IMAGE_IMPORT_BY_NAME *>(
          base + names->u1.AddressOfData);
      if (func_name == reinterpret_cast<const char *>(by_name->Name)) {
        return reinterpret_cast<void **>(&slots->u1.Function);
      }
    }
  }
  return nullptr;
}

void WriteImportSlot(void **slot, void *target) noexcept {
  DWORD old_protect;
  VirtualProtect(slot, sizeof(*slot), PAGE_READWRITE, &old_protect);
  InterlockedExchangePointer(slot, target);
  VirtualProtect(slot, sizeof(*slot), old_protect, &old_protect);
}

} // namespace

export class XactClockPatch : public IPatch {
public:
  [[nodiscard]] std::string_view Name() const noexcept override {
    return tr("XACT Sound Clock Fix",
              {{Language::Russian, "Исправление таймера звука XACT"},
               {Language::Chinese, "XACT 音频时钟修复"}});
  }

  [[nodiscard]] std::string_view Description() const noexcept override {
    return tr(
        "Fixes every sound playing only once per game launch when Windows has "
        "been running for 24.9 to 49.7 days (repeating every 49.7 days). "
        "XACT's 32-bit millisecond clock overflows and its cue update loop "
        "stops running.",
        {{Language::Russian,
          "Исправляет ошибку, из-за которой каждый звук воспроизводится только "
          "один раз за запуск игры, если Windows работает без перезагрузки от "
          "24,9 до 49,7 дня (повторяется каждые 49,7 дня). 32-битный "
          "миллисекундный таймер XACT переполняется, и цикл обновления звуков "
          "перестаёт выполняться."},
         {Language::Chinese,
          "修复当 Windows 连续运行 24.9 至 49.7 天时（每 49.7 天重复一次），"
          "每个声音在每次启动游戏后只播放一次的问题。XACT 的 32 位毫秒计时器"
          "溢出，导致其声音更新循环停止运行。"}});
  }

  void Apply() override;
  void RenderUi() override;
};

void XactClockPatch::Apply() {
  const bool already_loaded = GetModuleHandleW(kXactDll) != nullptr;
  // Loading it here also pins it, so the patched import slot stays valid.
  const HMODULE xact = LoadLibraryW(kXactDll);
  if (!xact) {
    g_state = XactClockState::DllMissing;
    return;
  }

  LARGE_INTEGER now;
  QueryPerformanceCounter(&now);

  // An engine created earlier with a non-negative clock is healthy and has
  // deadlines on the real clock; rebasing now would stall it instead.
  if (already_loaded && XactMilliseconds(now.QuadPart) >= 0) {
    g_state = XactClockState::SkippedHealthy;
    return;
  }

  void **slot = FindImportSlot(xact, "KERNEL32.dll", "QueryPerformanceCounter");
  if (!slot) {
    g_state = XactClockState::ImportMissing;
    return;
  }

  g_counter_base = now.QuadPart;
  WriteImportSlot(slot, reinterpret_cast<void *>(&QueryPerformanceCounterRebased));
  g_state = XactClockState::Hooked;
}

void XactClockPatch::RenderUi() {
  LocalizedString status{""};
  switch (g_state) {
  case XactClockState::Hooked:
    status = tr("Active: XACT's clock now counts from game start.",
                {{Language::Russian,
                  "Активно: таймер XACT теперь отсчитывается от запуска игры."},
                 {Language::Chinese,
                  "已启用：XACT 计时器现在从游戏启动时开始计时。"}});
    break;
  case XactClockState::SkippedHealthy:
    status = tr("Inactive: XACT was already running with a valid clock, so it "
                "was left unchanged.",
                {{Language::Russian, "Неактивно: XACT уже работал с корректным "
                                     "таймером, поэтому он не изменён."},
                 {Language::Chinese,
                  "未启用：XACT 已在使用正常的计时器运行，因此未作更改。"}});
    break;
  case XactClockState::DllMissing:
    status = tr("Inactive: xactengine2_9.dll is not installed.",
                {{Language::Russian,
                  "Неактивно: xactengine2_9.dll не установлен."},
                 {Language::Chinese, "未启用：未安装 xactengine2_9.dll。"}});
    break;
  case XactClockState::ImportMissing:
    status = tr("Inactive: unexpected xactengine2_9.dll build "
                "(QueryPerformanceCounter import not found).",
                {{Language::Russian,
                  "Неактивно: неизвестная сборка xactengine2_9.dll (импорт "
                  "QueryPerformanceCounter не найден)."},
                 {Language::Chinese,
                  "未启用：xactengine2_9.dll 版本异常（未找到 "
                  "QueryPerformanceCounter 导入）。"}});
    break;
  case XactClockState::NotApplied:
    return;
  }

  ImGui::TextWrapped("%s", status.c_str());
}
