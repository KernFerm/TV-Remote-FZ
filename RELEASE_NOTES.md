# TV Remote FZ 1.1.1

## Fixed

- Flipper OK now sends a profile's generic `Enter` signal as TV OK/Select in direct Navigation mode.
- The same generic `Enter` signal fills the NUM keypad's Enter position when the profile has no separate numeric-enter command.
- Explicit `Num_enter`, `Channel_enter`, and `Digit_enter` names remain classified as numeric Enter.

## Interface improvements

- Purpose-built Quick, conventional 3x4 Numbers, Media, and Extra layouts.
- Direct physical D-pad control on the Navigation page.
- Geometry-aware focus movement, safe long-label truncation, and corrected page-indicator spacing.
- Signal-driven pages: profiles without digit commands do not display a nonfunctional Numbers page.

For the complete version history, see [CHANGELOG.md](CHANGELOG.md).

## Install from PowerShell or Command Prompt

Connect the Flipper Zero by USB and close qFlipper before running either command. These commands require uFBT.

PowerShell, assuming the release file is in Downloads:

```powershell
python "$env:USERPROFILE\.ufbt\current\scripts\runfap.py" -p auto -s "$env:USERPROFILE\Downloads\tv_remote_fz.fap" -t /ext/apps/Infrared/tv_remote_fz.fap
```

Command Prompt:

```bat
python "%USERPROFILE%\.ufbt\current\scripts\runfap.py" -p auto -s "%USERPROFILE%\Downloads\tv_remote_fz.fap" -t /ext/apps/Infrared/tv_remote_fz.fap
```

The command installs the app at `/ext/apps/Infrared/tv_remote_fz.fap` and launches it. Change the path after `-s` if the downloaded FAP is stored elsewhere.
