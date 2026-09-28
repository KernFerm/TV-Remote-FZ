# TV Remote FZ

TV Remote FZ is a graphical infrared remote for Flipper Zero. It ships with 3,051 genuine TV remotes organized by brand and model, imports other Flipper `.ir` files, remembers up to eight televisions, and presents only commands actually stored in the selected file. It does not generate, infer, or substitute IR codes.

Version: **1.0.2**  
Firmware target: official Flipper firmware **1.4.3+**, target f7 / API 87.1  
License: **GNU GPL v3 or later**

## Install

1. Download `tv_remote_fz.fap`.
2. Connect the Flipper Zero by USB and close qFlipper if another tool will copy the file.
3. Copy the FAP to `/ext/apps/Infrared/tv_remote_fz.fap` on the Flipper microSD card.
4. Open **Apps → Infrared → TV Remote FZ**.

**First launch:** Allow about two minutes while the Flipper extracts and signs the bundled TV database. This happens once; later launches reuse the extracted database and load quickly.

## Set up a television from the included catalog

1. Open **Set Up TV**.
2. Select the first letter of the manufacturer, the manufacturer, and then a remote/model code.
3. The app validates the selected signals, saves the selection, and opens the graphical remote.
4. Point the Flipper's infrared window at the TV and test Power or Volume. If that family does not respond, go Back and select another model for that brand.

Infrared TVs do not pair like Bluetooth devices. Selecting a matching code file is the infrared equivalent of setup. The included catalog is from the official Flipper Devices IRDB at revision `f7b15366521cc81ba11b341538f0097bddbb748b`.

## Add an existing or learned television

1. Create or download a valid Flipper **IR signals file** (`.ir`). The stock Infrared application can learn and save commands.
2. Open TV Remote FZ and select **Add Existing .ir**.
3. Choose the file once. The app validates the header and every signal it imports.
4. The first valid television becomes the favorite. Hold **OK** while its graphical remote is open to rename it or change the favorite.

The app remembers the file path; it does not duplicate or alter the source remote. Keep that `.ir` file on the microSD card.

## Controls

- **Short Left/Right/Up/Down**: move between graphical buttons.
- **OK**: send the selected command.
- **Hold OK on Volume/Channel buttons**: controlled repeat; release stops immediately.
- **Hold Left/Right**: move between Quick, Channel, Navigation, Numbers, Media, and Extra pages that contain real commands.
- **Navigation page**: the physical directional pad and OK directly send the matching TV navigation commands. Hold **OK** to switch to a focused Home/Menu/Back/Exit panel; hold **OK** again to return to direct control.
- **Hold OK on any non-Navigation remote page**: open Remote Information, Rename, Favorite, All Commands, or Remove.
- **Back**: return without transmitting.

`TX Sent` means the Flipper transmitted the stored signal. Infrared is normally one-way, so the app cannot claim that the television received or acted on it.

## Real signal behavior

- Parsed signals retain their exact protocol, address, and command.
- RAW signals retain their exact carrier frequency, duty cycle, and timing list.
- Alias normalization affects only where a command appears in the UI.
- Unknown and manufacturer-specific commands remain available under **Extra** and **All Commands**.
- The number page is always arranged 1–9, then 0 and dash, independent of source-file order.
- Missing actions create no dead or guessed buttons.

## Build

With uFBT configured for firmware 1.4.3/API 87.1:

```powershell
Set-Location "C:\Users\frost\OneDrive\Desktop\flipper-applications\tv-remote"
python -m ufbt build
```

The FAP is written to `.ufbt\build\tv_remote_fz.fap` (the resolved global uFBT build directory may be under the user profile).

## Limitations

- The standard `.ir` format does not contain formal brand, TV model, remote family, or database-source fields. The information screen reports those as unavailable rather than guessing them.
- A remote can contain up to 224 commands in this bounded port; a larger file is rejected rather than partially presented as complete.
- TV control is limited to the Flipper's infrared hardware and the signals in the chosen file.

See [TESTING.md](TESTING.md), [SECURITY.md](SECURITY.md), and [FEATURE_MATRIX.md](FEATURE_MATRIX.md).
