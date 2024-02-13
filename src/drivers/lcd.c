#include <util/delay.h>
#include <string.h>

#include "lcd.h"
#include "i2c.h"
#include "../logging/logging.h"

unsigned char portlcd = 0;

void LCDInit(void)
{
	I2CInit();
	_delay_ms(50);

	LCDsend4Bit(0b00000011);
	_delay_ms(5);
	LCDsend4Bit(0b00000011);
	_delay_us(100);
	LCDsend4Bit(0b00000011);
	_delay_ms(100);
	LCDsend4Bit(0b00000010);
	_delay_ms(1);
	LCDSend8Bit(0b00101000, 0); // Data 4bit, Line 2, Font 5x8
	_delay_ms(1);
	LCDSend8Bit(0b00001101, 0); // Display ON, Cursor OFF, Blink ON
	_delay_ms(1);

	I2CSendByAddr(portlcd |= 0x08, 0x4E);  // BackLight ON
	I2CSendByAddr(portlcd &= ~0x02, 0x4E); // LCD Write ON

	_delay_ms(100);
	LCDClear();
	LCDSetCursorPosition(0, 1);
}

void LCDClearRow(uint8_t row)
{
	LCDSetCursorPosition(0, row - 1);
	LCDWriteString("                ");
	LCDSetCursorPosition(0, row - 1);
}

void LCDClearLastInput(void)
{
	LCDSend8Bit(0b00010001, 0);
	LCDSend8Bit(' ', 1);
	LCDSend8Bit(0b00010001, 0);
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

void LCDSend8Bit(unsigned char c, unsigned char mode)
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
	uint8_t n;
	for (n = 0; s[n] != '\0'; n++){
		LCDSend8Bit(s[n], 1);
	}
}

void LCDSetCursorPosition(unsigned char x, unsigned char y)
{
	switch (y)
	{
	case 0:
		LCDSend8Bit(x | 0x80, 0);
		break;
	case 1:
		LCDSend8Bit((0x40 + x) | 0x80, 0);
		break;
	case 2:
		LCDSend8Bit((0x10 + x) | 0x80, 0);
		break;
	case 3:
		LCDSend8Bit((0x50 + x) | 0x80, 0);
		break;
	}
}

void LCDClear(void)
{
	LCDSend8Bit(0x01, 0);
	_delay_ms(10);
}

uint8_t validateRowMessage(char* message){
	if(strlen(message) > 16){
		logMessage("lcd.c: validation of message failed, message is too long (a maximum of 16 characters is allowed)", ERROR);
		return 1;
	}
	return 0;
}

void LCDOverwriteStringRowOne(char* message){
	if(validateRowMessage(message) != 0){
		return;
	}
	LCDClearRow(1);
	LCDSetCursorPosition(0, 0);
	LCDWriteString(message);
}

void LCDOverwriteStringRowTwo(char* message){
	if(validateRowMessage(message) != 0){
		return;
	}
	LCDClearRow(2);
	LCDSetCursorPosition(0, 1);
	LCDWriteString(message);
}