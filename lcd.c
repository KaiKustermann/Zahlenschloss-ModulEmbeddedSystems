#include <util/delay.h>

#include "lcd.h"
#include "i2c.h"

unsigned char portlcd = 0;

void LCDInit(void)
{
	LCDsend4Bit(0b00000011);
	_delay_ms(5);
	LCDsend4Bit(0b00000011);
	_delay_us(100);
	LCDsend4Bit(0b00000011);
	_delay_ms(100);
	LCDsend4Bit(0b00000010);
	_delay_ms(1);
	LCDsend8Bit(0b00101000, 0); // Data 4bit, Line 2, Font 5x8
	_delay_ms(1);
	LCDsend8Bit(0b00001110, 0); // Display ON, Cursor ON, Blink OFF
	_delay_ms(1);

	I2CSendByAddr(portlcd |= 0x08, 0x4E);  // BackLight ON
	I2CSendByAddr(portlcd &= ~0x02, 0x4E); // LCD Write ON
}

void LCDBackLight(unsigned char mode)
{
	switch (mode)
	{
	case 0:
		I2CSendByAddr(portlcd &= ~0x08, 0x4E); // BackLight OFF
		break;
	case 1:
		I2CSendByAddr(portlcd |= 0x08, 0x4E); // BackLight ON
		break;
	}
}

void LCDsend4Bit(unsigned char c)
{
	c <<= 4;

	I2CSendByAddr(portlcd |= 0x04, 0x4E); // Enable E
	_delay_us(50);

	I2CSendByAddr(portlcd | c, 0x4E);

	I2CSendByAddr(portlcd &= ~0x04, 0x4E); // Disable E
	_delay_us(50);
}

void LCDsend8Bit(unsigned char c, unsigned char mode)
{
	if (mode == 0)
		I2CSendByAddr(portlcd &= ~0x01, 0x4E);
	else
		I2CSendByAddr(portlcd |= 0x01, 0x4E);
	unsigned char hc = 0;
	hc = c >> 4;
	LCDsend4Bit(hc);
	LCDsend4Bit(c);
}

void LCDWriteString(char s[])
{
	char n;
	for (n = 0; s[n] != '\0'; n++)
		LCDsend8Bit(s[n], 1);
}

void LCDSetCursorPosition(unsigned char x, unsigned char y)
{
	switch (y)
	{
	case 0:
		LCDsend8Bit(x | 0x80, 0);
		break;
	case 1:
		LCDsend8Bit((0x40 + x) | 0x80, 0);
		break;
	case 2:
		LCDsend8Bit((0x10 + x) | 0x80, 0);
		break;
	case 3:
		LCDsend8Bit((0x50 + x) | 0x80, 0);
		break;
	}
}

void LCDclear(void)
{
	LCDsend8Bit(0x01, 0);
	_delay_ms(10);
}