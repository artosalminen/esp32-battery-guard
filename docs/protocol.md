# BM6 / intAct Battery Guard BLE protocol

Sources: [Waldmensch1/BatteryGuard](https://github.com/Waldmensch1/BatteryGuard)
and [tarball.ca: Reverse engineering the BM6](https://www.tarball.ca/posts/reverse-engineering-the-bm6-ble-battery-monitor/).

## GATT

| UUID | Use |
|---|---|
| `FFF0` | service |
| `FFF3` | write: commands |
| `FFF4` | notify: data, about 1 Hz after the handshake |

## Encryption

Every frame is 16 bytes, AES-128-CBC, zero IV, with a static key shared by
all BM6-family devices:

```
6c 65 61 67 65 6e 64 ff fe 30 31 30 30 30 30 39   ("leagend" FF FE "0100009")
```

(The BM2 / Ancel BM300 family uses a different key.)

## Handshake (plaintext before encryption)

BatteryGuard's sequence for lead acid / AGM / lithium (`TT` = battery
type: 01 lead acid, 02 AGM, 05 lithium):

```
1  D1 55 01 00 00 00 00 00 00 00 00 00 00 00 00 00   session init
2  D1 55 08 TT 00 00 00 00 00 00 00 00 00 00 00 00   battery type
3  D1 55 05 00 00 00 00 00 1E 00 00 00 00 00 00 00   config
4  D1 55 05 00 00 00 00 CA 94 00 00 00 00 00 00 00   config
5  D1 55 03 00 00 00 00 00 00 00 00 00 00 00 00 00   pre-finalize
6  D1 55 07 00 00 00 00 00 00 00 00 00 00 00 00 00   start data stream
```

tarball.ca gets readings with write 6 alone. If the full sequence ever
causes trouble, that's the minimal fallback.

## Data frame (decrypted)

| Byte | Field | Notes |
|---|---|---|
| 0–2 | `D1 55 07` | header |
| 3 | temp sign | `01` = negative |
| 4 | temperature | °C |
| 5 | status | `02` = charging (above about 13.3 V). Not charging: BatteryGuard saw `01`; the author's unit sends `00`. Treat anything but `02` as not charging |
| 6 | SOC | % |
| 7–8 | voltage | big-endian, /100 = V |
| 9–10 | rapid voltage rise count | big-endian |
| 11–12 | rapid voltage drop count | big-endian |
| 13–15 | unused | |

BatteryGuard drops the first 5 frames after the handshake because they
carry bogus values (e.g. 61 °C). This port does the same.
