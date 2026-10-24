# Cricket 07 CPU vs CPU — ASI Plugin

**Version 1.1**

Watch both teams play under computer control in **EA Sports Cricket 07 for Windows**. This plugin enables **Continue** when no controller is assigned to either side, allowing the game's existing CPU-versus-CPU gameplay to run.

CPU-versus-CPU gameplay has been confirmed in **exhibition matches and normal tournament fixtures**.

## Features

- CPU vs CPU in **Play Now / exhibition** matches.
- CPU vs CPU in **normal tournament matches** through the next-match controller screen.
- Human vs CPU and human vs human remain available by assigning controllers normally.
- No configuration file or hotkey required.
- Applies its changes in memory while the game runs. The executable, rosters and shot files on disk are untouched.

## Requirements

- EA Sports Cricket 07 for Windows, using the supported executable described below.
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

### Updating from version 1.0

Close the game and replace the old `Cricket07CpuVsCpu.asi` with version 1.1. Keep only one active copy of this plugin. Store backups outside the game's ASI-loading folders.

**Version 1.0 supports exhibition only. Version 1.1 adds normal tournament matches.**

## How to use

### Exhibition / Play Now

1. Open **Play Now** and select your teams.
2. On the side/controller-selection screen, leave the **keyboard and every gamepad in the centre**, assigned to neither team.
3. Select **Continue**.
4. Proceed through the normal pre-match screens. Both teams will use CPU control during gameplay.

To play a normal human-controlled match, assign the controller to a team as usual.

### Tournament matches

1. Load or create your tournament normally.
2. Keep your chosen tournament team marked **Human** so you can choose to play its fixture.
3. Choose **Play** for the fixture.
4. On that match's controller-selection screen, leave **all controllers in the centre**.
5. Select **Continue** and proceed into the match.

**Tournament team ownership and per-match controller assignment are separate.** Keep the tournament team selected; leave only the match's controllers unassigned.

## Compatibility and limitations

| Mode | Version 1.1 |
|---|---|
| Exhibition / Play Now | CPU-vs-CPU gameplay confirmed |
| Normal tournament fixtures | CPU-vs-CPU gameplay confirmed |
| Human vs CPU / human vs human | Original controller assignment remains available |
| Practice nets | Not enabled by this plugin |
| Ashes 2005 challenge mode | Not supported |

The historical **Ashes 2005 challenge screen** uses a separate match-start path and is excluded from this version.

Innings changes, completed matches and post-match tournament standings updates have not yet been explicitly verified. The plugin does not make every menu automatic; some pre-match screens may still need input.

### Supported executable

Version 1.1 supports the `Cricket07.exe` build with this SHA-256:

```text
d776dba0cc506b968f7d228e036cf42aeb4abe45a8d1ad0d7226cdebf91553a2
```

The plugin checks the executable and hook instructions before applying. Other executable builds, including `AbBh.exe`, are not currently supported.

To check your executable, open PowerShell in the game folder and run:

```powershell
Get-FileHash -LiteralPath .\Cricket07.exe -Algorithm SHA256
```

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

## Version history

### 1.1

- Added the normal tournament next-match controller screen.
- Preserved exhibition support and the original checks on other screens.

### 1.0

- Initial release with exhibition CPU-vs-CPU support.

## Building from source

The included PowerShell script builds the 32-bit plugin and runs its native tests using **Zig 0.15.2** as a C compiler:

```powershell
.\build.ps1 -Zig "C:\path\to\zig.exe"
```

The compiled plugin is written to `dist/Cricket07CpuVsCpu.asi`. Version 1.1 passes **72 native screen/controller-state test cases**, plus loading, executable-compatibility and conflicting-hook checks.

See [TECHNICAL.md](TECHNICAL.md) for implementation details and [VALIDATION.md](VALIDATION.md) for the distinction between automated tests and confirmed gameplay.

## Credits

- **ThirteenAG and contributors** — [Ultimate ASI Loader](https://github.com/ThirteenAG/Ultimate-ASI-Loader).

