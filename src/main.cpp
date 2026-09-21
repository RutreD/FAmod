#include <Windows.h>
#include <xbyak/xbyak.h>
#include <string_view>

namespace {

enum class GameFlavor {
  Unknown,
  FAForever,
  Steam,
};

[[nodiscard]] GameFlavor DetectGameFlavor() noexcept {
  const auto *dos =
      reinterpret_cast<const IMAGE_DOS_HEADER *>(GetModuleHandleW(nullptr));
  if (!dos || dos->e_magic != IMAGE_DOS_SIGNATURE) {
    return GameFlavor::Unknown;
  }

  const auto *nt = reinterpret_cast<const IMAGE_NT_HEADERS *>(
      reinterpret_cast<const std::uint8_t *>(dos) + dos->e_lfanew);
  if (nt->Signature != IMAGE_NT_SIGNATURE) {
    return GameFlavor::Unknown;
  }

  const auto *section = IMAGE_FIRST_SECTION(nt);
  for (WORD i = 0; i < nt->FileHeader.NumberOfSections; ++i, ++section) {
    const std::string_view name{
        reinterpret_cast<const char *>(section->Name),
        strnlen(reinterpret_cast<const char *>(section->Name), 8)};
    if (name == ".exxt") {
      return GameFlavor::FAForever;
    }
    if (name == ".bind") {
      return GameFlavor::Steam;
    }
  }

  if (nt->FileHeader.TimeDateStamp == 0x4E5C092E) {
    return GameFlavor::Steam;
  }

  return GameFlavor::Unknown;
}

struct PatchReplayDesync : public Xbyak::CodeGenerator {
  PatchReplayDesync()
      : Xbyak::CodeGenerator(11, reinterpret_cast<void *>(0x0053C71E)) {
    setProtectMode(PROTECT_RWE);
    constexpr int hasCommandSource = -0x8E;
    mov(byte[esp + 0x0A4 + hasCommandSource], 1);
    jmp(reinterpret_cast<const void *>(0x0053C7D7));
    nop();
    setProtectModeRE();
  }
};

DWORD WINAPI MainThread(LPVOID /*param*/) {
  Sleep(1000);

  const auto flavor = DetectGameFlavor();
  if (flavor != GameFlavor::FAForever) {
    return 0;
  }

  PatchReplayDesync patch;

  return 0;
}

} // namespace

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID /*reserved*/) {
  if (reason == DLL_PROCESS_ATTACH) {
    // DisableThreadLibraryCalls(hModule);
    // HMODULE h_dummy{nullptr};
    // GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_PIN |
    //                        GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,
    //                    reinterpret_cast<LPCWSTR>(hModule), &h_dummy);
    if (const HANDLE hThread =
            CreateThread(nullptr, 0, MainThread, hModule, 0, nullptr)) {
      CloseHandle(hThread);
    }
  }
  return TRUE;
}
