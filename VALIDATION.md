# Validation record — 2026-10-22

## Static verification

The supplied executable hash matches the supported build. The real controller
screen gate, screen ID, neutral-controller transition, match-manager rebuild,
and built-in both-CPU predicate were traced in the local instruction translations
and checked against the original PE bytes/callback table. Existing game and
loader files are retained.

## Native fixture

The x86 plugin and fixture compile with warnings treated as errors. All 24
screen/assignment/old-state combinations pass. Loading the ASI into the 32-bit
fixture succeeds; exact undecorated exports exist; the executable hash guard
refuses the fixture without patching it. Conflicting or changed hook bytes are
rejected. An initially incorrect relative jump displacement was caught by this
native execution test and corrected before installation.

## Actual-game startup

Launched the installed `E:\Cricket07\Cricket07.exe`. Ultimate ASI Loader loaded
`E:\Cricket07\Cricket07CpuVsCpu.asi`; its log reports `APPLIED`. A read-only
OpenProcess/ReadProcessMemory observation confirmed the loaded module, seven-byte
detour and all 39 executable stub bytes in the real game. The process remained
responsive. The observation is retained in `tests/runtime_startup.json`.

At the startup observation, controller counts were both zero, but the current
screen was `0x336`, not the exhibition controller screen. This observation alone
did not establish that a match started.

## User-confirmed gameplay

On 2026-10-22, the user tested Play Now with controllers left unassigned and
reported: "Both teams play as CPU". This confirms that the installed plugin
permits progression into a playable CPU-versus-CPU match. It is user-reported
gameplay verification, separate from the read-only startup observation above.
An innings change and a completed full match have not yet been explicitly
confirmed.

The version 1.0 plugin SHA-256 is:

`084ae352e9c8227cb08ef7fe4fed0b73d043b39abec99902e54b2722e28f99b5`

The original executable SHA-256 was checked after game startup and remained
`d776dba0cc506b968f7d228e036cf42aeb4abe45a8d1ad0d7226cdebf91553a2`.

## Version 1.1 — tournament selection

The user reported exhibition working but a tournament match remaining blocked.
Version 1.0 intentionally allowed only screen `0x0B`. The installed tournament
asset `fe_next_match_cntrlselect.rfe` uses screen `0x14`, enters through
`0x4F2930`, installs the same `0x47A520` updater, and continues through the same
`OnCheckControllers` callback (`0x47FD60`). Fixture entry `0x531680` opens `0x14`.
No further zero-human rejection was found in that shared controller callback.

Version 1.1 adds `0x14` to the whitelist, retaining `0x0B` and preserving all
other screen behaviour. The executable stub is now 46 bytes. All 72 native
screen/assignment/old-state cases pass, including tournament, exhibition,
practice, Ashes 2005, unrelated IDs, low-byte aliases and a null screen. ASI
load/export, unsupported executable and signature-conflict tests also pass.

Version 1.1 plugin SHA-256:

`0337c31cea334d3808a100cffaba15b254dc8d4b8c9de417f1ff1d92f7985d91`

The known version 1.0 was backed up outside the game directory before replacing
the installed ASI. The game was restarted; the 1.1 log reports `APPLIED`, and
read-only process inspection confirmed its installed detour and 46-byte stub,
including the additional comparison against screen ID `0x14`. The game remained
responsive. See `tests/runtime_startup_v1.1.json`. Cricket07.exe retains its
original supported SHA-256.

On 2026-10-22, after installing version 1.1, the user tested a tournament fixture
with per-match controllers centred and reported: "Tournament CPU vs CPU works".
This confirms progression into tournament CPU-versus-CPU gameplay, in addition
to the earlier exhibition confirmation. Innings changes, a completed match and
post-match tournament standings remain unverified.
The separate Ashes 2005 screen `0x1FB` is excluded: its loader `0x518F80` has
additional code forcing a human-side flag, unlike the shared tournament path.
