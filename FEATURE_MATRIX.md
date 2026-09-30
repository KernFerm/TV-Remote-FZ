# Feature matrix

| Requirement | Status | Implementation |
|---|---:|---|
| Graphical primary remote | Implemented | Custom 128×64 Canvas view; no command submenu is used as the primary control |
| Quick controls | Implemented | Purpose-built Power/Mute/Input row with paired Volume and Channel controls; actual aliases only |
| Channel/TV page | Implemented | Actual guide/info/last/favorite/TV/CC/subtitle/audio commands |
| Direct navigation pad | Implemented | Physical D-pad/OK map to present TV navigation signals; generic `Enter` is recognized as OK/Select; held OK toggles a focused Home/Menu/Back/Exit panel |
| Number page | Implemented | Conventional 3x4 keypad positions for actual 1–9, dash, 0, and Enter signals; generic `Enter` fills the keypad Enter slot when no separate numeric Enter exists; Last remains accessible on a continuation screen when present |
| Media page | Implemented | Two rows of large icon controls for actual play/pause/stop/rewind/forward/previous/next/record signals |
| Discrete inputs page | Implemented | Actual generic Input, HDMI/HDMI 1-4, AV, Component 1-3, and TV signals where present |
| Smart/Extra | Implemented | Unclassified/manufacturer-specific commands in full-width rows with font-aware truncation and bounded paging |
| All Commands | Implemented | Complete original-name browser and sender, grouped by Quick/TV/Navigation/Numbers/Media/Inputs/Extra |
| Alias normalization | Implemented | Classification only; signal data is untouched |
| Held volume/channel | Implemented | Official `InfraredWorker` steady callback; release/back/page change/exit stop it |
| Saved/favorite TVs | Implemented | Up to 8 file references; transactional settings at `/ext/apps_data/tv_remote_fz/tvs.cfg`; bundled selections are visible under `/ext/infrared/TV_Remote_FZ` |
| Included TV profiles | Implemented | 3,207 verified profiles: official Flipper Devices IRDB revision `f7b1536` plus unique valid community Flipper-IRDB profiles from revision `d126fb1`, packed into one bounded indexed database |
| Friendly names | Implemented | Rename without changing source filename or signal names |
| Remote information | Implemented | Friendly name, brand, TV model availability, remote family, database source, file path, total, parsed, and RAW counts |
| TX feedback | Implemented | Brief `TX Sent`, blue LED flash, optional haptic |
| Monochrome visual assets | Implemented | Application-owned vector icons in `tv_remote_icons.c` for power, mute, volume, channel, input, home, menu, back, guide, info, and media |
| Text and page spacing | Implemented | Font-aware ellipsis protects long labels; page/subpage indicators remain in the header; Navigation helper text has a reserved footer |
| Missing commands | Implemented | Never displayed or guessed |
| Physical-device setup flow | Verified | User confirmed the v1.0.2 build reaches the brand-letter selector on a physical Flipper Zero |
| Complete physical UI test | Pending user verification | Remaining signal/layout checklist is in `TESTING.md` |
