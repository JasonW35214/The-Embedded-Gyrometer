 // file to interact with LCD


#include "drivers/LCD_DISCO_F429ZI.h"


// LCD Settings
#define BACKGROUND 1
#define FOREGROUND 0
#define MODE CENTER_MODE

LCD_DISCO_F429ZI lcd;
//buffer for holding displayed text strings
char display_buf[2][60];

//sets the background layer to be visible, transparent, and resets its colors to all black
void setup_background_layer(){
  lcd.SelectLayer(BACKGROUND);
  lcd.Clear(LCD_COLOR_BLACK);
  lcd.SetBackColor(LCD_COLOR_BLACK);
  lcd.SetTextColor(LCD_COLOR_GREEN);
  lcd.SetLayerVisible(BACKGROUND,ENABLE);
  lcd.SetTransparency(BACKGROUND,0x7Fu);
}

//resets the foreground layer to black
void setup_foreground_layer(){
    lcd.SelectLayer(FOREGROUND);
    lcd.Clear(LCD_COLOR_BLACK);
    lcd.SetBackColor(LCD_COLOR_BLACK);
    lcd.SetTextColor(LCD_COLOR_LIGHTGREEN);
}

void clearData(int lineNumber){
    lcd.ClearStringLine(LINE(lineNumber));
}
void writeData(int displayBuffer, int fontSize, char *printData, int lineNumber){
    snprintf(display_buf[displayBuffer], fontSize, printData);
    lcd.SelectLayer(FOREGROUND);
    lcd.DisplayStringAt(5, LINE(lineNumber), (uint8_t *)display_buf[displayBuffer], MODE);
}

void writeDataVariable(int displayBuffer, int fontSize, float data, int lineNumber){
    snprintf(display_buf[displayBuffer], fontSize, "%.2f metres", data);
    lcd.SelectLayer(FOREGROUND);
    lcd.DisplayStringAt(5, LINE(lineNumber), (uint8_t *)display_buf[displayBuffer], MODE);
}
