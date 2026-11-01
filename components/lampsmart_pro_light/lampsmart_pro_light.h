#pragma once

#include "esphome.h"
#include "esphome/components/api/custom_api_device.h"
#include "esphome/components/light/light_output.h"

#include <array>
#include <vector>

#define CMD_PAIR (0x28)
#define CMD_UNPAIR (0x45)
#define CMD_TURN_ON (0x10)
#define CMD_TURN_OFF (0x11)
#define CMD_DIM (0x21)

namespace esphome
{
  namespace lampsmartpro
  {

    class LampSmartProLight : public light::LightOutput, public Component, public EntityBase, public api::CustomAPIDevice
    {
    public:
      void setup() override;
      void loop() override;
      void dump_config() override;

      void set_cold_white_temperature(float cold_white_temperature) { cold_white_temperature_ = cold_white_temperature; }
      void set_warm_white_temperature(float warm_white_temperature) { warm_white_temperature_ = warm_white_temperature; }
      void set_constant_brightness(bool constant_brightness) { constant_brightness_ = constant_brightness; }
      void set_reversed(bool reversed) { reversed_ = reversed; }
      void set_min_brightness(uint8_t min_brightness) { min_brightness_ = min_brightness; }
      void set_tx_duration(uint32_t tx_duration) { tx_duration_ = tx_duration; }
      void set_max_tx_duration(uint32_t max_tx_duration) { max_tx_duration_ = max_tx_duration; }
      void setup_state(light::LightState *state) override { this->light_state_ = state; }
      void write_state(light::LightState *state) override;
      light::LightTraits get_traits() override;

    protected:
      // Advertising payload without the leading length byte.
      using Packet = std::array<uint8_t, 31>;

      void on_pair();
      void on_unpair();
      Packet build_packet(uint16_t cmd, uint8_t arg1, uint8_t arg2);
      void send_packets(std::vector<Packet> packets);
      void advertise(Packet &packet);
      char *getHostDeviceIdentifier();

      float cold_white_temperature_{167};
      float warm_white_temperature_{333};
      bool constant_brightness_;
      bool reversed_;
      uint8_t min_brightness_;
      // Unknown at boot: send ON with the first non-off state.
      bool is_off_{true};
      uint32_t turned_on_at_{0};
      uint32_t tx_duration_;
      uint32_t max_tx_duration_;
      light::LightState *light_state_;

      // The lamp gives no acknowledgement, so the latest state is re-advertised for
      // max_tx_duration_, rotating between its packets every tx_duration_.
      std::vector<Packet> packets_;
      std::vector<Packet> next_packets_;
      size_t packet_index_{0};
      uint32_t packets_started_at_{0};
      uint32_t packet_started_at_{0};
      bool advertising_{false};
      uint16_t last_cmd_{0};
      uint8_t last_cold_{0};
      uint8_t last_warm_{0};
    };

  } // namespace lampsmartpro
} // namespace esphome
