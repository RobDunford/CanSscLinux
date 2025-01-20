#ifndef DATA_STORE_DEFINED
#define DATA_STORE_DEFINED
#include <stdbool.h>
#include <stdint.h>

void DataStoreInit(void);

bool DataStoreVerifyPut(uint16_t id, uint32_t len);

bool DataStoreVerifyGet(uint16_t id, uint32_t* len);

uint8_t DataStoreGet(uint16_t id, char* pData, uint16_t maxLen);

bool DataStorePut(uint16_t id, char* pData, uint8_t len);

void DataStoreClose(uint16_t id, bool state);

void DataStoreListFiles(void);

#endif
