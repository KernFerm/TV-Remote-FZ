# Changelog

## 1.1.1 - 2026-09-29

- Treats a generic IR command named `Enter` as the TV navigation OK/Select action.
- Reuses that Enter signal in the numeric keypad's Enter position when no separately named numeric Enter command exists.
- Keeps explicit `Num_enter`, `Channel_enter`, and `Digit_enter` commands classified as numeric Enter.

## 1.1.0 - 2026-09-29

- Added purpose-built Quick, numeric keypad, Media, and Extra layouts instead of rendering every page as the same grid.
- Added geometry-aware focus navigation so the physical D-pad follows each page's visual arrangement.
- Added width-aware label truncation for long manufacturer-specific commands and long saved-TV names.
- Moved all subpage indicators into the header and removed the overlapping bottom-corner counter.
- Repositioned the Navigation D-pad and helper label so they no longer compete for the same pixels.

## 1.0.10 - 2026-09-29

- Recognizes the common IRDB command name `Vol_dn` as Volume Down, restoring the missing button on both saved Insignia profiles and other affected remotes.

## 1.0.9 - 2026-09-29

- Automatically removes duplicate My TVs entries that point to the same saved `.ir` profile while preserving the favorite.
- Selecting an already-saved setup profile now reopens its existing My TVs entry instead of adding another copy.

## 1.0.8 - 2026-09-29

- Replaced the cramped 12-button, three-column remote grid with six larger buttons in two columns.
- Added automatic six-command paging while preserving directional selection across the full command list.
- Prevented Left and Right from unexpectedly wrapping between grid rows.

## 1.0.7 - 2026-09-29

- Back from a remote opened during Set Up TV now returns to the current brand's profile list, making it quick to try another profile.
- Back from a remote opened through My TVs returns to My TVs; a Favorite still returns to the main menu.

## 1.0.6 - 2026-09-29

- Fixed `Furi check failed` when opening a selected or favorite TV profile by correctly acquiring the locking view model before every redraw commit.

## 1.0.5 - 2026-09-29

- Expanded the bundled catalog from 3,051 to 3,207 verified TV profiles by merging unique valid profiles from the maintained CC0 Flipper-IRDB with the current official Flipper Devices IRDB.
- Catalog generation now excludes files missing the required Flipper IR header/version.
- Added a fourth Insignia profile while retaining all three official Insignia profiles and recording the correct source for every catalog entry.
- Saved selected bundled profiles to the visible microSD folder `/ext/infrared/TV_Remote_FZ`.
- Made settings storage explicit at `/ext/apps_data/tv_remote_fz/tvs.cfg`.
- Added rollback-safe profile replacement so a failed SD-card write does not destroy an existing saved profile.
- Replaced misleading "code" wording with "profile" where appropriate; vendor setup numbers such as `7838` are not standardized IR signal definitions.

## 1.0.4 - 2026-09-27

- Preserved structured manufacturer, remote-family, and database-source metadata for bundled remotes.
- Added automatic metadata migration for bundled remotes saved by v1.0.2.
- Added a dynamic Inputs page for genuine HDMI, HDMI 1-4, AV, Component 1-3, and TV commands.
- Added Enter and Previous/Last to the number-pad page when the selected remote contains them.
- Grouped All Commands into Quick, TV, Navigation, Numbers, Media, Inputs, and Extra categories without changing original command names.
- Expanded the physical-device verification checklist for the new pages and metadata.
- Clean-built and validated for target f7/API 87.1.

## 1.0.2 - 2026-09-27

- Bundled 3,051 genuine television remotes from the official Flipper Devices IRDB revision `f7b15366521cc81ba11b341538f0097bddbb748b`.
- Added the on-device **Set Up TV** flow: brand letter, manufacturer, and remote/model code.
- Replaced thousands of loose application assets with one validated, bounded database to prevent excessive first-launch file creation.
- Added safe extraction of only the selected remote into application data.
- Added clearer Settings labels and an About page with version and database licensing.
- Preserved exact parsed and RAW infrared records; no codes are inferred, substituted, or fabricated.
- Validated formatting, linting, and the target f7/API 87.1 build.

