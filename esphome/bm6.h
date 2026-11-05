// BM6 / intAct Battery Guard BLE protocol helpers.
//
// Protocol from https://github.com/Waldmensch1/BatteryGuard (MIT) and
// https://www.tarball.ca/posts/reverse-engineering-the-bm6-ble-battery-monitor/
//
// Every frame on the wire is 16 bytes, AES-128-CBC with a zero IV and a
// static key that is the same for every BM6-family device (it's baked into
// the phone app). Commands go to characteristic FFF3, readings arrive as
// notifications on FFF4 about once a second.
#pragma once

#include <cstring>
#include <vector>
#include "mbedtls/aes.h"

namespace bm6 {

// "leagend" 0xFF 0xFE "0100009" — the BM6 / intAct Battery Guard key.
// (The BM2 / Ancel BM300 family uses a different key and isn't covered here.)
static const uint8_t KEY[16] = {0x6c, 0x65, 0x61, 0x67, 0x65, 0x6e, 0x64, 0xff,
                                0xfe, 0x30, 0x31, 0x30, 0x30, 0x30, 0x30, 0x39};

inline void crypt(int mode, const uint8_t *in, uint8_t *out) {
  uint8_t iv[16] = {0};  // CBC overwrites the IV, so always start fresh
  mbedtls_aes_context aes;
  mbedtls_aes_init(&aes);
  if (mode == MBEDTLS_AES_ENCRYPT)
    mbedtls_aes_setkey_enc(&aes, KEY, 128);
  else
    mbedtls_aes_setkey_dec(&aes, KEY, 128);
  mbedtls_aes_crypt_cbc(&aes, mode, 16, iv, in, out);
  mbedtls_aes_free(&aes);
}

// Handshake write #n (0..5), already encrypted, ready for ble_write.
// Same 6-write sequence BatteryGuard uses for LEAD_ACID / AGM / LITHIUM.
// battery_type: 1 = lead acid, 2 = AGM, 5 = lithium.
inline std::vector<uint8_t> handshake(int n, uint8_t battery_type) {
  uint8_t cmd[16] = {0xD1, 0x55};
  switch (n) {
    case 0: cmd[2] = 0x01; break;                                   // session init
    case 1: cmd[2] = 0x08; cmd[3] = battery_type; break;            // battery type
    case 2: cmd[2] = 0x05; cmd[8] = 0x1E; break;                    // config
    case 3: cmd[2] = 0x05; cmd[7] = 0xCA; cmd[8] = 0x94; break;     // config
    case 4: cmd[2] = 0x03; break;                                   // pre-finalize
    default: cmd[2] = 0x07; break;                                  // start streaming
  }
  std::vector<uint8_t> out(16);
  crypt(MBEDTLS_AES_ENCRYPT, cmd, out.data());
  return out;
}

struct Reading {
  float voltage;       // V
  int temperature;     // °C
  uint8_t status;      // 2 = charging (>~13.3 V); not charging is 0 or 1 depending on unit
  uint8_t soc;         // %
  uint16_t vrise;      // rapid voltage rise events (alternator kicking in)
  uint16_t vdrop;      // rapid voltage drop events (cranking, heavy loads)
};

// Decrypts and parses one FFF4 notification. Returns false for anything
// that isn't a D1 55 07 data frame.
inline bool parse(const std::vector<uint8_t> &x, Reading &r) {
  if (x.size() != 16) return false;
  uint8_t d[16];
  crypt(MBEDTLS_AES_DECRYPT, x.data(), d);
  if (d[0] != 0xD1 || d[1] != 0x55 || d[2] != 0x07) return false;
  r.temperature = d[3] == 1 ? -(int) d[4] : (int) d[4];
  r.status = d[5];
  r.soc = d[6];
  r.voltage = ((d[7] << 8) | d[8]) / 100.0f;
  r.vrise = (d[9] << 8) | d[10];
  r.vdrop = (d[11] << 8) | d[12];
  return true;
}

}  // namespace bm6
