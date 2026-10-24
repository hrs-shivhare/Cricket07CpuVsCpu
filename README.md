# Cricket 07 CPU vs CPU — ASI Plugin

**Version 1.1**

Watch both teams play under computer control in **EA Sports Cricket 07**. This plugin enables **Continue** when no controller is assigned to either side, allowing the game's existing CPU-versus-CPU gameplay to run.

CPU-versus-CPU gameplay has been confirmed in **exhibition matches and normal tournament fixtures**.

## Features

- CPU vs CPU in **Play Now / exhibition** matches.
- CPU vs CPU in **normal tournament matches** through the next-match controller screen.
- Human vs CPU and human vs human remain available by assigning controllers normally.
- Applies its changes in memory while the game runs.

## Requirements

- A recent **32-bit / x86 [Ultimate ASI Loader](https://github.com/ThirteenAG/Ultimate-ASI-Loader)** with `InitializeASI` support.
- The plugin file: `Cricket07CpuVsCpu.asi`.

Cricket 07 is a 32-bit game. Use the **x86 loader**, even on 64-bit Windows.

## Installation

1. **Close the game.**
2. Open the folder containing `Cricket07.exe`.
3. If you already have a working ASI loader, keep it. Otherwise, download the **32-bit `ddraw.dll` loader** from the [official Ultimate ASI Loader releases](https://github.com/ThirteenAG/Ultimate-ASI-Loader/releases) and place it beside `Cricket07.exe`.
4. Extract `Cricket07CpuVsCpu.asi` from this plugin's release ZIP and copy it into that same folder.
5. Start the game.

Example folder layout:

```text
Cricket07/
├── Cricket07.exe
├── ddraw.dll                  ← ASI loader
└── Cricket07CpuVsCpu.asi       ← this plugin
```

If `ddraw.dll` already exists, identify it before replacing anything: it may be an existing loader or graphics wrapper. Keep other working ASI plugins in place.

**Tournament team ownership and per-match controller assignment are separate.** Keep the tournament team selected; leave only the match's controllers unassigned.

The historical **Ashes 2005 challenge screen** uses a separate match-start path and is excluded from this version.

### Supported executable

Version 1.1 supports the original `Cricket07.exe`

## Troubleshooting

### Continue is still disabled

- Check that **every controller is centred**, including connected gamepads.
- For tournaments, confirm you installed **version 1.1** and restarted the game.
- Check `Cricket07CpuVsCpu.log` beside the plugin.

### What should the log say?

A successful initialization contains:

```text
Cricket07CpuVsCpu 1.1: initializing (exhibition and tournament screens).
APPLIED: controller Continue gate hooked at RVA 0x7AC38.
```

- **`APPLIED`** means the plugin loaded and installed its hook.
- **`NOT APPLIED: executable SHA-256 ...`** means your executable differs from the supported build.
- **`NOT APPLIED: code signature conflict ...`** means the expected instructions were unavailable, or memory setup failed. Another mod may have changed that code.
- **No log file** can mean the ASI loader did not load or initialize the plugin, or the game folder is not writable. Check the loader installation and use a recent version supporting `InitializeASI`.

When reporting a problem on GitHub or PlanetCricket, include the game mode, plugin version, executable SHA-256 and relevant log lines.

## Uninstallation

1. Close the game.
2. Remove `Cricket07CpuVsCpu.asi`.
3. Optionally remove `Cricket07CpuVsCpu.log`.

Keep the ASI loader if other plugins use it. No executable restoration is needed.

## Building from source

The included PowerShell script builds the 32-bit plugin and runs its native tests using **Zig 0.15.2** as a C compiler:

```powershell
.\build.ps1 -Zig "C:\path\to\zig.exe"
```

The compiled plugin is written to `dist/Cricket07CpuVsCpu.asi`. Version 1.1 passes **72 native screen/controller-state test cases**, plus loading, executable-compatibility and conflicting-hook checks.

See [TECHNICAL.md](TECHNICAL.md) for implementation details and [VALIDATION.md](VALIDATION.md) for the distinction between automated tests and confirmed gameplay.

## Credits

- **ThirteenAG and contributors** — [Ultimate ASI Loader](https://github.com/ThirteenAG/Ultimate-ASI-Loader).

