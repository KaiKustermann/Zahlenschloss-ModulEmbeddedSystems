#ifndef TWI_H_
#define TWI_H_

void I2CStart(void);
void I2CInit(void);
void I2CStop(void);
void I2CSend(unsigned char);
void I2CSendByAddr(unsigned char, unsigned char);

#endif /* TWI_H_ */