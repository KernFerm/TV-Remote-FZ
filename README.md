# TV Remote FZ

TV Remote FZ is a graphical infrared remote for Flipper Zero. It ships with 3,207 verified TV profiles organized by brand and profile name, imports other Flipper `.ir` files, remembers up to eight televisions, and presents only commands actually stored in the selected file. It does not generate, infer, or substitute IR codes.

Version: **1.1.1**

Firmware target: official Flipper firmware **1.4.3+**, target f7 / API 87.1

License: **GNU GPL v3 or later**

## Install

1. Download `tv_remote_fz.fap`.
2. Connect the Flipper Zero by USB and close qFlipper if another tool will copy the file.
3. Copy the FAP to `/ext/apps/Infrared/tv_remote_fz.fap` on the Flipper microSD card.
4. Open **Apps → Infrared → TV Remote FZ**.

### Install a downloaded FAP with PowerShell

This requires uFBT. Connect the Flipper by USB, close qFlipper so it releases the serial port, and run:

```powershell
python "$env:USERPROFILE\.ufbt\current\scripts\runfap.py" -p auto -s "$env:USERPROFILE\Downloads\tv_remote_fz.fap" -t /ext/apps/Infrared/tv_remote_fz.fap
```

Change the path after `-s` if the downloaded file is somewhere other than Downloads. The command copies the FAP to the correct microSD folder and launches it.

### Install a downloaded FAP with Command Prompt

```bat
python "%USERPROFILE%\.ufbt\current\scripts\runfap.py" -p auto -s "%USERPROFILE%\Downloads\tv_remote_fz.fap" -t /ext/apps/Infrared/tv_remote_fz.fap
```

### Build and install directly from this repository

PowerShell:

```powershell
Set-Location "C:\Users\frost\OneDrive\Desktop\flipper-applications\tv-remote"
python -m ufbt launch
```

Command Prompt:

```bat
cd /d "C:\Users\frost\OneDrive\Desktop\flipper-applications\tv-remote"
python -m ufbt launch
```

`ufbt launch` builds the current source, copies the resulting FAP to `/ext/apps/Infrared/tv_remote_fz.fap`, and launches TV Remote FZ.

## Set up a television from the included catalog

1. Open **Set Up TV**.
2. Select the first letter of the manufacturer, the manufacturer, and then a profile name.
3. The app validates the selected signals, saves the selection, and opens the graphical remote.
4. Point the Flipper's infrared window at the TV and test Power or Volume. If that family does not respond, go Back and select another model for that brand.

Infrared TVs do not pair like Bluetooth devices. Selecting a matching profile is the infrared equivalent of setup. Selected profiles are copied to the visible microSD folder `/ext/infrared/TV_Remote_FZ`; settings are saved at `/ext/apps_data/tv_remote_fz/tvs.cfg`.

The catalog combines the official Flipper Devices IRDB at revision `f7b15366521cc81ba11b341538f0097bddbb748b` with unique TV profiles from the maintained community Flipper-IRDB at revision `d126fb1b6f1e114c52b4a8c19839ea65e3a9c24d`. Official profiles win exact name duplicates. Profile names and numbers used by phone dongle applications are vendor-specific and cannot be converted into Flipper signals without an exported or learned IR recording.

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
- **Hold Left/Right**: move between Quick, Channel, Navigation, Numbers, Media, Inputs, and Extra pages that contain real commands.
- **Navigation page**: the physical directional pad and OK directly send the matching TV navigation commands. Hold **OK** to switch to a focused Home/Menu/Back/Exit panel; hold **OK** again to return to direct control.
- **Numbers page**: use the conventional 3x4 channel keypad (`1-9`, dash, `0`, Enter). Move with the directional pad and press **OK** to transmit the highlighted key.
- **Hold OK on any non-Navigation remote page**: open Remote Information, Rename, Favorite, All Commands, or Remove.
- **Back**: return without transmitting.

Pages are shown only when the selected IR profile contains commands for them. For example, the bundled **Insignia NS 40D510NA17** profile includes digits and displays the Numbers page. A smaller profile without digit signals correctly omits that page. A generic command named `Enter` is used as Navigation OK/Select and also fills the keypad Enter position when no separate numeric-enter signal exists.

`TX Sent` means the Flipper transmitted the stored signal. Infrared is normally one-way, so the app cannot claim that the television received or acted on it.

## Real signal behavior

- Parsed signals retain their exact protocol, address, and command.
- RAW signals retain their exact carrier frequency, duty cycle, and timing list.
- Alias normalization affects only where a command appears in the UI.
- Unknown and manufacturer-specific commands remain available under **Extra** and **All Commands**.
- The number page is always arranged 1–9, then 0 and dash, independent of source-file order.
- Missing actions create no dead or guessed buttons.
- Real discrete HDMI, AV, Component, and TV input commands appear on the Inputs page when present.
- Enter and Previous/Last appear with the numeric keypad when present; they remain backed by the original stored signals.
- Remote Information preserves the bundled manufacturer, remote family, and database source. It explicitly reports when an upstream TV model was not provided.

## Build

With uFBT configured for firmware 1.4.3/API 87.1:

```powershell
Set-Location "C:\Users\frost\OneDrive\Desktop\flipper-applications\tv-remote"
python -m ufbt build
```

The FAP is written to `.ufbt\build\tv_remote_fz.fap` (the resolved global uFBT build directory may be under the user profile).

For this project, the resolved release artifact is normally:

```text
C:\Users\frost\.ufbt\build\tv_remote_fz.fap
```

Attach that file to the matching GitHub release; do not attach the debug ELF as the installable application.

## Limitations

- The standard `.ir` format does not contain formal brand, TV model, remote family, or database-source fields. The information screen reports those as unavailable rather than guessing them.
- Universal-remote setup numbers such as `7838` are not standardized IR commands. The app can include them only when the corresponding signals are available as a valid Flipper `.ir` profile or can be learned from the original remote.
- A remote can contain up to 224 commands in this bounded port; a larger file is rejected rather than partially presented as complete.
- TV control is limited to the Flipper's infrared hardware and the signals in the chosen file.

See [TESTING.md](TESTING.md), [SECURITY.md](SECURITY.md), and [FEATURE_MATRIX.md](FEATURE_MATRIX.md).
