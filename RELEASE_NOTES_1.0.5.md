# TV Remote FZ 1.0.5

This release improves both the built-in TV profile coverage and microSD persistence.

- The catalog now contains 3,207 verified TV profiles from the current official Flipper Devices IRDB plus unique valid profiles from the maintained community Flipper-IRDB.
- Insignia now has four selectable profiles, including an additional generic community profile.
- Selected built-in profiles are saved where they are easy to find: `/ext/infrared/TV_Remote_FZ`.
- App settings are stored at `/ext/apps_data/tv_remote_fz/tvs.cfg`.
- Profile writes use a temporary file and backup rollback so a failed replacement preserves the previous saved copy.
- Remote Information identifies whether a profile came from the official or community database.

The number `7838` shown by a phone dongle application is a vendor-specific setup-profile identifier, not a standardized infrared command. This release does not claim that the new generic Insignia profile corresponds to `7838`; matching that exact profile requires an exported signal definition or a learned recording from a working remote.
