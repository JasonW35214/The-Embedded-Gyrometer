 // file to interact with LCD

#include "mbed.h"
#include "drivers/LCD_DISCO_F429ZI.h"


// LCD Settings
#define BACKGROUND 1
#define FOREGROUND 0
#define MODE CENTER_MODE

LCD_DISCO_F429ZI lcd;
//buffer for holding displayed text strings
char display_buf[3][60];

//sets the background layer to be visible, transparent, and resets its colors to all black
void setup_background_layer();
//resets the foreground layer to black
void setup_foreground_layer();
//clears data on the LCD at a given line number
void clearData(int lineNumber);
// writes data to the LCD Screen
void writeData(int displayBuffer, int fontSize, char *printData, int lineNumber);
// writes data to the LCD Screen with a variable
void writeDataVariable(int displayBuffer, int fontSize, float data, int lineNumber);

