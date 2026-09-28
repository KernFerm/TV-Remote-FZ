# Changelog

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

