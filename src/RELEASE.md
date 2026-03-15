# Release Process

## Versioning

Releases use calendar-based versioning: `YY.MM.patch`

- **Major** — 2-digit year (e.g. `26`)
- **Minor** — month, no leading zero (e.g. `3` for March)
- **Patch** — sequential release within the month, starting at `1`

Version is defined in [src/v1/version.h](src/v1/version.h).

## Cutting a Release

1. **Update the version** in `src/v1/version.h` if needed (bump `FIRMWARE_VERSION_PATCH`, or major/minor for a new month/year).

2. **Commit any outstanding changes** — the release script requires a clean working tree.

3. **Run the release script** from the project root:
   ```
   ./scripts/release.sh
   ```
   The script will:
   - Build all four profiles cleanly (`make clean && make PROFILE=N`)
   - Copy the `.bin` files into `releases/<version>/`
   - Create a git commit and tag

4. **Review the output** in `releases/<version>/`:
   | File | Profile |
   |------|---------|
   | `TubeClock-<version>-us.bin` | 0 — US/North America (12h, °F, DST) |
   | `TubeClock-<version>-eu.bin` | 1 — European (24h, °C, no DST) |
   | `TubeClock-<version>-minimal.bin` | 2 — Minimal (silent, basic display) |
   | `TubeClock-<version>-kbx.bin` | 3 — kbx preferred |

5. **Push** the commit and tag:
   ```
   git push && git push --tags
   ```

## Flashing

Flash the appropriate `.bin` to the STM32F072 using your preferred SWD/JTAG tool. For example:

```
st-flash write releases/<version>/TubeClock-<version>-us.bin 0x08000000
```
