# Hardware

## Parts

- **ESP32-C3 dev board with built-in 0.42" OLED**: [AYWHP, Amazon.de B0FWK7N67Y](https://www.amazon.de/dp/B0FWK7N67Y).
  The same board design is sold under several brands (diymore, 01Space and
  others): an SSD1306 OLED with a 72x40 visible area on I2C (SDA = GPIO5,
  SCL = GPIO6, address 0x3C), a BOOT button on GPIO9, and 4 MB flash.
- **[intAct Battery Guard](https://www.amazon.de/dp/B00ZETXNAQ)** on the car battery.
  Other BM6 clones should work too.
- A USB-C power supply near where the car parks

No wiring needed: the display is on the board.

> If the display stays blank, check the listing images or silkscreen for
> your board's I2C pins and change `i2c:` in the YAML to match.

## Placement

BLE range through a car body and a garage wall is short, so put the board
as close to the car's engine bay as you can: on the garage wall in front
of the car, or on a window sill facing the driveway.

After flashing, the `Battery Guard RSSI` sensor in HA helps you pick a
spot:

| RSSI | Link |
|---|---|
| better than -75 dBm | solid |
| -75 to -88 dBm | usually works, occasional reconnects |
| worse than -90 dBm | move the board |

Both radios run at full TX power: WiFi at 20.5 dB and BLE at +20 dBm
(the ESP-IDF default is +9 dBm). Some units of this board design have a
poorly matched antenna and fail to join WiFi at full power ("Auth Expired"
in the log). In that case lower `output_power`, e.g. to 8.5dB. Check `WiFi
Signal` as well.

## Display

```
 87%          4m       <- SOC            age of the reading ("now" while connected)
    12.64V             <- voltage (inverted = charging / engine running)
 [██████████░░░]       <- SOC bar
```

With no fresh data it shows `WiFi ● HA ●` connection dots and a blinking
`no car` (not connected to the Battery Guard) or `reading..` (connected,
waiting for the first valid frame).

## BOOT button

| Press | Action |
|---|---|
| short | take a reading now (same as `Read Now` in HA); the display shows `now` while connected |
| long (≥ 1 s) | details screen for 5 s (below) |

```
 8°C       BT-85       <- temperature       Battery Guard RSSI (last cycle)
 rise 0                <- rapid voltage rise events (engine starts)
 drop 0                <- rapid voltage drop events (cranking, heavy loads)
 WiFi -62dB
```

The **RST** button restarts the board. Holding BOOT while pressing RST, or
while plugging in, puts it into USB flashing mode.

## Battery drain

The Battery Guard runs from the car battery around the clock either way:
it keeps measuring, logging and advertising, with a vendor-spec draw of
about 1.5 mA, or roughly 1.1 Ah a month. That's small next to a modern car's own
20–50 mA standby draw. To add as little as possible on top, the board
doesn't hold a connection. It connects for about 3 seconds every 15
minutes, takes one reading, and disconnects. The board itself runs on
mains USB, not the car.
