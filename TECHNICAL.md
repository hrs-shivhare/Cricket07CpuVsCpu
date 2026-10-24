# Cricket07CpuVsCpu 1.1

An x86 ASI plugin for CPU-versus-CPU exhibition and normal tournament gameplay
in EA Sports Cricket 07. It permits Continue on the Play Now and tournament
next-match controller-selection screens when every controller is left in the
middle. The game's existing match setup then assigns CPU control to both batting
and bowling.

## Use

1. Put `Cricket07CpuVsCpu.asi` next to `E:\Cricket07\Cricket07.exe`.
   This installation already has Ultimate ASI Loader as `ddraw.dll`; keep it and
   `Cricket07FileLoader.asi` in place.
2. Restart the game. Open **Play Now**, or choose **Play** for your tournament
   fixture so its controller-selection screen opens.
3. At side/controller selection, leave the keyboard and all gamepads in the
   centre, assigned to neither side. Continue should now be enabled.
4. Continue through match settings and the normal pre-match screens.
5. Selecting a human side still uses normal human-versus-CPU or human-versus-human
   behaviour. Exhibition screen `0x0B` and tournament next-match screen `0x14`
   receive the unlock. Keep your tournament's existing team ownership/selection;
   only leave the per-match controllers neutral. Practice, the separate Ashes
   2005 challenge screen and other screens retain the original count check.

`Cricket07CpuVsCpu.log` beside the plugin records whether initialization applied
the hook. A successful startup contains `APPLIED`. `NOT APPLIED` means the plugin
did not modify the loaded game code.

To uninstall, close the game and remove only `Cricket07CpuVsCpu.asi`. The optional
log may also be removed. The plugin does not write Cricket07.exe, roster files,
shot resources, or existing loader files. Its code patch lasts for that process.

## Supported executable

SHA-256:

`d776dba0cc506b968f7d228e036cf42aeb4abe45a8d1ad0d7226cdebf91553a2`

PE32 x86, original image base `0x400000`, SizeOfImage `6606446`.
Other builds, including AbBh.exe, are deliberately refused until separately
analysed. This plugin was developed from the installed Cricket07.exe; it does not
reproduce or claim to identify Abhinav Bhat's implementation.

## Implementation and evidence

Addresses below are original executable virtual addresses (RVA = VA - 0x400000).

* `0x47A520`: controller-screen update. At `0x47AC38` it tests human counts
  `0x90E50C` and `0x90E50D`. Both zero clears bit 0 of the Continue control's
  `+0x35` byte; either nonzero sets that bit. Enabled branch: `0x47AC5B`.
* `0x487930`: returns frontend state `0x95BFE8`. Its first pointer identifies the
  current screen; the word at screen `+4` is the screen ID.
* The plugin detours seven bytes at `0x47AC38` to an allocated 46-byte x86 stub.
  It preserves EAX/EFLAGS and takes the enabled branch only for screens `0x0B`
  and `0x14`.
  Otherwise it replays the displaced comparison and resumes at `0x47AC3F`.
* `0x47FD60` (`OnCheckControllers`) reads that same enabled bit. Neutral
  controllers go through `0x487D90` to remove their side assignments. It does
  not silently assign them to a human side. Exhibition requests route `0x0C`;
  the tournament screen requests the native non-exhibition route `0x3C`.
* Actual installed `fe_next_match_cntrlselect.rfe` identifies tournament screen
  `20` (`0x14`), with `OnTmntControllerSelectTransitionIn` (`0x4F2930`) and
  Continue callback `OnCheckControllers`. The enter callback installs the same
  controller updater `0x47A520`. Fixture entry `OnTournPlayHumanMatch` (`0x531680`)
  opens that screen. Thus the version 1.0 exhibition-only screen guard was the
  reason normal tournament Continue remained blocked.
* The Ashes 2005 screen is `0x1FB` and has a separate Continue callback,
  `OnAshes05LoadMatch` (`0x518F80`), which explicitly forces a human-side flag.
  It is excluded rather than being claimed as supported by the normal unlock.
* `0x4A6870` rebuilds batting and bowling control independently. Zero assigned
  controllers sets each corresponding CPU flag to 1. Native lifecycle calls
  rebuild this state during setup/innings; the plugin does not repeatedly force
  the flags or add a polling thread.
* `0x488100` explicitly tests whether both CPU flags are 1. `0x41DBE0` uses this
  for a 15-second automatic advance on certain native UI objects. This is not a
  promise that every menu in the game auto-advances.
* `0x481510` is `OnStartPractice`; its "One side must be selected" warning is a
  separate path and is not patched.

Primary local references in the existing generated translation:

* `out_all/recomp_015.c`: lines 5843 onward (gate), 12947 onward (controller check).
* `out_all/recomp_016.c`: 11914 onward (assignments), 12330 onward (both-CPU test).
* `out_all/recomp_020.c`: 2303 onward (match control rebuild).

Those C files are instruction translations, not original source. Native PE bytes
and the frontend callback table were checked separately. Installation validates
the complete executable SHA-256, loaded PE identity, 39 bytes around the gate,
and the frontend getter. Unexpected code prevents application.

`DllMain` only remembers the module and disables thread notifications. Ultimate
ASI Loader invokes exported `InitializeASI` after loading; hashing, logging and
allocation happen there, outside the loader lock. Initialization is idempotent.
Do not manually unload the DLL during a running game. Close/restart to remove it.

## Build and validation

The checked build uses portable [Zig 0.15.2](https://ziglang.org/download/) as a C
cross-compiler for `x86-windows-gnu`. No global compiler installation is needed.
The downloaded official toolchain archive SHA-256 is:

`3a0ed1e8799a2f8ce2a6e6290a9ff22e6906f8227865911fb7ddedc3cc14cb0c`

From this directory:

```powershell
.\build.ps1 -Zig C:\path\to\zig.exe
```

The default path uses the workspace's `work\toolchains` copy. `-Install` builds,
tests, checks the game hash, and installs only if the destination is absent and
the game is closed. `-Upgrade` may replace the known version 1.0 after copying it
to `output/plugins/backups/Cricket07CpuVsCpu-1.0.asi.bak`. It refuses an unknown
installed plugin. The version 1.0 ZIP also remains available for rollback.

Tests execute the actual generated x86 stub in a separate 32-bit fixture:

* Exhibition and tournament screens; practice, Ashes 2005, unrelated screens,
  similar low-byte screen IDs and a null screen.
* All four combinations of empty/nonempty home and away assignments.
* Both old enabled-bit values; unrelated flag bits retained.
* Altered instruction contexts and already-hooked contexts rejected.
* ASI LoadLibrary/import/export smoke test.
* Unsupported host executable refused; repeated initialization safe.

There are 72 native screen/assignment/old-state execution cases. These tests do
not substitute for playing through a complete match in Cricket07.exe. Refer to
VALIDATION.md for the observed build/runtime results and remaining checks.
