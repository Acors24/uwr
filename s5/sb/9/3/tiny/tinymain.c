// L9Z3

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#define DDR_USI DDRA
#define PORT_USI PORTA
#define PIN_USI PINA
#define PIN_USI_SCL PA4
#define PIN_USI_SDA PA6

#define MESSAGEBUF_SIZE 4
#define TWI_GEN_CALL 0x00

#define TWI_CMD_MASTER_WRITE 0x10
#define TWI_CMD_MASTER_READ 0x20

#include "USI_TWI_Master.c"

int main() {
  uint8_t messageBuf[MESSAGEBUF_SIZE];
  uint8_t TWI_targetSlaveAddress = 0x7F;
  uint8_t data = 0;
  uint8_t result = TRUE;

  USI_TWI_Master_Initialise();

  sei();

  while (1) {
    messageBuf[0] = (TWI_targetSlaveAddress << TWI_ADR_BITS) | (FALSE << TWI_READ_BIT);
    messageBuf[1] = TWI_CMD_MASTER_WRITE;
    messageBuf[2] = data;
    result = USI_TWI_Start_Transceiver_With_Data(messageBuf, 3);

    _delay_ms(1000);
    data++;
  }
}
