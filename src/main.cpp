#include <Windows.h>
#if defined(_DEBUG)
#include <cstdio>
#endif

import std;
import core.app;

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

#if defined(_DEBUG)
void SetupConsole() {
  if (!AllocConsole())
    return;

  FILE *f = nullptr;
  freopen_s(&f, "CONOUT$", "w", stdout);
  freopen_s(&f, "CONOUT$", "w", stderr);
  freopen_s(&f, "CONIN$", "r", stdin);

  std::cout.clear();
  std::clog.clear();
  std::cerr.clear();
  std::cin.clear();

  const HANDLE h_con_out =
      CreateFileW(L"CONOUT$", GENERIC_READ | GENERIC_WRITE,
                  FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING,
                  FILE_ATTRIBUTE_NORMAL, nullptr);
  const HANDLE h_con_in =
      CreateFileW(L"CONIN$", GENERIC_READ | GENERIC_WRITE,
                  FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING,
                  FILE_ATTRIBUTE_NORMAL, nullptr);

  SetStdHandle(STD_OUTPUT_HANDLE, h_con_out);
  SetStdHandle(STD_ERROR_HANDLE, h_con_out);
  SetStdHandle(STD_INPUT_HANDLE, h_con_in);

  std::wcout.clear();
  std::wclog.clear();
  std::wcerr.clear();
  std::wcin.clear();

  SetConsoleOutputCP(CP_UTF8);
  SetConsoleCP(CP_UTF8);

  // Disable the close button on the console window to prevent accidental
  // shutdown.
  if (HWND hwnd = GetConsoleWindow()) {
    if (HMENU menu = GetSystemMenu(hwnd, FALSE)) {
      DeleteMenu(menu, SC_CLOSE, MF_BYCOMMAND);
    }
  }
}
#endif

DWORD WINAPI MainThread(LPVOID /*param*/) {
  Sleep(1000);

#if defined(_DEBUG)
  SetupConsole();
#endif

  const auto flavor = DetectGameFlavor();
  if (flavor != GameFlavor::FAForever) {
    const wchar_t *msg =
        flavor == GameFlavor::Steam
            ? L"Detected Steam version of Supreme Commander: Forged "
              L"Alliance.\n\n"
              L"FAmod is currently only compatible with FAForever (FAF).\n"
              L"Please launch the game through the FAF Client."
            : L"Detected unsupported game binary.\n\n"
              L"FAmod is currently only compatible with FAForever (FAF).\n"
              L"Please launch the game through the FAF Client.";

    MessageBoxW(nullptr, msg, L"FAmod - Unsupported Game Version",
                MB_OK | MB_ICONWARNING);
    return 0;
  }

#if defined(_DEBUG)
  std::println("FAmod loaded. Press \"~\" in-game to open the patch manager.");
#endif

  core::app::Initialize();

  return 0;
}

} // namespace

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID /*reserved*/) {
  if (reason == DLL_PROCESS_ATTACH) {
    DisableThreadLibraryCalls(hModule);
    HMODULE h_dummy{nullptr};
    GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_PIN |
                           GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,
                       reinterpret_cast<LPCWSTR>(hModule), &h_dummy);
    if (const HANDLE hThread =
            CreateThread(nullptr, 0, MainThread, hModule, 0, nullptr)) {
      CloseHandle(hThread);
    }
  }
  return TRUE;
}
