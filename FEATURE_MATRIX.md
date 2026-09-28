# Feature matrix

| Requirement | Status | Implementation |
|---|---:|---|
| Graphical primary remote | Implemented | Custom 128×64 Canvas view; no command submenu is used as the primary control |
| Quick controls | Implemented | Actual Power, Mute, Input, Volume, and Channel aliases only |
| Channel/TV page | Implemented | Actual guide/info/last/favorite/TV/CC/subtitle/audio commands |
| Direct navigation pad | Implemented | Physical D-pad/OK map to present TV navigation signals; held OK toggles a focused Home/Menu/Back/Exit panel |
| Number page | Implemented | Actual 1–9, 0, and dash signals in deterministic keypad order |
| Media page | Implemented | Actual play/pause/stop/rewind/forward/previous/next/record signals |
| Smart/Extra | Implemented | Unclassified/manufacturer-specific commands, paged in bounded groups |
| All Commands | Implemented | Complete original-name browser and sender |
| Alias normalization | Implemented | Classification only; signal data is untouched |
| Held volume/channel | Implemented | Official `InfraredWorker` steady callback; release/back/page change/exit stop it |
| Saved/favorite TVs | Implemented | Up to 8 file references; transactional settings file |
| Included TV codes | Implemented | 3,051 real remotes from official Flipper Devices IRDB revision `f7b1536`, packed into one bounded indexed database |
| Friendly names | Implemented | Rename without changing source filename or signal names |
| Remote information | Implemented | Friendly name, path/source, total, parsed, and RAW counts |
| TX feedback | Implemented | Brief `TX Sent`, blue LED flash, optional haptic |
| Monochrome visual assets | Implemented | Application-owned vector icons in `tv_remote_icons.c` for power, mute, volume, channel, input, home, menu, back, guide, info, and media |
| Missing commands | Implemented | Never displayed or guessed |
| Physical-device setup flow | Verified | User confirmed the v1.0.2 build reaches the brand-letter selector on a physical Flipper Zero |
| Complete physical UI test | Pending user verification | Remaining signal/layout checklist is in `TESTING.md` |
