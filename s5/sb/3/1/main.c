#define __AVR_ATmega328P__
#define __DELAY_BACKWARD_COMPATIBLE__

#include <avr/pgmspace.h>
#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <inttypes.h>

#define BUZZ PB5
#define BUZZ_DDR DDRB
#define BUZZ_PORT PORTB

#include "notes.h"
#include "durations.h"

#define NOTE_PAUSE UINT16_MAX

#define FLASH_STEP(address)  pgm_read_word(&address[0])
#define FLASH_DELAY(address) pgm_read_word(&address[1])

#define TONE(step, delay) \
    for (uint16_t i = 0; i < (uint32_t)1000 * (delay) / (step) / 2; i++) { \
      BUZZ_PORT |= _BV(BUZZ); \
      _delay_us(step); \
      BUZZ_PORT &= ~_BV(BUZZ); \
      _delay_us(step); \
    }

int main() {
  BUZZ_DDR |= _BV(BUZZ);

  static const uint16_t SANS[] PROGMEM = {
    NOTE_D3, DUR_16,
    NOTE_D3, DUR_16,
    NOTE_D4, DUR_8,
    NOTE_A3, DUR_8,
    NOTE_PAUSE, DUR_16,
    NOTE_G3, DUR_16,
    NOTE_PAUSE, DUR_16,
    NOTE_G3, DUR_16,
    NOTE_PAUSE, DUR_16,
    NOTE_F3, DUR_16,
    NOTE_F3, DUR_16,
    NOTE_D3, DUR_16,
    NOTE_F3, DUR_16,
    NOTE_G3, DUR_16,
    
    NOTE_C3, DUR_16,
    NOTE_C3, DUR_16,
    NOTE_D4, DUR_8,
    NOTE_A3, DUR_8,
    NOTE_PAUSE, DUR_16,
    NOTE_G3, DUR_16,
    NOTE_PAUSE, DUR_16,
    NOTE_G3, DUR_16,
    NOTE_PAUSE, DUR_16,
    NOTE_F3, DUR_16,
    NOTE_F3, DUR_16,
    NOTE_D3, DUR_16,
    NOTE_F3, DUR_16,
    NOTE_G3, DUR_16,
    

    NOTE_H2, DUR_16,
    NOTE_H2, DUR_16,
    NOTE_D4, DUR_8,
    NOTE_A3, DUR_8,
    NOTE_PAUSE, DUR_16,
    NOTE_G3, DUR_16,
    NOTE_PAUSE, DUR_16,
    NOTE_G3, DUR_16,
    NOTE_PAUSE, DUR_16,
    NOTE_F3, DUR_16,
    NOTE_F3, DUR_16,
    NOTE_D3, DUR_16,
    NOTE_F3, DUR_16,
    NOTE_G3, DUR_16,
    
    NOTE_H2, DUR_16,
    NOTE_H2, DUR_16,
    NOTE_D4, DUR_8,
    NOTE_A3, DUR_8,
    NOTE_PAUSE, DUR_16,
    NOTE_G3, DUR_16,
    NOTE_PAUSE, DUR_16,
    NOTE_G3, DUR_16,
    NOTE_PAUSE, DUR_16,
    NOTE_F3, DUR_16,
    NOTE_F3, DUR_16,
    NOTE_D3, DUR_16,
    NOTE_F3, DUR_16,
    NOTE_G3, DUR_16,
    

    NOTE_D4, DUR_16,
    NOTE_D4, DUR_16,
    NOTE_D5, DUR_8,
    NOTE_A4, DUR_8,
    NOTE_PAUSE, DUR_16,
    NOTE_G4, DUR_16,
    NOTE_PAUSE, DUR_16,
    NOTE_G4, DUR_16,
    NOTE_PAUSE, DUR_16,
    NOTE_F4, DUR_16,
    NOTE_F4, DUR_16,
    NOTE_D4, DUR_16,
    NOTE_F4, DUR_16,
    NOTE_G4, DUR_16,
    
    NOTE_C4, DUR_16,
    NOTE_C4, DUR_16,
    NOTE_D5, DUR_8,
    NOTE_A4, DUR_8,
    NOTE_PAUSE, DUR_16,
    NOTE_G4, DUR_16,
    NOTE_PAUSE, DUR_16,
    NOTE_G4, DUR_16,
    NOTE_PAUSE, DUR_16,
    NOTE_F4, DUR_16,
    NOTE_F4, DUR_16,
    NOTE_D4, DUR_16,
    NOTE_F4, DUR_16,
    NOTE_G4, DUR_16,
    

    NOTE_H3, DUR_16,
    NOTE_H3, DUR_16,
    NOTE_D5, DUR_8,
    NOTE_A4, DUR_8,
    NOTE_PAUSE, DUR_16,
    NOTE_G4, DUR_16,
    NOTE_PAUSE, DUR_16,
    NOTE_G4, DUR_16,
    NOTE_PAUSE, DUR_16,
    NOTE_F4, DUR_16,
    NOTE_F4, DUR_16,
    NOTE_D4, DUR_16,
    NOTE_F4, DUR_16,
    NOTE_G4, DUR_16,
    
    NOTE_H3, DUR_16,
    NOTE_H3, DUR_16,
    NOTE_D5, DUR_8,
    NOTE_A4, DUR_8,
    NOTE_PAUSE, DUR_16,
    NOTE_G4, DUR_16,
    NOTE_PAUSE, DUR_16,
    NOTE_G4, DUR_16,
    NOTE_PAUSE, DUR_16,
    NOTE_F4, DUR_16,
    NOTE_F4, DUR_16,
    NOTE_D4, DUR_16,
    NOTE_C4, DUR_16,
    NOTE_D4, DUR_32,
    NOTE_E4, DUR_32,
    
    
    NOTE_F4, DUR_8,
    NOTE_F4, DUR_16,
    NOTE_F4, DUR_16,
    NOTE_PAUSE, DUR_16,
    NOTE_F4, DUR_16,
    NOTE_PAUSE, DUR_16,
    NOTE_F4, DUR_16,
    NOTE_F4, DUR_16,
    NOTE_D4, DUR_16,
    NOTE_PAUSE, DUR_16,
    NOTE_D4, DUR_16,
    NOTE_D4, DUR_4,

    NOTE_F4, DUR_8,
    NOTE_F4, DUR_16,
    NOTE_F4, DUR_16,
    NOTE_PAUSE, DUR_16,
    NOTE_G4, DUR_16,
    NOTE_PAUSE, DUR_16,
    NOTE_A4, DUR_16,
    NOTE_A4, DUR_16,
    NOTE_G4, DUR_32,
    NOTE_A4, DUR_32,
    NOTE_G4, DUR_16,
    NOTE_D4, DUR_16,
    NOTE_F4, DUR_16,
    NOTE_G4, DUR_16,
    NOTE_PAUSE, DUR_8,


    NOTE_F4, DUR_8,
    NOTE_F4, DUR_16,
    NOTE_F4, DUR_16,
    NOTE_PAUSE, DUR_16,
    NOTE_G4, DUR_16,
    NOTE_PAUSE, DUR_16,
    NOTE_A4, DUR_16,
    NOTE_PAUSE, DUR_16,
    NOTE_A4, DUR_16,
    NOTE_PAUSE, DUR_16,
    NOTE_C5, DUR_16,
    NOTE_PAUSE, DUR_16,
    NOTE_A4, DUR_16,
    NOTE_A4, DUR_8,

    NOTE_D5, DUR_8,
    NOTE_D5, DUR_8,
    NOTE_D5, DUR_16,
    NOTE_A4, DUR_16,
    NOTE_D5, DUR_16,
    NOTE_C5, DUR_16,
    NOTE_C5, DUR_8 + DUR_16,
    NOTE_G5, DUR_16,
    NOTE_G5, DUR_8,


    NOTE_A4, DUR_8,
    NOTE_A4, DUR_16,
    NOTE_A4, DUR_16,
    NOTE_PAUSE, DUR_16,
    NOTE_A4, DUR_16,
    NOTE_PAUSE, DUR_16,
    NOTE_A4, DUR_16,
    NOTE_G4, DUR_16,
    NOTE_PAUSE, DUR_16,
    NOTE_G4, DUR_16,
    NOTE_G4, DUR_16,
    
    NOTE_A4, DUR_8,
    NOTE_A4, DUR_8,
    NOTE_A4, DUR_16,
    NOTE_A4, DUR_16,
    NOTE_PAUSE, DUR_16,
    NOTE_G4, DUR_16,
    NOTE_PAUSE, DUR_16,
    NOTE_A4, DUR_16,
    NOTE_PAUSE, DUR_16,
    NOTE_D5, DUR_16,
    NOTE_D5, DUR_16,
    NOTE_A4, DUR_16,
    NOTE_G4, DUR_8,
    

    NOTE_D5, DUR_8,
    NOTE_A4, DUR_8,
    NOTE_G4, DUR_8,
    NOTE_F4, DUR_8,
    NOTE_D5, DUR_8,
    NOTE_A4, DUR_8,
    NOTE_G4, DUR_8,
    NOTE_F4, DUR_8,

    NOTE_H3, DUR_8,
    NOTE_D4, DUR_16,
    NOTE_E4, DUR_8,
    NOTE_F4, DUR_8,
    NOTE_C5, DUR_16,
    NOTE_C5, DUR_2,


    NOTE_PAUSE, DUR_2,
    NOTE_F4, DUR_16,
    NOTE_D4, DUR_16,
    NOTE_F4, DUR_16,
    NOTE_G4, DUR_16,
    NOTE_A4, DUR_16,
    NOTE_G4, DUR_16,
    NOTE_F4, DUR_16,
    NOTE_D4, DUR_16,

    NOTE_A4, DUR_32,
    NOTE_G4, DUR_32,
    NOTE_D4, DUR_16,
    NOTE_F4, DUR_8,
    NOTE_G4, DUR_2,
    NOTE_G4, DUR_16,
    NOTE_G4, DUR_8,
    NOTE_A4, DUR_16,


    NOTE_C5, DUR_16,
    NOTE_A4, DUR_16,
    NOTE_G4, DUR_16,
    NOTE_G4, DUR_16,
    NOTE_F4, DUR_16,
    NOTE_D4, DUR_16,
    NOTE_E4, DUR_16,
    NOTE_F4, DUR_8,
    NOTE_G4, DUR_8,
    NOTE_C5, DUR_8,

    NOTE_D5, DUR_8,
    NOTE_A4, DUR_16,
    NOTE_PAUSE, DUR_16,
    NOTE_A4, DUR_16,
    NOTE_G4, DUR_16,
    NOTE_F4, DUR_16,
    NOTE_G4, DUR_16,
    NOTE_G4, DUR_8 + DUR_16,
    NOTE_G4, DUR_16,


    NOTE_D4, DUR_8,
    NOTE_E4, DUR_8,
    NOTE_F4, DUR_8,
    NOTE_F5, DUR_8,
    NOTE_E5, DUR_4,
    NOTE_D5, DUR_4,

    NOTE_E5, DUR_4,
    NOTE_F5, DUR_4,
    NOTE_G5, DUR_4,
    NOTE_E5, DUR_4,


    NOTE_A5, DUR_2,
    NOTE_A5, DUR_16,
    NOTE_G5, DUR_16,
    NOTE_G5, DUR_16,
    NOTE_F5, DUR_16,
    NOTE_F5, DUR_16,
    NOTE_E5, DUR_16,
    NOTE_D5, DUR_16,
    NOTE_D5, DUR_16,

    NOTE_C5, DUR_4,
    NOTE_C5, DUR_4,
    NOTE_E5, DUR_2,


    NOTE_G1, DUR_8,
    NOTE_G1, DUR_8,
    NOTE_G1, DUR_16,
    NOTE_G1, DUR_16,
    NOTE_PAUSE, DUR_16,
    NOTE_G1, DUR_16,
    NOTE_PAUSE, DUR_16,
    NOTE_G1, DUR_16,
    NOTE_PAUSE, DUR_16,
    NOTE_G1, DUR_16,
    NOTE_G1, DUR_16,
    NOTE_G1, DUR_16,
    NOTE_G1, DUR_8,

    NOTE_A1, DUR_8,
    NOTE_A1, DUR_8,
    NOTE_A1, DUR_16,
    NOTE_A1, DUR_16,
    NOTE_PAUSE, DUR_16,
    NOTE_A1, DUR_16,
    NOTE_PAUSE, DUR_16,
    NOTE_A1, DUR_16,
    NOTE_PAUSE, DUR_16,
    NOTE_A1, DUR_16,
    NOTE_A1, DUR_16,
    NOTE_A1, DUR_16,
    NOTE_A1, DUR_8,


    NOTE_PAUSE, DUR_16,
    NOTE_H1, DUR_16,
    NOTE_H2, DUR_8,
    NOTE_F2, DUR_16,
    NOTE_PAUSE, DUR_16,
    NOTE_F2, DUR_16,
    NOTE_PAUSE, DUR_16,
    NOTE_E2, DUR_16,
    NOTE_PAUSE, DUR_16,
    NOTE_D2, DUR_16,
    NOTE_D2, DUR_16,
    NOTE_H2, DUR_16,
    NOTE_D2, DUR_16,
    NOTE_E2, DUR_16,

    NOTE_A1, DUR_16,
    NOTE_A1, DUR_16,
    NOTE_H2, DUR_8,
    NOTE_F2, DUR_8,
    NOTE_PAUSE, DUR_16,
    NOTE_F2, DUR_16,
    NOTE_PAUSE, DUR_16,
    NOTE_E2, DUR_16,
    NOTE_PAUSE, DUR_16,
    NOTE_D2, DUR_16,
    NOTE_D2, DUR_16,
    NOTE_H2, DUR_16,
    NOTE_D2, DUR_16,
    NOTE_E2, DUR_16,


    NOTE_G1, DUR_16,
    NOTE_G1, DUR_16,
    NOTE_H2, DUR_8,
    NOTE_F2, DUR_8,
    NOTE_PAUSE, DUR_16,
    NOTE_F2, DUR_16,
    NOTE_PAUSE, DUR_16,
    NOTE_E2, DUR_16,
    NOTE_PAUSE, DUR_16,
    NOTE_D2, DUR_16,
    NOTE_D2, DUR_16,
    NOTE_H1, DUR_16,
    NOTE_D2, DUR_16,
    NOTE_E2, DUR_16,

    NOTE_G4, DUR_16,
    NOTE_D5, DUR_16,
    NOTE_F5, DUR_16,
    NOTE_D5, DUR_16,
    NOTE_G5, DUR_8,
    NOTE_F5, DUR_8,
    NOTE_D5, DUR_16,
    NOTE_C5, DUR_16,
    NOTE_PAUSE, DUR_16,
    NOTE_A4, DUR_16,
    NOTE_A4, DUR_16,
    NOTE_G4, DUR_16,
    NOTE_A4, DUR_16,
    NOTE_C5, DUR_16,


    NOTE_PAUSE, DUR_16,
    NOTE_H1, DUR_16,
    NOTE_H2, DUR_8,
    NOTE_F2, DUR_8,
    NOTE_PAUSE, DUR_16,
    NOTE_F2, DUR_16,
    NOTE_PAUSE, DUR_16,
    NOTE_E2, DUR_16,
    NOTE_PAUSE, DUR_16,
    NOTE_D2, DUR_16,
    NOTE_D2, DUR_16,
    NOTE_H1, DUR_16,
    NOTE_D2, DUR_16,
    NOTE_E2, DUR_16,

    NOTE_A1, DUR_16,
    NOTE_A1, DUR_16,
    NOTE_H2, DUR_8,
    NOTE_F2, DUR_8,
    NOTE_PAUSE, DUR_16,
    NOTE_F2, DUR_16,
    NOTE_PAUSE, DUR_16,
    NOTE_E2, DUR_16,
    NOTE_PAUSE, DUR_16,
    NOTE_D2, DUR_16,
    NOTE_D2, DUR_16,
    NOTE_H1, DUR_16,
    NOTE_D2, DUR_16,
    NOTE_E2, DUR_16,


    NOTE_D3, DUR_8,
    NOTE_F4, DUR_8,
    NOTE_E4, DUR_16,
    NOTE_PAUSE, DUR_16,
    NOTE_C4, DUR_16,
    NOTE_PAUSE, DUR_16,
    NOTE_E4, DUR_16,
    NOTE_PAUSE, DUR_16,
    NOTE_D4, DUR_16,
    NOTE_D4, DUR_16,
    NOTE_G3, DUR_16,
    NOTE_A3, DUR_16,
    NOTE_C4, DUR_16,

    NOTE_PAUSE, DUR_8,
    NOTE_F4, DUR_8,
    NOTE_E4, DUR_8,
    NOTE_PAUSE, DUR_16,
    NOTE_C4, DUR_16,
    NOTE_PAUSE, DUR_16,
    NOTE_E4, DUR_16,
    NOTE_PAUSE, DUR_16,
    NOTE_D4, DUR_16,
    NOTE_D4, DUR_16,
    NOTE_G3, DUR_16,
    NOTE_A3, DUR_16,
    NOTE_C4, DUR_16,


    NOTE_G1, DUR_8,
    NOTE_G1, DUR_8,
    NOTE_G1, DUR_16,
    NOTE_G1, DUR_16,
    NOTE_PAUSE, DUR_16,
    NOTE_G1, DUR_16,
    NOTE_PAUSE, DUR_16,
    NOTE_G1, DUR_16,
    NOTE_PAUSE, DUR_16,
    NOTE_G1, DUR_16,
    NOTE_G1, DUR_16,
    NOTE_G1, DUR_16,
    NOTE_G1, DUR_8,
    
    NOTE_A1, DUR_8,
    NOTE_A1, DUR_8,
    NOTE_A1, DUR_16,
    NOTE_A1, DUR_16,
    NOTE_PAUSE, DUR_16,
    NOTE_A1, DUR_16,
    NOTE_PAUSE, DUR_16,
    NOTE_A1, DUR_16,
    NOTE_PAUSE, DUR_16,
    NOTE_A1, DUR_16,
    NOTE_A1, DUR_16,
    NOTE_A1, DUR_16,
    NOTE_A1, DUR_16,
    NOTE_C1, DUR_16, // << C1


    NOTE_H2, DUR_16,
    NOTE_H2, DUR_16,
    NOTE_D4, DUR_8,
    NOTE_A3, DUR_8,
    NOTE_PAUSE, DUR_16,
    NOTE_G3, DUR_16,
    NOTE_PAUSE, DUR_16,
    NOTE_G3, DUR_16,
    NOTE_PAUSE, DUR_16,
    NOTE_F3, DUR_16,
    NOTE_F3, DUR_16,
    NOTE_D3, DUR_16,
    NOTE_F3, DUR_16,
    NOTE_G3, DUR_16,

    NOTE_C3, DUR_16,
    NOTE_C3, DUR_16,
    NOTE_D4, DUR_8,
    NOTE_A3, DUR_8,
    NOTE_PAUSE, DUR_16,
    NOTE_G3, DUR_16,
    NOTE_PAUSE, DUR_16,
    NOTE_G3, DUR_16,
    NOTE_PAUSE, DUR_16,
    NOTE_F3, DUR_16,
    NOTE_F3, DUR_16,
    NOTE_D3, DUR_16,
    NOTE_F3, DUR_16,
    NOTE_G3, DUR_16,


    NOTE_D3, DUR_16,
    NOTE_D3, DUR_16,
    NOTE_D4, DUR_8,
    NOTE_A3, DUR_8,
    NOTE_PAUSE, DUR_16,
    NOTE_G3, DUR_16,
    NOTE_PAUSE, DUR_16,
    NOTE_G3, DUR_16,
    NOTE_PAUSE, DUR_16,
    NOTE_F3, DUR_16,
    NOTE_F3, DUR_16,
    NOTE_D3, DUR_16,
    NOTE_F3, DUR_16,
    NOTE_G3, DUR_16,

    NOTE_D3, DUR_16,
    NOTE_D3, DUR_16,
    NOTE_D4, DUR_8,
    NOTE_A3, DUR_8,
    NOTE_PAUSE, DUR_16,
    NOTE_G3, DUR_16,
    NOTE_PAUSE, DUR_16,
    NOTE_G3, DUR_16,
    NOTE_PAUSE, DUR_16,
    NOTE_F3, DUR_16,
    NOTE_F3, DUR_16,
    NOTE_D3, DUR_16,
    NOTE_F3, DUR_16,
    NOTE_G3, DUR_16,
  };

  uint16_t* current_note;
  
  uint16_t step, delay;
  while (1) {
    for (current_note = SANS; current_note != SANS + sizeof(SANS) / sizeof(uint16_t); current_note += 2) {
      step = FLASH_STEP(current_note);
      delay = FLASH_DELAY(current_note);
      TONE(step, delay);
      _delay_ms(5);
    }

    _delay_ms(2000);
  }
}
