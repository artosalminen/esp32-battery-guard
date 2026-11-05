# Contributing

Issues and pull requests are welcome, especially from people running this
with hardware other than the original setup.

## Reporting a device

If you tried another BM6-family monitor, or another ESP32 board, please
open an issue with:

- the monitor's brand and model, and whether readings came through
- the ESP32 board, and any pin or config changes you needed
- your ESPHome version
- for problems: the device log from connect to disconnect (ESPHome
  dashboard → Logs, or `esphome logs battery-guard.yaml`)

Remove your MAC address, WiFi details and keys from logs before posting.

## Changing the code

- **Test on real hardware.** At minimum, run one full read cycle: connect,
  a `bm6:` reading line in the log, then disconnect.
- **Validate the config** before opening a PR:
  ```sh
  docker run --rm -v "$PWD/esphome:/config" ghcr.io/esphome/esphome config battery-guard.yaml
  ```
  This needs a `secrets.yaml`. The example file with dummy values is
  enough for validation.
- **Keep the style:** comment the *why* in the YAML, match the existing
  section layout, and keep protocol code in `bm6.h`.
- **Update the docs** in the same PR when behaviour, entities or the
  display change.

## Never commit

- `esphome/secrets.yaml` (gitignored)
- `esphome/.esphome/` build output (gitignored)
- Your Battery Guard's MAC in any tracked file. It belongs in
  `secrets.yaml`.

## Protocol findings

New findings about the BM6 protocol (status byte values, other commands,
history download) are very welcome. Add them to `docs/protocol.md` with
how you observed them.
