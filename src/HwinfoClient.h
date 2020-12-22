#ifndef HWINFOCLIENT_H
#define HWINFOCLIENT_H

#include <Arduino.h>
#include <ESPAsyncTCP.h>

namespace HwinfoClientLib {

typedef std::function<void(uint32_t, uint32_t, double)> HwinfoSensorReadCompletedFunction;

template <typename T> 
union HwValue { 
  T value;
  uint8_t bytes[sizeof(T)]; 
}; 

struct HwSensor{
  HwValue<uint32_t> group;
  HwValue<uint32_t> id;
  HwValue<double> value;
};

class HwinfoClient
{
  private:
    const char* HWINFO_HOST;
    const uint16_t HWINFO_PORT = 27007;
    const uint8_t HWINFO_POLLING_RATE; // seconds
    const uint8_t HWINFO_CONNECTION_TIMEOUT; // seconds

    const static uint16_t HWINFO_OFFSET_POLLING_TIME = 24;
    const static uint16_t HWINFO_OFFSET_READINGS_OFFSET = 44;
    const static uint16_t HWINFO_OFFSET_READINGS_SIZE = 48;
    const static uint16_t HWINFO_OFFSET_READINGS = 52;

    const static uint16_t HWINFO_OFFSET_READING_GROUP = 16;
    const static uint16_t HWINFO_OFFSET_READING_ID = 20;
    const static uint16_t HWINFO_OFFSET_READING_VALUE = 296;

    HwValue<long> polling_time;
    HwValue<uint32_t> offset_of_readings;
    HwValue<uint32_t> size_of_reading;
    HwValue<uint32_t> number_of_readings;
    HwSensor reading;

    ulong magic_received_time;

    uint16_t idx = 0;

    HwinfoSensorReadCompletedFunction cb = nullptr;

    template <typename T> 
    bool valueParsed(HwValue<T>* hwValue, uint16_t offset, char* data, size_t len);

    void parse(char *data, size_t len);
    void onReadingCompleted();

    void sendHwinfoPacket(char* packet, uint8_t len);
    void sendHelloPacket();
    void sendRequestPacket();

    bool isHelloPacket(char* data);
    bool isDataStartPacket(char* data);
    bool isMagicPacket(char* data);

    AsyncClient* client;
    void onConnect();
    void onDisconnect();
    void onPoll();
    void handleData(char *data, size_t len);        
  public:
    void connect();
    void disconnect();
    uint32_t getNumberOfReadings();
    void setSensorReadHandler(HwinfoSensorReadCompletedFunction cb);

    HwinfoClient(const char* host, uint8_t polling_rate = 10, uint8_t timeout = 15);
    ~HwinfoClient();
};

} // End Of namespace HwinfoClientLib

typedef HwinfoClientLib::HwinfoClient HwinfoClient;

#endif