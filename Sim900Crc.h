/* 
* Sim900Crc.h
*
* Created: 2014-03-21 12:46:05
* Author: Tomasz Œcis³owicz
*/
#include <stdint.h>

#ifndef __SIM900CRC_H__
#define __SIM900CRC_H__

#define CRC8INIT  0x00
#define CRC8POLY  0x18          //0X18 = X^8+X^5+X^4+X^0

uint8_t crc8 ( uint8_t *data_in, uint16_t number_of_bytes_to_read );

#endif //__SIM900CRC_H__
