# Replay Desync Fix for Supreme Commander: Forged Alliance (FAForever)

Standalone binary patch implementing the fix for replay desynchronization caused by `CommandSourceTerminated`.

## Overview

According to tests by [Strogoo](https://github.com/Strogoo) and reverse-engineering insights from [Draiget](https://github.com/Draiget), `CMDST_CommandSourceTerminated` clears the internal `hasCommandSource` flag on stack (`[esp + 0x16]`) in `CClientBase::Process`. 

In live games each player has their own `CClientBase`, but in replays all command sources are funneled through a single `CReplayClient`. Because the authorized `CMDST_SetCommandSource` handler never restores `hasCommandSource` on the stack, all subsequent command payloads in that beat are dropped, causing replay desync on the tick after someone leaves.

Reference: [FAForever/FA-Binary-Patches #172](https://github.com/FAForever/FA-Binary-Patches/pull/172).

### Patch Details
```assembly
hasCommandSource = -0x8E
0x0053C71E:
    mov     byte ptr [esp + 0x0A4 + hasCommandSource], 1
    jmp     0x0053C7D7
    nop
```

## Building

Requires [xmake](https://xmake.io):
```bash
xmake
```
This produces `dsound.dll` in `build/windows/x86/release/dsound.dll`.

### Options

- `--launch_game=y`: Automatically copies `dsound.dll` to the game folder and launches `ForgedAlliance.exe` after building.
- `--game_path="path"`: Path to the directory containing `ForgedAlliance.exe` (default: `C:/ProgramData/FAForever/bin`).

Example:
```bash
xmake f --launch_game=y
xmake
```
or with custom game path:
```bash
xmake f --launch_game=y --game_path="C:/ProgramData/FAForever/bin"
xmake
```
