# FAmod — Forged Alliance Binary Enhancement Suite

[![Language](https://img.shields.io/badge/C%2B%2B-23-blue.svg)](https://en.cppreference.com/w/cpp/23)
[![Build](https://img.shields.io/badge/build-xmake-orange.svg)](https://xmake.io)
[![Target](https://img.shields.io/badge/target-FAForever-red.svg)]()
[![License](https://img.shields.io/badge/license-MIT-green.svg)](LICENSE)

**FAmod** is a lightweight in-game enhancement mod and binary patch suite for **[FAForever (Forged Alliance Forever)](https://faforever.com/)**.

> [!IMPORTANT]
> **Compatibility Notice**: FAmod is currently compatible **only with FAForever (FAF)**. It targets the engine modifications and structures present in FAForever's patched binary and will **not** work on vanilla Steam, GOG, or retail CD versions (non-FAF executables are automatically detected and rejected to prevent crashes).

**In-Game GUI**: Press **`~` (Tilde)** at any time  (or type `famod` in the game console) to configure patches

![FAmod Patch Manager Window](docs/screenshots/main_window.png)

---

## 🚀 Player Quick Start Guide

### Installation

1. Download the latest **`dsound.dll`** from the [Releases](https://github.com/RutreD/FAmod/releases) page.
2. Place `dsound.dll` directly into your FAForever binary directory alongside `ForgedAlliance.exe`.
   > **Default FAF Path**: `C:\ProgramData\FAForever\bin`
   * **Replays**: To make FAmod work when watching replays, also copy `dsound.dll` into the replay binary directory:
     > **Default Replay Path**: `C:\ProgramData\FAForever\replaydata\bin`
3. Launch the game via the **FAForever Client** and play as usual!

### Using the In-Game Menu

* Press **`~` (Tilde / Backquote)** on your keyboard at any time in-game to toggle the configuration menu. You can rebind this hotkey in the settings.
* **Lost your hotkey?** You can always open the game's built-in console and type `famod` to force open the configuration menu.
* **UI Scaling**: If you play on a high-DPI (2K / 4K) monitor, use the UI scale slider in the top bar to resize the menu comfortably.
* **Languages**: Full built-in localization in **English**, **Русский (Russian)**, and **简体中文 (Simplified Chinese)**.
* **Auto-Save**: All changes are automatically persisted to `famod_settings.json` in your game folder.

### Uninstallation

To remove FAmod, simply delete `dsound.dll` and `famod_settings.json` from your `FAForever\bin` folder (and from `FAForever\replaydata\bin` if installed for replays).

---

## ✨ Features & Patches


### 1. 📡 Allied Range Rings
* **What it does**: Adds the ability to view allied intel range rings (Radar, Sonar, Omni) with three selectable modes:
  - **Own units only**: Standard vanilla behavior.
  - **Own units and allied static structures**: Shows allied radar towers, sonars, and bases without cluttering the screen with mobile units.
  - **Own and all allied units**: Full vision coverage for all allied units and structures.
* **Preview** *(Vanilla vs. FAmod Enabled)*:

  **Allied Buildings**
  ![Allied Buildings](docs/screenshots/allied_rings_structures.png)

  **All Allied Units**
  ![All Allied Units](docs/screenshots/allied_rings_all_units.png)

  **Own Units Only**
  ![Own Units Only](docs/screenshots/allied_rings_own_only.png)

---

### 2. 🛡️ Range Ring Stencil Fix
* **What it does**: Fixes the game engine's internal stencil buffer overflow when rendering numerous overlapping range rings on screen. Eliminates visual artifacts and delivers a FPS boost.
* **Preview**:
  ![Range Ring Stencil Fix](docs/screenshots/stencil_fix.png)

---

### 3. 📶 Network Stats Colorizer
* **What it does**: Color-codes player latency and ping in the in-game connectivity screen (green $\to$ yellow $\to$ red), allowing you to immediately spot lagging players or connectivity spikes.
* **Preview**:
  ![Network Stats](docs/screenshots/network_stats.png)

---

### 4. ❤️ Health Bars Visibility
* **What it does**: Allows hiding health bars for full-health units (100% HP), keeping the battlefield visually clean and reducing draw overhead.
* **Preview**:
  ![Health Bars](docs/screenshots/health_bars.png)

---

### 5. 🗺️ Minimap Range Rings
* **What it does**: Allows toggling range rings on the minimap, keeping tactical radar views clean and uncluttered.
* **Preview**:
  ![Minimap Range Rings](docs/screenshots/minimap_ranges.png)

---

## ❓ Frequently Asked Questions (FAQ)

<details>
<summary><b>Why should I trust FAmod? / Is this safe? / My antivirus flagged dsound.dll</b></summary>
FAmod is completely open-source (MIT licensed) and fully transparent. All binary builds are built publicly through <b>GitHub Actions</b> directly from the codebase. You don't have to trust opaque prebuilt binaries: you can inspect every line of code, verify GitHub Actions build logs, or build the DLL yourself locally or in your own GitHub fork.
Because FAmod uses a standard DirectX proxy DLL technique (named <code>dsound.dll</code>) to load alongside <code>ForgedAlliance.exe</code>, some overly aggressive heuristics may flag it as unknown. The entire project is open source, safe, and can be built from source yourself.
</details>

<details>
<summary><b>Can I use FAmod with the Steam or GOG version of Supreme Commander: Forged Alliance?</b></summary>
No. FAmod is designed specifically for <b>FAForever (FAF)</b> and relies on the patched engine binaries generated by the FAF ecosystem. Running FAmod with unpatched Steam, GOG, or retail versions is not supported — FAmod will automatically detect the executable format and gracefully abort initialization to prevent game crashes.
</details>

<details>
<summary><b>Where is my FAForever folder located?</b></summary>
The default path is <code>C:\ProgramData\FAForever\bin</code> (and <code>C:\ProgramData\FAForever\replaydata\bin</code> for replays). Note that <code>ProgramData</code> is a hidden folder by default in Windows Explorer (press <code>Win + R</code>, type <code>C:\ProgramData\FAForever\bin</code>, and press Enter).
</details>

---
---

# 🧑‍💻 Developer & AI Sloper Guide

This section is for developers and AI coding agents working on the codebase, adding binary patches, or building custom configurations.

## ☁️ Building with GitHub Actions (Zero Local Setup)

You can build your own `dsound.dll` in the cloud without installing compilers or build tools locally:

1. **Fork** this repository to your GitHub account.
2. Navigate to the **Actions** tab in your fork.
3. In the left sidebar, select the **Release** workflow.
4. Click **Run workflow** on the right:
   - **Build mode**: `release` (or `debug`)
   - **C/C++ Runtime**: `MT` (static runtime)
5. When the run finishes, download `dsound.dll` in either of two ways:
   - **From your Fork's Releases**: The workflow automatically publishes a Nightly Release under the **Releases** tab of your fork.
   - **From Workflow Artifacts**: Download the zip from the **Artifacts** section at the bottom of the Action run summary page.

---

## 🛠️ Local Building & Toolchain Setup

### Prerequisites

To build the project locally, you will need to install the following tools:
1. **[Xmake](https://xmake.io/#/guide/installation)** — The build system used for this project.
2. A C++23 compatible compiler, such as **[MSVC (Visual Studio)](https://visualstudio.microsoft.com/)**, **[LLVM/Clang](https://github.com/llvm/llvm-project)**, or **[MinGW-w64 i686 (niXman builds)](https://github.com/niXman/mingw-builds-binaries)**.

### Tested Compilers & Versions

The project requires a C++23 compiler with C++20 standard library module support (`import std;`):

| Compiler / Toolchain | Tested Version | Target | Configuration Command |
| :--- | :--- | :--- | :--- |
| **MSVC** | Visual Studio 2022 (MSVC v19.40+, Toolset 14.40+) | x86 (32-bit) | `xmake f -p windows -a x86 --toolchain=msvc -m release` |
| **LLVM Clang** | Clang 22.1.0+ (`x86_64-pc-windows-msvc` target) | x86 (32-bit) | `xmake f -p windows -a x86 --toolchain=clang -m release` |
| **MinGW-w64 (GCC)** | GCC 16.2.0 (`i686-win32-dwarf-rev1`) | x86 (32-bit) | `xmake f -p windows -a x86 --toolchain=mingw -m release` |

### Local Build Commands

```powershell
# 1. Standard Release Build
xmake f --toolchain=msvc -m release
xmake

# 2. Debug Build with Debug Console Output
xmake f --toolchain=msvc -m debug
xmake

# 3. Build + Automatically Deploy to Game Directory and Launch
xmake f --toolchain=msvc -m release --launch_game=true --game_path="C:/ProgramData/FAForever/bin"
xmake

# 4. Clean Rebuild
xmake clean
xmake -r
```

The compiled proxy DLL will be output to:
`build/windows/x86/release/dsound.dll`

---

## 🧩 Architecture & How to Add a New Patch

FAmod uses C++23 named modules (`export module patch.<name>`). All `src/**.cppm` files are automatically discovered and compiled by xmake.

Each patch inherits from the `IPatch` base interface (`core:patch`) and provides metadata, binary hooks, declarative settings binding, and an optional ImGui configuration UI.

### Key Concepts & APIs

* **Localization (`tr`)**: Use `tr("English text", {{Language::Russian, "Текст"}, {Language::Chinese, "文本"}})` for all user-visible strings. Returns a `LocalizedString` which implicitly converts to both `std::string_view` and `const char*` / `.c_str()`.
* **Settings (`SettingsBinder`)**: Declarative two-way serialization via `b.Bind("Key.name", variable, default_value)`. Automatically handles both loading on startup and saving when the user modifies settings in the UI.
* **In-game Console Variables**: Register console variables with `fa::ConDescReg` so players can inspect or toggle values directly from the in-game console (`~`).
* **JIT Code Lifetime**: When using `Xbyak::CodeGenerator` or `rcmp` hooks inside `Apply()`, ensure code buffers and variable descriptors remain alive across the process lifetime (e.g. `static Trampoline tramp;` or member variables).

---

### Patch Implementations

#### Option A: Single-File Patch (`src/patches/my_feature/patch.cppm`)

Suitable for small patches with straightforward settings and hooks:

```cpp
module;
#include <imgui.h>
#include <xbyak/xbyak.h>

export module patch.my_feature;
import fa;
import core;

using namespace fa;

export class MyFeaturePatch : public IPatch {
public:
  inline static bool enabled_{true};

  [[nodiscard]] std::string_view Name() const noexcept override {
    return tr("My Feature",
              {{Language::Russian, "Моя фича"},
               {Language::Chinese, "我的功能"}});
  }

  [[nodiscard]] std::string_view Description() const noexcept override {
    return tr("Brief description of what this patch does.",
              {{Language::Russian, "Краткое описание работы патча."},
               {Language::Chinese, "补丁功能说明。"}});
  }

  void Apply() override;

  void RenderUi() override {
    ImGui::Checkbox(
        tr("Enable feature",
           {{Language::Russian, "Включить функцию"},
            {Language::Chinese, "启用功能"}}),
        &enabled_);
    ImGui::SetItemTooltip(
        "%s",
        tr("Detailed explanation of this setting.",
           {{Language::Russian, "Подробное объяснение этой настройки."},
            {Language::Chinese, "该设置的详细说明。"}})
            .c_str());
  }

  void BindSettings(SettingsBinder &b) override {
    b.Bind("MyFeature.enabled", enabled_, true);
  }
};

// 1. Direct in-place address patch at a fixed game code address
struct PatchEngineJump : public Xbyak::CodeGenerator {
  explicit PatchEngineJump(const void *trampoline_address)
      : Xbyak::CodeGenerator(5, reinterpret_cast<void *>(0x00 GAME-ADDRESS)) {
    setProtectMode(PROTECT_RWE);
    jmp(trampoline_address); // or call(trampoline_address)
    setProtectModeRE();
  }
};

// 2. Dynamic trampoline / hook body generated in memory
struct FeatureTrampoline : public Xbyak::CodeGenerator {
  FeatureTrampoline() {
    mov(al, byte[reinterpret_cast<const void *>(&MyFeaturePatch::enabled_)]);
    test(al, al);
    // ... custom assembly instructions ...
    ret();
  }
};

void MyFeaturePatch::Apply() {
  // Keep JIT trampoline buffer alive in memory across the process lifetime
  static FeatureTrampoline tramp;

  // Apply in-place address patch redirecting execution to trampoline
  PatchEngineJump patch(tramp.getCode<const void *>());

  // Optional: register in-game console variable (e.g. `ren_MyFeature`)
  static ConDescReg var("ren_MyFeature", "Toggle my feature (true/false)", &enabled_);
}
```

---

#### Option B: Patch Without Settings / UI (`src/patches/my_fix/patch.cppm`)

For patches that only apply fixes or optimizations without user-configurable options, `RenderUi()` and `BindSettings()` can be omitted entirely (the base class provides defaults):

```cpp
module;
#include <xbyak/xbyak.h>

export module patch.my_fix;
import core;

export class MyFixPatch : public IPatch {
public:
  [[nodiscard]] std::string_view Name() const noexcept override {
    return tr("My Engine Fix",
              {{Language::Russian, "Исправление движка"},
               {Language::Chinese, "引擎修复"}});
  }

  [[nodiscard]] std::string_view Description() const noexcept override {
    return tr("Fixes an internal engine issue.",
              {{Language::Russian, "Исправляет ошибку в движке игры."},
               {Language::Chinese, "修复游戏引擎内部问题。"}});
  }

  void Apply() override {
    // Apply hooks / memory modifications
  }
};
```

---

#### Option C: Multi-File Patch (`patch.cppm` + `ui.cppm`)

For complex patches with extensive ImGui menus, separate the patch logic and UI into two files in `src/patches/<patch_name>/`:

1. `src/patches/my_feature/patch.cppm`:
   ```cpp
   module;
   #include <xbyak/xbyak.h>

   export module patch.my_feature;
   import fa;
   import core;

   using namespace fa;

   export class MyFeaturePatch : public IPatch {
   public:
     inline static bool enabled_{true};

     [[nodiscard]] std::string_view Name() const noexcept override {
       return tr("My Feature", {{Language::Russian, "Моя фича"}, {Language::Chinese, "我的功能"}});
     }
     [[nodiscard]] std::string_view Description() const noexcept override {
       return tr("Description...", {{Language::Russian, "Описание..."}, {Language::Chinese, "描述..."}});
     }

     void Apply() override;
     void RenderUi() override; // Implemented in ui.cppm

     void BindSettings(SettingsBinder &b) override {
       b.Bind("MyFeature.enabled", enabled_, true);
     }
   };

   void MyFeaturePatch::Apply() {
     // Apply hooks
   }
   ```

2. `src/patches/my_feature/ui.cppm`:
   ```cpp
   module;
   #include <imgui.h>

   module patch.my_feature;
   import core;

   void MyFeaturePatch::RenderUi() {
     ImGui::Checkbox(tr("Enable feature", {{Language::Russian, "Включить"}, {Language::Chinese, "启用"}}), &enabled_);
   }
   ```

---

### Registering the Patch in `core.app`

In [`src/core/app.cppm`](src/core/app.cppm):

1. **Import** your patch module at the top of the file:
   ```cpp
   import patch.my_feature;
   ```
2. **Register** the patch class inside `Initialize()`:
   ```cpp
   registry.RegisterPatch<MyFeaturePatch>();
   ```

Run `xmake` to build. The patch will automatically appear in the in-game GUI with localization, hotkey integration, and persistent JSON configuration.

---

## 📄 License

This project is licensed under the [MIT License](LICENSE).
