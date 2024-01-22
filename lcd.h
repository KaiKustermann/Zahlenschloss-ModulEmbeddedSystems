#ifndef LCD_H_
#define LCD_H_

void LCDInit(void);
void LCDsend4Bit(unsigned char);
void LCDsend8Bit(unsigned char, unsigned char);
void LCDSetCursorPosition(unsigned char, unsigned char);
void LCDWriteString(char[]);
void LCDclear(void);
void LCDBackLight(unsigned char);
void LCDclearRow(uint8_t);
void LCDclearLastInput(void);

#endif