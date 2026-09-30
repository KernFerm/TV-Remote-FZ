# Testing

## Automated and build checks

- Compile `tests/core_test.c` with `tv_remote_core.c` and run it.
- Build with `python -m ufbt build`.
- Confirm `APPCHK` reports target 7/API 87.1 and no unresolved symbols.

## Parser cases

- Valid parsed `.ir` file.
- Valid RAW `.ir` file.
- Mixed parsed/RAW file.
- Wrong header/version, empty file, malformed protocol, invalid address/command width.
- RAW count 0, count 1, count 1,024, count 1,025, invalid frequency/duty, allocation failure.
- More than 224 commands is rejected as an unsupported oversized remote rather than silently called complete.
- SD removal during import and immediately before transmission.

## Physical Flipper checklist (required before UI completion)

1. Add a real `.ir` remote and reopen the application.
2. Confirm the favorite opens directly when the setting is enabled, then restart the Flipper and confirm it remains saved.
3. Select a bundled profile and verify its `.ir` file exists under `/ext/infrared/TV_Remote_FZ` and `tvs.cfg` exists under `/ext/apps_data/tv_remote_fz`.
4. Replace a previously selected bundled profile while simulating a failed write or full card; confirm the previously saved profile remains usable and temporary/backup files are not left behind after a successful retry.
5. Inspect all populated pages on the physical 128x64 display for overlap/clipping.
6. Confirm selected controls are obvious and long names remain within the header.
7. Send Power, Mute, Input, Volume +/-, and Channel +/- where present.
8. Hold/release Volume and Channel; confirm release and Back stop transmission immediately.
9. Verify D-pad direct mode, hold-OK Home/Menu/Back/Exit focus, ordered number grid, media controls, named Extra commands, and All Commands.
10. Confirm missing actions do not appear and page switching never transmits.
11. Rapidly press controls, switch pages, reopen, rename, and alternate between saved TVs.
12. Remove the SD card during a safe non-write test and confirm an error is shown instead of a false send result.
13. On a remote containing discrete inputs, confirm the Inputs page contains only its real HDMI/AV/Component/TV commands.
14. Open the bundled **Insignia NS 40D510NA17** profile and confirm the Numbers page is arranged as `1-9`, dash, `0`, and Enter in conventional 3x4 positions.
15. On the Numbers page, move focus with the D-pad and confirm OK transmits the highlighted digit or Enter signal.
16. Open Navigation and confirm short Up, Down, Left, Right, and OK directly transmit the matching TV commands without moving a graphical cursor.
17. Confirm the generic `Enter` signal acts as Navigation OK/Select and also occupies the keypad Enter position when no distinct numeric Enter command exists.
18. On a remote containing Previous/Last, confirm it remains reachable from the numeric continuation page.
15. Confirm Remote Information shows the saved brand, remote family, and correct official or community database source for a bundled remote.
16. Under Insignia, confirm the three official profiles and the additional community `Insignia` profile appear; verify the source label of each.
17. Confirm All Commands is grouped by category and still contains every original command name.

Record the tested firmware, remote file, screen observations, and results. Source inspection or emulator-only testing does not satisfy the physical UI requirement.
