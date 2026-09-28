# Security and robustness

- Only files with header `IR signals file` and version `1` are accepted.
- RAW timing arrays are restricted to the official `MAX_TIMINGS_AMOUNT` (1,024) and allocation failure rejects the signal.
- Carrier frequency and duty cycle are validated before transmission.
- Parsed protocol, address width, and command width are validated against the official infrared API.
- Command count, names, paths, friendly names, and settings-file size are bounded.
- The app never executes content from an `.ir` file and never invokes a shell.
- Saved-TV configuration uses a synchronized temporary file and backup/restore promotion.
- A failed import is not saved and a failed signal reload is not transmitted.
- Held transmission stops on release, Back, page change, view change, and application shutdown.

Report security issues privately through the repository's GitHub Security page. Do not include private device data or proprietary IR recordings in a public report.
