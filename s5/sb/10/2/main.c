// L10Z2

#include <stdint.h>
#include <stdio.h>
#include <util/delay.h>
#include "hd44780.c"

int hd44780_transmit(char data, FILE *stream)
{
  LCD_WriteData(data);
  return 0;
}

FILE hd44780_file;

int main()
{
  // skonfiguruj wyświetlacz
  LCD_Initialize();
  LCD_Clear();
  // skonfiguruj strumienie wyjściowe
  fdev_setup_stream(&hd44780_file, hd44780_transmit, NULL, _FDEV_SETUP_WRITE);
  stdout = stderr = &hd44780_file;

  // program testowy

  // 0
  LCD_WriteCommand(HD44780_CGRAM_SET | 0b000000);
  for (uint8_t i = 0; i < 7; i++)
    LCD_WriteData(0b00000);
  LCD_WriteData(0b00000);

  // 1
  for (uint8_t i = 0; i < 7; i++)
    LCD_WriteData(0b10000);
  LCD_WriteData(0b00000);

  // 2
  for (uint8_t i = 0; i < 7; i++)
    LCD_WriteData(0b11000);
  LCD_WriteData(0b00000);

  // 3
  for (uint8_t i = 0; i < 7; i++)
    LCD_WriteData(0b11100);
  LCD_WriteData(0b00000);

  // 4
  for (uint8_t i = 0; i < 7; i++)
    LCD_WriteData(0b11110);
  LCD_WriteData(0b00000);

  // 5
  for (uint8_t i = 0; i < 7; i++)
    LCD_WriteData(0b11111);
  LCD_WriteData(0b00000);

  // 6
  const uint8_t L[] = {
    0b10000,
    0b10000,
    0b10010,
    0b10100,
    0b11000,
    0b10000,
    0b11111,
  };
  for (uint8_t i = 0; i < 7; i++)
    LCD_WriteData(L[i]);
  LCD_WriteData(0b00000);

  // 7
  const uint8_t ellipsis[] = {
    0b00000,
    0b00000,
    0b00000,
    0b00000,
    0b00000,
    0b00000,
    0b10101,
  };
  for (uint8_t i = 0; i < 7; i++)
    LCD_WriteData(ellipsis[i]);
  LCD_WriteData(0b00000);

  uint16_t i = 0;
  while(1) {
    for (int c = 1; c <= 5; c++) {
      _delay_ms(50);
      LCD_GoTo(i, 0);
      putchar(c);
      LCD_GoTo(0, 1);
      printf("%cadowanie%c  % 3u%%", 6, 7, (i * 5 + c) * 100 / 80);
    }
    if (++i == 16)
      break;
  }

  {
    // 0
    const uint8_t amogus1[] = {
      0b01111,
      0b11111,
      0b11001,
      0b11111,
      0b11111,
      0b01001,
      0b01001,
    };
    LCD_WriteCommand(HD44780_CGRAM_SET | 0b000000);
    for (int i = 0; i < 7; i++)
      LCD_WriteData(amogus1[i]);
    LCD_WriteData(0b00000);

    // 1
    const uint8_t amogus2[] = {
      0b00000,
      0b01111,
      0b11001,
      0b11111,
      0b11111,
      0b11111,
      0b01001,
    };
    for (int i = 0; i < 7; i++)
      LCD_WriteData(amogus2[i]);
    LCD_WriteData(0b00000);

    // 2
    const uint8_t amogus3[] = {
      0b00000,
      0b01111,
      0b11111,
      0b11001,
      0b11111,
      0b11111,
      0b01001,
    };
    for (int i = 0; i < 7; i++)
      LCD_WriteData(amogus3[i]);
    LCD_WriteData(0b00000);

    // 3
    const uint8_t amogus4[] = {
      0b01111,
      0b11111,
      0b11111,
      0b11001,
      0b11111,
      0b01001,
      0b01001,
    };
    for (int i = 0; i < 7; i++)
      LCD_WriteData(amogus4[i]);
    LCD_WriteData(0b00000);

    LCD_GoTo(0, 1);
    printf("% 16c", ' ');
    i = 0;
    while(1) {
      LCD_GoTo(0, 1);
      putchar(i++);
      _delay_ms(100);
      if (i == 4)
        i = 0;
    }
  }
}
