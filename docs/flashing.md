# Building and flashing

Only the first flash needs USB. After that, updates go over WiFi (OTA).
Nothing has to be installed on the Mac: use either the ESPHome add-on in
Home Assistant, or throwaway Docker containers plus a browser.

## Secrets

Copy `esphome/secrets.yaml.example` to `esphome/secrets.yaml` and fill it
in. If you use the ESPHome add-on, add the two `battery_guard_*` keys to
its existing secrets instead.

## Option 1: ESPHome add-on in Home Assistant

1. ESPHome dashboard → **New Device** → name it `battery-guard` → skip the
   board wizard, then **Edit** and replace the YAML with
   `esphome/battery-guard.yaml`.
2. Copy `esphome/bm6.h` into the add-on's config folder (`/config/esphome/`,
   next to the YAML), e.g. with the File editor or Samba add-on. The YAML
   `includes:` it.
3. **Install** → *Plug into this computer* (Chrome/Edge, WebSerial) for the
   first flash, then *Wirelessly* afterwards.

## Option 2: Docker on the Mac (nothing installed permanently)

Docker on macOS can't pass USB serial ports through to a container, so the
container builds the firmware and the browser flashes it.

```sh
cd esp32-battery-guard

# Build. Its cache goes in esphome/.esphome (gitignored); delete it to clean up.
docker run --rm -v "$PWD/esphome:/config" ghcr.io/esphome/esphome \
  compile battery-guard.yaml
```

First flash: open <https://web.esphome.io> in Chrome or Edge, plug the
board in over USB-C, **Connect** → **Install** → pick
`esphome/.esphome/build/battery-guard/build/firmware.factory.bin`.

> If the board isn't detected, hold BOOT while plugging it in to force
> download mode.

Later updates over WiFi. mDNS `.local` names don't resolve inside the
container, so use the board's IP (shown in your router or in HA under the
device):

```sh
docker run --rm -v "$PWD/esphome:/config" ghcr.io/esphome/esphome \
  run battery-guard.yaml --device 192.168.x.y --no-logs

# Live logs
docker run --rm -it -v "$PWD/esphome:/config" ghcr.io/esphome/esphome \
  logs battery-guard.yaml --device 192.168.x.y
```

When you're done with the project for a while:

```sh
docker rmi ghcr.io/esphome/esphome     # remove the image (~1.5 GB)
rm -rf esphome/.esphome                # remove the build cache
```

## First-run checks

With the intAct phone app **closed**, watch the logs. A working run looks
like:

```
[I][bm6]: Connected, sending handshake
[D][bm6]: 12.64 V | 87 % | 8 °C | status 1 | rise 3 | drop 5
```

| Symptom | Likely cause |
|---|---|
| Never connects | Out of range (check `Battery Guard RSSI`), wrong MAC, or the phone app is holding the connection |
| Only `Skipping non-data notification` lines, never a reading (logger at DEBUG) | Wrong AES key, i.e. a BM2-family device rather than BM6 |
| Connects, then drops right after the handshake | Try `battery_type` matching the app; or as a fallback send only handshake write 6 (see `protocol.md`) |
| Display blank | Different I2C pins on your board batch (see `hardware.md`) |
