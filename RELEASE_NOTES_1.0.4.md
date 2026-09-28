# TV Remote FZ v1.0.4

TV Remote FZ is a graphical infrared television remote for Flipper Zero. It includes 3,051 genuine remotes from the official Flipper Devices IRDB and never invents missing signals.

## What is new

- Bundled selections now retain their manufacturer, remote family, and database source in Remote Information.
- Existing bundled remotes saved by v1.0.2 are migrated automatically.
- A dedicated Inputs page shows only genuine Input, HDMI/HDMI 1-4, AV, Component 1-3, and TV commands present in the selected remote.
- Enter and Previous/Last are available from the numeric page when present.
- All Commands is grouped into Quick, TV, Navigation, Numbers, Media, Inputs, and Extra while preserving every original command name and signal.
- Version and supporting documentation are updated for v1.0.4.

## Existing capabilities

- Graphical Quick, Channel, Navigation, Numbers, Media, Inputs, and Extra pages.
- Exact parsed and RAW signal transmission.
- Controlled held repeat for Volume and Channel.
- Up to eight saved televisions, friendly names, favorite startup, TX feedback, and optional vibration.
- Import of existing or learned Flipper `.ir` files.

## Install

Download `tv_remote_fz.fap` and copy it to `/ext/apps/Infrared/` on the Flipper microSD card.

**First installation:** Allow about two minutes while the Flipper extracts and signs the bundled TV database. This happens once; later launches are fast.

Requires official Flipper firmware 1.4.3 or newer. Built for target f7/API 87.1.

Application code is GNU GPL v3 or later. The included Flipper Devices IRDB data retains its MIT license and attribution.
