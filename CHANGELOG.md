# Changelog

## 1.0.2 - 2026-09-27

- Bundled 3,051 genuine television remotes from the official Flipper Devices IRDB revision `f7b15366521cc81ba11b341538f0097bddbb748b`.
- Added the on-device **Set Up TV** flow: brand letter, manufacturer, and remote/model code.
- Replaced thousands of loose application assets with one validated, bounded database to prevent excessive first-launch file creation.
- Added safe extraction of only the selected remote into application data.
- Added clearer Settings labels and an About page with version and database licensing.
- Preserved exact parsed and RAW infrared records; no codes are inferred, substituted, or fabricated.
- Validated formatting, linting, and the target f7/API 87.1 build.

