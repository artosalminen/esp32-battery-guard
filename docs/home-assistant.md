# Home Assistant

## Adding the device

After the first flash, HA finds the board on its own: **Settings → Devices
& services → ESPHome → Configure**, then enter the API key from
`secrets.yaml`. The device is called **Car Battery**.

## Entities

| Entity | Notes |
|---|---|
| `sensor.car_battery_voltage` | every 15 min; `unknown` after two missed cycles (car away) |
| `sensor.car_battery_state_of_charge` | % as computed by the Battery Guard |
| `sensor.car_battery_temperature` | Battery Guard's own sensor, close to battery temperature |
| `binary_sensor.car_battery_charging` | on if the alternator was running (> ~13.3 V) at the last reading |
| `sensor.car_battery_battery_state` | good / ok / low / critical / charging, from resting voltage |
| `binary_sensor.car_battery_car_in_range` | on if the last read cycle got a reading: "car is at home" |
| `button.car_battery_read_now` | take a reading immediately instead of waiting for the next cycle |
| `switch.car_battery_battery_guard_connection` | off = stop polling altogether |
| diagnostics | RSSI, WiFi signal, voltage rise/drop event counters |

Entity IDs come from the device's friendly name. If the device is in an
area, Home Assistant also puts the area in front of entities created after
that, e.g. `binary_sensor.garage_car_battery_car_in_range`. So IDs can
differ from this table, even between entities of the same device. If a
card or automation says "Entity not found", look up the real IDs on the
**Car Battery** device page.

## Automations and dashboard

- `homeassistant/car_battery_low.yaml`: notify when the resting voltage
  stays under 12.2 V for 30 min. Edit the notify service, then paste it
  into the automation editor (Edit in YAML).
- `homeassistant/dashboard_card.yaml`: an entities card plus a 7-day
  voltage graph.

## Alternative: no extra hardware, via your Bluetooth proxies

If the car parks within range of one of your existing ESPHome Bluetooth
proxies, you can skip the board:

1. On that proxy, make sure connections are enabled:
   ```yaml
   bluetooth_proxy:
     active: true
   ```
2. HACS → Custom repositories → `https://github.com/rafciq/bm6`
   (Integration) → install → restart HA.
3. The Battery Guard should then be discovered under Devices & services.

Caveats: you get no display, forum reports on reliability are mixed, and
only one client can hold the Battery Guard. **Don't run this and the
ESP32 board at the same time**, or they'll fight over the connection.
