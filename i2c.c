#include <avr/io.h>

#include "i2c.h"

void I2CInit(void)
{
	/* SCL CLOCK
	 * CPU 16Mhz CLK 100khz
	 */
	TWBR = 0x48;
}

void I2CStart(void)
{
	TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
	while (!(TWCR & (1 << TWINT)))
		;
}

void I2CSend(unsigned char data)
{
	TWDR = data;
	TWCR = (1 << TWINT) | (1 << TWEN);
	while (!(TWCR & (1 << TWINT)))
		;
}

void I2CSendByAddr(unsigned char data, unsigned char addr)
{
	I2CStart();
	I2CSend(addr);
	I2CSend(data);
	I2CStop();
}

void I2CStop(void)
{
	TWCR = (1 << TWINT) | (1 << TWSTO) | (1 << TWEN);
}