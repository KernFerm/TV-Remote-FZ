# TV Remote FZ v1.0.2

TV Remote FZ turns the Flipper Zero into a graphical infrared television remote using genuine stored IR signals.

## Included in this release

- 3,051 television remotes from the official Flipper Devices IRDB.
- Guided setup by brand letter, manufacturer, and remote/model code.
- Graphical Quick, Channel, Navigation, Numbers, Media, and Extra pages.
- Exact parsed and RAW signal transmission without guessed codes.
- Support for importing an existing or learned Flipper `.ir` file.
- Up to eight saved televisions, favorite startup, friendly names, command browser, optional vibration, and transmission feedback.
- Compact indexed database packaging that avoids unpacking thousands of loose files.

## Install

Download `tv_remote_fz.fap` and copy it to `/ext/apps/Infrared/` on the Flipper microSD card.

**Important:** The first launch takes about two minutes while the Flipper extracts and signs the bundled TV database. This happens once; launches after that are fast.

Requires official Flipper firmware 1.4.3 or newer. Built for target f7/API 87.1.

Application code is GNU GPL v3 or later. The included Flipper Devices IRDB data retains its MIT license and attribution.
