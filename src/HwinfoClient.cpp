#include "HwinfoClient.h"

using namespace HwinfoClientLib;


void HwinfoClient::setSensorReadHandler(HwinfoSensorReadCompletedFunction sensorReadHandler) {
  cb = sensorReadHandler;
}

void HwinfoClient::onReadingCompleted() {
  if(cb != nullptr) {
    cb(reading.group.value, reading.id.value, reading.value.value);
  }
}

template<typename T>
bool HwinfoClient::valueParsed(HwValue<T> *hwValue, uint16_t offset, char* data, size_t len) {
  if(offset + sizeof(hwValue->value) > idx && offset < idx + len) {
    uint16_t start = max(idx, offset);
    uint16_t end = min(offset + sizeof(hwValue->value), idx + len);
    char* src = data + start - idx;
    uint8_t* dst = hwValue->bytes + max(0, idx - offset);
    memcpy(dst, src, end - start);

    return true;
  } 
  
  return false;
}

void HwinfoClient::parse(char *data, size_t len) {
  valueParsed(&polling_time, HWINFO_OFFSET_POLLING_TIME, data, len);
  valueParsed(&offset_of_readings, HWINFO_OFFSET_READINGS_OFFSET, data, len);
  valueParsed(&size_of_reading, HWINFO_OFFSET_READINGS_SIZE, data, len);
  valueParsed(&number_of_readings, HWINFO_OFFSET_READINGS, data, len);

  for(uint16_t reading_offset = offset_of_readings.value; reading_offset < idx + len; reading_offset += size_of_reading.value){
    valueParsed(&reading.group, HWINFO_OFFSET_READING_GROUP + reading_offset, data, len);
    valueParsed(&reading.id, HWINFO_OFFSET_READING_ID + reading_offset, data, len);
    bool reading_value_parsed = valueParsed(&reading.value, HWINFO_OFFSET_READING_VALUE + reading_offset, data, len);
    if(reading_value_parsed) {
      onReadingCompleted();
    }
  }

  idx += len;
}

void HwinfoClient::handleData(char *data, size_t len) {
  magic_received_time = millis();

  if(isHelloPacket(data)) {
      sendRequestPacket();
      return;
  }

  if(isMagicPacket(data)) {
      return;
  }

  if(isDataStartPacket(data)) {
      idx=0;
  }

  parse(data, len);
}

bool HwinfoClient::isHelloPacket(char* data) {
  uint8_t packet[] = { 0x50, 0x52, 0x57, 0x48, 0x01 };
  return memcmp(packet, data, sizeof packet) == 0;
}

bool HwinfoClient::isDataStartPacket(char* data) {
  uint8_t packet[] = { 0x50, 0x52, 0x57, 0x48, 0x02 };
  return memcmp(packet, data, sizeof packet) == 0;
}

bool HwinfoClient::isMagicPacket(char* data) {
  uint8_t packet[] = { 0x52, 0x52, 0x57, 0x48 /*, 0x01 or 0x02 */ };
  return memcmp(packet, data, sizeof packet) == 0;
}

void HwinfoClient::connect() {
  if(client->connected() || client->connecting()) return;

  client->connect(HWINFO_HOST, HWINFO_PORT);
}

void HwinfoClient::onConnect() {
  sendHelloPacket();
}

void HwinfoClient::onDisconnect() {
  connect();
}

void HwinfoClient::onPoll() {
  if(! magic_received_time) return;

  ulong last_received = millis() - magic_received_time;

  if(last_received > HWINFO_POLLING_RATE * 1000) {
      sendRequestPacket();
      return;
  }

  if(last_received > HWINFO_CONNECTION_TIMEOUT * 1000) {
      client->close();
      return;
  }
}

void HwinfoClient::sendHelloPacket() {
  char packet[]  = { 0x43, 0x52, 0x57, 0x48, 0x01 };
  sendHwinfoPacket(packet, sizeof packet);
}

void HwinfoClient::sendRequestPacket() {
  char packet[] = { 0x43, 0x52, 0x57, 0x48, 0x02};
  sendHwinfoPacket(packet, sizeof packet);
}

void HwinfoClient::sendHwinfoPacket(char* packet, uint8_t len) {
  client->add(packet, len);
  for(uint8_t i = len; i<128;i++) {
      client->add("\0", 1);
  }
  client->send();
}


HwinfoClient::HwinfoClient(const char* host, uint8_t polling_rate, uint8_t timeout):
  HWINFO_HOST(host)
  , HWINFO_POLLING_RATE(polling_rate)
  , HWINFO_CONNECTION_TIMEOUT(timeout)
{
  client = new AsyncClient;
  client->onConnect([](void *obj, AsyncClient *c){ ((HwinfoClient*)(obj))->onConnect(); }, this);
  client->onPoll([](void *obj, AsyncClient* c){ (void)c; ((HwinfoClient*)(obj))->onPoll(); }, this);
  client->onDisconnect([](void *obj, AsyncClient* c){ ((HwinfoClient*)(obj))->onDisconnect(); }, this);
  client->onData([](void *obj, AsyncClient* c, void *data, size_t len){ (void)c; ((HwinfoClient*)(obj))->handleData((char*)data, len); }, this);
}

HwinfoClient::~HwinfoClient() {
  delete client;
}
