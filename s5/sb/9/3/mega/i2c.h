#ifndef __I2C_H
#define __I2C_H

#include <avr/io.h>

/* Ustawia pull-upy i konfiguruje bitrate magistrali */
void i2cInit();
/* Czeka na ustawienie flagi TWINT */
void i2cWaitForComplete();
/* Wysyła warunek startu (ustawia TWSTA) */
void i2cStart();
/* Wysyła warunek stopu (ustawia TWSTO) */
void i2cStop();
/* Wysyła warunek stopu (ustawia TWSTO) i resetuje TWI */
void i2cReset();
/* Wysyła dane, czeka na ukończenie */
void i2cSend(uint8_t data);
/* Czyta dane, wysyła ACK po ukończeniu (ustawia TWEA) */
uint8_t i2cReadAck();
/* Czyta dane, wysyła NOACK po ukończeniu (nie ustawia TWEA) */
uint8_t i2cReadNoAck();

#endif
