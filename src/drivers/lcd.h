#ifndef LCD_H_
#define LCD_H_

void LCDInit(void);
void LCDsend4Bit(unsigned char);
void LCDSend8Bit(unsigned char, unsigned char);
void LCDSetCursorPosition(unsigned char, unsigned char);
void LCDWriteString(char[]);
void LCDClear(void);
void LCDBackLight(unsigned char);
void LCDClearRow(uint8_t);
void LCDClearLastInput(void);
void LCDOverwriteStringRowOne(char* message);
void LCDOverwriteStringRowTwo(char* message);

#endif