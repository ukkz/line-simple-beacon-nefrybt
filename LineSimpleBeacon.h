class LineSimpleBeacon {
private:
  struct {
    char header_flags[2];
    char ad_type_flags;
    char header_manufacturer[2];
    char manufacturer_id_1[2];
    char service_data_length;
    char ad_type_service_data;
    char manufacturer_id_2[2];
    char line_beacon_frame_id;
    char hardware_id[5];
    char tx_power;
    char device_message[13];
  } __attribute__((packed)) rawData;
public:
  LineSimpleBeacon(const char hwid[5]);
  std::string getAdvPacket();
  void setMessage(const char message[13], char mesSize);
};
