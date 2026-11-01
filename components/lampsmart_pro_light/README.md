# Known issues

* Only tested with Marpou Ceiling CCT light (definitely doesn't support RGB lights currently, but that could be added in the future).

# How to try it

1. Create an ESPHome configuration that references the repo (using `external_components`)
2. Add a light entity to the configuration with the `lampsmart_pro_light` platform
3. Build the configuration and flash it to an ESP32 device (since BLE is used, ESP8266-based devices are a no-go)
4. Add the new ESPHome node to your Home Assistant instance
5. Use the newly exposed service (`esphome.<esphome-node-name>_pair`) to pair with your light (call the service withing 5 seconds of powering it with a switch.
6. Enjoy controlling your Marpou light with Home Assistant!

# Example configuration

```yaml
light:
  - platform: lampsmart_pro_light
    name: Kitchen Light
    duration: 100
    max_duration: 3000
    default_transition_length: 0s
    gamma_correct: 1.0
```

The light gives no acknowledgement, so each new state is advertised in the background (without blocking ESPHome) and repeated for `max_duration` milliseconds, or until a newer state replaces it. A state that needs more than one packet (ON plus brightness) rotates them every `duration` milliseconds. Commands are absolute, so repeating them is harmless.

The light applies its own brightness curve, so `gamma_correct: 1.0` sends Home Assistant's brightness linearly. With ESPHome's default of 2.8, everything below about 28% maps to the same raw value.

The pair and unpair services need `custom_services: true` in the `api:` section.

# Potentially fixable issues

If this component works, but the cold and warm temperatures are reversed (that is, setting the temperature in Home Assistant to warm results in cold/blue light, and setting it to cold results in warm/yellow light), add a `reversed: true` line to your `lightsmart_pro_light` config.

If the minimum brightness is too bright, and you know that your light can go darker - try changing the minimum brightness via the `min_brightness` configuration option (it takes a number between 1 and 255).

If the light sometimes misses commands, raise `max_duration` (how long each state is repeated) or `duration` (how long each packet is advertised before the next state may replace it).
