# Included TV-profile database

- Primary project: Flipper Devices IRDB
- Repository: https://github.com/flipperdevices/irdb
- Revision: `f7b15366521cc81ba11b341538f0097bddbb748b`
- Included scope: `database/categories/TVs/**/*.ir`
- License: MIT; see `assets/IRDB-LICENSE-MIT.txt`
- Supplemental project: Flipper-IRDB
- Repository: https://github.com/Lucaslhm/Flipper-IRDB
- Revision: `d126fb1b6f1e114c52b4a8c19839ea65e3a9c24d`
- Included scope: unique profiles from `TVs/**/*.ir`
- License: CC0-1.0; see `assets/FLIPPER-IRDB-LICENSE-CC0.txt`
- Packed brands: 630
- Packed profiles: 3,207
- Uncompressed signal data: 23,412,923 bytes
- Packed SHA-256: `9e2588e0983b6531cde482263444058d80d6f974ad95618f63a421551a75db3a`

The official source is loaded first and wins duplicate brand/profile names. The community source contributes only profiles whose normalized profile name is not already present for that brand. Inputs missing the required Flipper IR header/version are excluded; revision `d126fb1` contains one such file (`TVs/Enseo/Enseo.ir`). The application records the source of each included profile and shows it in Remote Information.

The application code remains licensed under GNU GPL v3 or later. Separately attributed database assets retain their upstream licenses. Command aliases only classify records for display; imported IR records are not rewritten. Vendor setup identifiers used by phone dongle apps are not standardized signal definitions and are not treated as IR codes.
