#include <string>
#include <string.h>
#include <sstream>
#include "LineSimpleBeacon.h"


LineSimpleBeacon::LineSimpleBeacon(const char hwid[5]) {
  rawData.header_flags[0]        = 0x02; // Length + AD_TYPE(flags)
  rawData.header_flags[1]        = 0x01; // Length + AD_TYPE(flags)
  rawData.ad_type_flags          = 0x06; // flags: LE General Discoverable Mode(2) + BR/EDR Not Supported(4)
  rawData.header_manufacturer[0] = 0x03; // Length + AD_TYPE(Complete list of 16-bit UUIDs available)
  rawData.header_manufacturer[1] = 0x03; // Length + AD_TYPE(Complete list of 16-bit UUIDs available)
  rawData.manufacturer_id_1[0]   = 0x6f; // LINE Corp(0xFE6F)
  rawData.manufacturer_id_1[1]   = 0xfe; // LINE Corp(0xFE6F)
  rawData.service_data_length    = 0x0b; // Data length: 0x0A(0byte)~0x17(13bytes)
  rawData.ad_type_service_data   = 0x16; // AD_TYPE(Service Data)
  rawData.manufacturer_id_2[0]   = 0x6f; // LINE Corp(0xFE6F)
  rawData.manufacturer_id_2[1]   = 0xfe; // LINE Corp(0xFE6F)
  rawData.line_beacon_frame_id   = 0x02; // LINE Beacon Frame(0x02)  
  memcpy(&rawData.hardware_id[0], hwid, 5); // Line Simple Beacon Hardware ID
  rawData.tx_power               = 0x7f; // TX Signal power(fixed as 0x7f)
  memset(rawData.device_message, 0, 13); // Message(default=0x00)
}

std::string LineSimpleBeacon::getAdvPacket() {
  return std::string((char*)&rawData, sizeof(rawData));
}

void LineSimpleBeacon::setMessage(const char message[13], char mesSize) {
  memcpy(&rawData.device_message[0], &message[0], 13);
  rawData.service_data_length = 0x0a + mesSize;
}
