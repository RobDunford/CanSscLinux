#ifndef CAN_H
#define CAN_H

#include <stdbool.h>
#include <stdint.h>

// another option is to extern the data and access direct
//  void CanDummySetRxBuff(uint23_t *id,char* msg , uint8_t *len);
//  //true if there is something
//  bool CanDummySetTxBuff(uint23_t *id,char* msg , uint8_t *len);

void CanStart(void);
// True if there is data
bool CanGet(uint32_t* id, char* msg, uint8_t* len);
// True if data ACKed
bool CanPut(uint32_t* id, char* msg, uint8_t len);

#endif