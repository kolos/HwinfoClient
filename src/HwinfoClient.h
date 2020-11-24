#ifndef HWINFOCLIENT_H
#define HWINFOCLIENT_H

#include <Arduino.h>
#include <ESPAsyncTCP.h>

namespace HwinfoClientLib {

typedef std::function<void(uint, uint, double)> HwinfoSensorReadCompletedFunction;

template <typename T> 
union HwValue { 
  T value;
  uint8_t bytes[sizeof(T)]; 
}; 

struct HwSensor{
  HwValue<uint> group;
  HwValue<uint> id;
  HwValue<double> value;
};

class HwinfoClient
{
  private:
    const char* HWINFO_HOST;
    const uint16_t HWINFO_PORT = 27007;
    const uint8_t HWINFO_POLLING_RATE = 10; // seconds
    const uint8_t HWINFO_CONNECTION_TIMEOUT = 15; // seconds

    const static uint16_t HWINFO_OFFSET_POLLING_TIME = 24;
    const static uint16_t HWINFO_OFFSET_READINGS_OFFSET = 44;
    const static uint16_t HWINFO_OFFSET_READINGS_SIZE = 48;
    const static uint16_t HWINFO_OFFSET_READINGS = 52;

    const static uint16_t HWINFO_OFFSET_READING_GROUP = 16;
    const static uint16_t HWINFO_OFFSET_READING_ID = 20;
    const static uint16_t HWINFO_OFFSET_READING_VALUE = 296;

    HwValue<long> polling_time;
    HwValue<uint> offset_of_readings;
    HwValue<uint> size_of_reading;
    HwValue<uint> number_of_readings;
    HwSensor reading;

    ulong magic_received_time;

    uint16_t idx = 0;
    uint16_t reading_idx = 0;

    HwinfoSensorReadCompletedFunction cb = nullptr;

    template <typename T> 
    bool valueByteParsed(HwValue<T>* hwValue, uint16_t offset, char ch);

    void parse(char *data, size_t len);
    void onReadingCompleted();

    void sendHwinfoPacket(char* packet, uint8_t len);
    void sendHelloPacket();
    void sendRequestPacket();

    bool isHelloPacket(char* data);
    bool isMagicPacket(char* data);
    bool isMagic2Packet(char* data);

    AsyncClient* client;
    void onConnect();
    void onDisconnect();
    void onPoll();
    void handleData(char *data, size_t len);        
  public:
    void connect();
    void setSensorReadHandler(HwinfoSensorReadCompletedFunction cb);

    HwinfoClient(const char* host);
    ~HwinfoClient();
};

} // End Of namespace HwinfoClientLib

typedef HwinfoClientLib::HwinfoClient HwinfoClient;

#endif