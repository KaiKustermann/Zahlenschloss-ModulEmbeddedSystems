#include <avr/io.h>

#include "i2c.h"

void I2CInit(void)
{
	/* SCL CLOCK
	 * CPU 16Mhz CLK 100khz
	 */
	TWBR = 0x48; // Set the TWI Bit Rate Register to achieve a 100kHz SCL clock
}

void I2CStart(void)
{
	TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN); // Send START condition on TWI bus
	while (!(TWCR & (1 << TWINT))) // Wait until START condition is transmitted
		;
}

void I2CSend(unsigned char data)
{
	TWDR = data; // Load data into TWI Data Register
	TWCR = (1 << TWINT) | (1 << TWEN); // Start transmission of data
	while (!(TWCR & (1 << TWINT))) // Wait until data is transmitted
		;
}

void I2CSendByAddr(unsigned char data, unsigned char addr)
{
	I2CStart(); // Send START condition
	I2CSend(addr); // Send address byte
	I2CSend(data); // Send data byte
	I2CStop(); // Send STOP condition
}

void I2CStop(void)
{
	TWCR = (1 << TWINT) | (1 << TWSTO) | (1 << TWEN); // Send STOP condition on TWI bus
}