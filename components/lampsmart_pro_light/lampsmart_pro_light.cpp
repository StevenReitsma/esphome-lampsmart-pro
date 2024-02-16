#include "lampsmart_pro_light.h"
#include "esphome/core/log.h"

#ifdef USE_ESP32

#include <esp_gap_ble_api.h>
#include <esp_gatts_api.h>

namespace esphome
{
  namespace lampsmartpro
  {
    static const char *TAG = "lampsmartpro";

    static const char PACKET_BASE[32] = {
        0x1F, 0x02, 0x01, 0x01, 0x1B, 0x03, 0x71, 0x0F,
        0x55, 0xAA, 0x98, 0x43, 0xAF, 0x0B, 0x46, 0x46,
        0x46, 0x00, 0x00, 0x00, 0x00, 0x00, 0x83, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

    static const int CRC_TABLE[256] = {
        0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50A5, 0x60C6, 0x70E7,
        0x8108, 0x9129, 0xA14A, 0xB16B, 0xC18C, 0xD1AD, 0xE1CE, 0xF1EF,
        0x1231, 0x0210, 0x3273, 0x2252, 0x52B5, 0x4294, 0x72F7, 0x62D6,
        0x9339, 0x8318, 0xB37B, 0xA35A, 0xD3BD, 0xC39C, 0xF3FF, 0xE3DE,
        0x2462, 0x3443, 0x0420, 0x1401, 0x64E6, 0x74C7, 0x44A4, 0x5485,
        0xA56A, 0xB54B, 0x8528, 0x9509, 0xE5EE, 0xF5CF, 0xC5AC, 0xD58D,
        0x3653, 0x2672, 0x1611, 0x0630, 0x76D7, 0x66F6, 0x5695, 0x46B4,
        0xB75B, 0xA77A, 0x9719, 0x8738, 0xF7DF, 0xE7FE, 0xD79D, 0xC7BC,
        0x48C4, 0x58E5, 0x6886, 0x78A7, 0x0840, 0x1861, 0x2802, 0x3823,
        0xC9CC, 0xD9ED, 0xE98E, 0xF9AF, 0x8948, 0x9969, 0xA90A, 0xB92B,
        0x5AF5, 0x4AD4, 0x7AB7, 0x6A96, 0x1A71, 0x0A50, 0x3A33, 0x2A12,
        0xDBFD, 0xCBDC, 0xFBBF, 0xEB9E, 0x9B79, 0x8B58, 0xBB3B, 0xAB1A,
        0x6CA6, 0x7C87, 0x4CE4, 0x5CC5, 0x2C22, 0x3C03, 0x0C60, 0x1C41,
        0xEDAE, 0xFD8F, 0xCDEC, 0xDDCD, 0xAD2A, 0xBD0B, 0x8D68, 0x9D49,
        0x7E97, 0x6EB6, 0x5ED5, 0x4EF4, 0x3E13, 0x2E32, 0x1E51, 0x0E70,
        0xFF9F, 0xEFBE, 0xDFDD, 0xCFFC, 0xBF1B, 0xAF3A, 0x9F59, 0x8F78,
        0x9188, 0x81A9, 0xB1CA, 0xA1EB, 0xD10C, 0xC12D, 0xF14E, 0xE16F,
        0x1080, 0x00A1, 0x30C2, 0x20E3, 0x5004, 0x4025, 0x7046, 0x6067,
        0x83B9, 0x9398, 0xA3FB, 0xB3DA, 0xC33D, 0xD31C, 0xE37F, 0xF35E,
        0x02B1, 0x1290, 0x22F3, 0x32D2, 0x4235, 0x5214, 0x6277, 0x7256,
        0xB5EA, 0xA5CB, 0x95A8, 0x8589, 0xF56E, 0xE54F, 0xD52C, 0xC50D,
        0x34E2, 0x24C3, 0x14A0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
        0xA7DB, 0xB7FA, 0x8799, 0x97B8, 0xE75F, 0xF77E, 0xC71D, 0xD73C,
        0x26D3, 0x36F2, 0x0691, 0x16B0, 0x6657, 0x7676, 0x4615, 0x5634,
        0xD94C, 0xC96D, 0xF90E, 0xE92F, 0x99C8, 0x89E9, 0xB98A, 0xA9AB,
        0x5844, 0x4865, 0x7806, 0x6827, 0x18C0, 0x08E1, 0x3882, 0x28A3,
        0xCB7D, 0xDB5C, 0xEB3F, 0xFB1E, 0x8BF9, 0x9BD8, 0xABBB, 0xBB9A,
        0x4A75, 0x5A54, 0x6A37, 0x7A16, 0x0AF1, 0x1AD0, 0x2AB3, 0x3A92,
        0xFD2E, 0xED0F, 0xDD6C, 0xCD4D, 0xBDAA, 0xAD8B, 0x9DE8, 0x8DC9,
        0x7C26, 0x6C07, 0x5C64, 0x4C45, 0x3CA2, 0x2C83, 0x1CE0, 0x0CC1,
        0xEF1F, 0xFF3E, 0xCF5D, 0xDF7C, 0xAF9B, 0xBFBA, 0x8FD9, 0x9FF8,
        0x6E17, 0x7E36, 0x4E55, 0x5E74, 0x2E93, 0x3EB2, 0x0ED1, 0x1EF0};

    char *bitReverse(char *bArr)
    {
      static char revArr[25];
      for (int i = 0; i < 25; i++)
      {
        char rev = 0;
        for (int j = 0; j < 8; j++)
        {
          rev += (((bArr[i] & 255) >> (7 - j)) & 1) << j;
        }
        revArr[i] = rev;
      }
      return revArr;
    }

    char *bleWhitening(char *bArr)
    {
      static char whArr[38];
      int i2 = 83;
      int i3 = 0;
      while (i3 < 38)
      {
        int i4 = i2;
        char b = 0;
        for (int i5 = 0; i5 < 8; i5++)
        {
          int i6 = i4 & 255;
          b |= ((((i6 & 64) >> 6) << i5) ^ (bArr[i3] & 255)) & (1 << i5);
          int i7 = i6 << 1;
          int i8 = (i7 >> 7) & 1;
          int i9 = (i7 & -2) | i8;
          i4 = ((i9 ^ (i8 << 4)) & 16) | (i9 & -17);
        }
        whArr[i3] = b;
        i2 = i4;
        i3++;
      }
      return whArr;
    }

    char *bleWhiteningForPacket(char *bArr)
    {
      char whArr[38];
      for (int i = 0; i < 25; i++)
      {
        whArr[i + 13] = bArr[i];
      }
      static char whitenedForPacket[25];
      char *bleWhitened = bleWhitening(whArr);
      for (int i = 0; i < 25; i++)
      {
        whitenedForPacket[i] = bleWhitened[i + 13];
      }
      return whitenedForPacket;
    }

    int CRC16(char *bArr, int len, int offset)
    {
      int crc = 65535;
      for (int i = 0; i < len; i++)
      {
        crc = CRC_TABLE[((crc >> 8) ^ bArr[offset + i]) & 255] ^ (crc << 8);
      }
      return crc;
    }

    char *buildPacket(char command, char mControl0, char mControl1, char arg1, char arg2)
    {
      char msgBase[25];
      static char packet[32];
      for (int i = 0; i < 25; i++)
      {
        msgBase[i] = PACKET_BASE[i + 6];
      }
      msgBase[11] = command;
      msgBase[12] = mControl0;
      msgBase[13] = mControl1;
      msgBase[14] = arg1;
      msgBase[15] = arg2;
      msgBase[17] = rand() & 255;
      int crc = CRC16(msgBase, 12, 11);
      msgBase[23] = (crc >> 8) & 255;
      msgBase[24] = crc & 255;
      char *msgRev = bitReverse(msgBase);
      char *msgWht = bleWhiteningForPacket(msgRev);
      for (int i = 0; i < 6; i++)
      {
        packet[i] = PACKET_BASE[i];
      }
      for (int i = 0; i < 25; i++)
      {
        packet[i + 6] = msgWht[i];
      }
      packet[31] = PACKET_BASE[31];

      return packet;
    }

    static esp_ble_adv_params_t ADVERTISING_PARAMS = {
        .adv_int_min = 0x20,
        .adv_int_max = 0x20,
        .adv_type = ADV_TYPE_NONCONN_IND,
        .own_addr_type = BLE_ADDR_TYPE_PUBLIC,
        .peer_addr =
            {
                0x00,
            },
        .peer_addr_type = BLE_ADDR_TYPE_PUBLIC,
        .channel_map = ADV_CHNL_ALL,
        .adv_filter_policy = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
    };

    void LampSmartProLight::setup()
    {
      register_service(&LampSmartProLight::on_pair, light_state_ ? "pair_" + light_state_->get_object_id() : "pair");
      register_service(&LampSmartProLight::on_unpair, light_state_ ? "unpair_" + light_state_->get_object_id() : "unpair");
    }

    light::LightTraits LampSmartProLight::get_traits()
    {
      auto traits = light::LightTraits();
      traits.set_supported_color_modes({light::ColorMode::COLD_WARM_WHITE});
      traits.set_min_mireds(this->cold_white_temperature_);
      traits.set_max_mireds(this->warm_white_temperature_);
      return traits;
    }

    char *LampSmartProLight::getHostDeviceIdentifier()
    {
      static char masterControl[2];
      uint32_t hash = light_state_ ? light_state_->get_object_id_hash() : 0xcafebabe;
      int crc = CRC16((char *)&hash, 4, 0);

      masterControl[0] = (crc >> 8) & 255;
      masterControl[1] = crc & 255;
      return masterControl;
    }

    void LampSmartProLight::write_state(light::LightState *state)
    {
      float cwf, wwf;
      state->current_values_as_cwww(&cwf, &wwf, this->constant_brightness_);

      if (!cwf && !wwf)
      {
        send_packet(CMD_TURN_OFF, 0, 0);
        _is_off = true;

        return;
      }

      uint8_t cwi = (uint8_t)(0xff * cwf);
      uint8_t wwi = (uint8_t)(0xff * wwf);

      if ((cwi < min_brightness_) && (wwi < min_brightness_))
      {
        if (cwf > 0.000001)
        {
          cwi = min_brightness_;
        }

        if (wwf > 0.000001)
        {
          wwi = min_brightness_;
        }
      }

      ESP_LOGD(TAG, "LampSmartProLight::write_state called! Requested cw: %d, ww: %d", cwi, wwi);

      if (_is_off)
      {
        send_packet(CMD_TURN_ON, 0, 0);
        _is_off = false;
      }

      send_packet(CMD_DIM, cwi, wwi);
    }

    void LampSmartProLight::dump_config()
    {
      ESP_LOGCONFIG(TAG, "LampSmartProLight '%s'", light_state_ ? light_state_->get_name().c_str() : "");
      ESP_LOGCONFIG(TAG, "  Cold White Temperature: %f mireds", cold_white_temperature_);
      ESP_LOGCONFIG(TAG, "  Warm White Temperature: %f mireds", warm_white_temperature_);
      ESP_LOGCONFIG(TAG, "  Constant Brightness: %s", constant_brightness_ ? "true" : "false");
      ESP_LOGCONFIG(TAG, "  Minimum Brightness: %d", min_brightness_);
      ESP_LOGCONFIG(TAG, "  Transmission Duration: %d millis", tx_duration_);
    }

    void LampSmartProLight::on_pair()
    {
      ESP_LOGD(TAG, "LampSmartProLight::on_pair called!");

      char *hostId = getHostDeviceIdentifier();
      send_packet(CMD_PAIR, hostId[0], hostId[1]);
    }

    void LampSmartProLight::on_unpair()
    {
      ESP_LOGD(TAG, "LampSmartProLight::on_unpair called!");

      char *hostId = getHostDeviceIdentifier();
      send_packet(CMD_UNPAIR, hostId[0], hostId[1]);
    }

    void LampSmartProLight::send_packet(uint16_t cmd, uint8_t arg1, uint8_t arg2)
    {
      char *hostId = getHostDeviceIdentifier();
      uint8_t *packet = (uint8_t *)buildPacket(cmd, hostId[0], hostId[1], arg1, arg2);

      // Skip first byte (BLE packet size indicator)
      ESP_ERROR_CHECK_WITHOUT_ABORT(esp_ble_gap_config_adv_data_raw(&packet[1], 31));
      ESP_ERROR_CHECK_WITHOUT_ABORT(esp_ble_gap_start_advertising(&ADVERTISING_PARAMS));
      delay(tx_duration_);
      ESP_ERROR_CHECK_WITHOUT_ABORT(esp_ble_gap_stop_advertising());
    }

  } // namespace lampsmartpro
} // namespace esphome

#endif
