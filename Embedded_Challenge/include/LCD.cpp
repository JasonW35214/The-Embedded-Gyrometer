#include "mbed.h"
#include "LCD.h"

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

void clearData(){
    lcd.Clear(FOREGROUND);
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
