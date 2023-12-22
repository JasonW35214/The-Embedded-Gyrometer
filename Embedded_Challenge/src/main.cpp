/*

Embedded Challenge Fall 2023


Current Issues:
- Measurement is not for 20 seconds, stops at 17.5, just like in scaling factor? connection? or 
make while loop <= 21
- data not getting written to file

*/

#include "mbed.h"
#include "drivers/LCD_DISCO_F429ZI.h" // file to interact with LCD
#include "Conv.h"

// LCD Settings
#define BACKGROUND 1
#define FOREGROUND 0

#define WINDOW_SIZE 10 // Example window size, adjust as needed

// Define Regs & Configurations --> Gyroscope's settings
#define CTRL_REG1 0x20
#define CTRL_REG1_CONFIG 0b01'10'1'1'1'1
#define CTRL_REG4 0x23 // Second configure to set the DPS // page 33
#define CTRL_REG4_CONFIG 0b0'0'01'0'00'0

#define CTRL_REG3 0x22 // page 32
#define CTRL_REG3_CONFIG 0b0'0'0'0'1'000

#define OUT_X_L 0x28

#define SPI_FLAG 1
#define DATA_READY_FLAG 2
#define BUTTON_PRESSED_FLAG 4

#define SCALING_FACTOR (17.5f * 0.017453292519943295769236907684886f / 1000.0f)

#define FILTER_COEFFICIENT 0.1f // Adjust this value as needed

// EventFlags object declaration
EventFlags flags;

// DigitalIn button(PA_0);
InterruptIn button(BUTTON1);

DigitalOut led(LED1);

LCD_DISCO_F429ZI lcd;
//buffer for holding displayed text strings
char display_buf[2][60];

// spi callback function
void spi_cb(int event) {
    flags.set(SPI_FLAG);
}

// data ready callback function
void data_cb() {
    flags.set(DATA_READY_FLAG);
}

// button pushed callback function
void button_pushed_cb(){
    led = !led;
    flags.set(BUTTON_PRESSED_FLAG);
}

//sets the background layer 
//to be visible, transparent, and
//resets its colors to all black
void setup_background_layer(){
  lcd.SelectLayer(BACKGROUND);
  lcd.Clear(LCD_COLOR_BLACK);
  lcd.SetBackColor(LCD_COLOR_BLACK);
  lcd.SetTextColor(LCD_COLOR_GREEN);
  lcd.SetLayerVisible(BACKGROUND,ENABLE);
  lcd.SetTransparency(BACKGROUND,0x7Fu);
}

//resets the foreground layer to
//all black
void setup_foreground_layer(){
    lcd.SelectLayer(FOREGROUND);
    lcd.Clear(LCD_COLOR_BLACK);
    lcd.SetBackColor(LCD_COLOR_BLACK);
    lcd.SetTextColor(LCD_COLOR_LIGHTGREEN);
}

Ticker timer;
int countSecs = 0;
void timer_cb(){
    countSecs++;
}


int main() {
    // LCD layer setup
    setup_background_layer();
    setup_foreground_layer();

    //spi initialization
    SPI spi(PF_9, PF_8, PF_7, PC_1, use_gpio_ssel);
    uint8_t write_buf[32], read_buf[32];

    //interrupt initialization
    InterruptIn int2(PA_2, PullDown);
    int2.rise(&data_cb);

    //button setup
    button.rise(&button_pushed_cb);
    
    //spi format and frequency
    spi.format(8, 3);
    spi.frequency(1'000'000);

    // Write to control registers --> spi transfer
    write_buf[0] = CTRL_REG1;
    write_buf[1] = CTRL_REG1_CONFIG;
    spi.transfer(write_buf, 2, read_buf, 2, spi_cb);
    flags.wait_all(SPI_FLAG);

    write_buf[0] = CTRL_REG4;
    write_buf[1] = CTRL_REG4_CONFIG;
    spi.transfer(write_buf, 2, read_buf, 2, spi_cb);
    flags.wait_all(SPI_FLAG);

    write_buf[0] = CTRL_REG3;
    write_buf[1] = CTRL_REG3_CONFIG;
    spi.transfer(write_buf, 2, read_buf, 2, spi_cb);
    flags.wait_all(SPI_FLAG);

    write_buf[1] = 0xFF;

    //(polling for\setting) data ready flag
    if (!(flags.get() & DATA_READY_FLAG) && (int2.read() == 1)) {
        flags.set(DATA_READY_FLAG);
    }

    // Example 2: LPF definitions
    float filtered_gx = 0.0f, filtered_gy = 0.0f, filtered_gz = 0.0f;

    // Example 3: HPF definitions
    // use with the example 2 definitions
    float high_pass_gx = 0.0f, high_pass_gy = 0.0f, high_pass_gz = 0.0f;

    led.write(0);
    timer.attach(&timer_cb, 1000ms);

    
    //-----Conversion----
    //uint16_t Angular_Z; //Axes needs to be finalised during calibration units?(deg/sec or rad/sec)
    uint16_t radius = 47.5; //needs to be measured based on the tester | unit = cms
    float Velocity[40];
    float Distance = 0; //cms

    while (1) {
        //----Raw Data----
        int16_t raw_gx, raw_gy, raw_gz;
        float gx_final[40], gy_final[40], gz_final[40];
        float gx, gy, gz;
        int i = 0;

        
        //Push a button to initiate data recording for 20 seconds
        flags.wait_all(BUTTON_PRESSED_FLAG);
        countSecs = 0;
        i = 0;
        snprintf(display_buf[0], 60, "Movement in Progress");
        lcd.SelectLayer(FOREGROUND);
        lcd.DisplayStringAt(5, LINE(8), (uint8_t *)display_buf[0], CENTER_MODE);
        while(countSecs <= 20){

            flags.wait_all(DATA_READY_FLAG);
            write_buf[0] = OUT_X_L | 0x80 | 0x40;

            spi.transfer(write_buf, 7, read_buf, 7, spi_cb);
            flags.wait_all(SPI_FLAG);

            // Process raw data
            raw_gx = (((uint16_t)read_buf[2]) << 8) | ((uint16_t)read_buf[1]);
            raw_gy = (((uint16_t)read_buf[4]) << 8) | ((uint16_t)read_buf[3]);
            raw_gz = (((uint16_t)read_buf[6]) << 8) | ((uint16_t)read_buf[5]);

            gx = ((float)raw_gx) * SCALING_FACTOR;
            gy = ((float)raw_gy) * SCALING_FACTOR;
            gz = ((float)raw_gz) * SCALING_FACTOR;

            // Example 2: Apply Simple low-pass filter
            filtered_gx = FILTER_COEFFICIENT * gx + (1 - FILTER_COEFFICIENT) * filtered_gx;
            filtered_gy = FILTER_COEFFICIENT * gy + (1 - FILTER_COEFFICIENT) * filtered_gy;
            filtered_gz = FILTER_COEFFICIENT * gz + (1 - FILTER_COEFFICIENT) * filtered_gz;

            //printf("RAW -> \t\tgx: %d \t gy: %d \t gz: %d\t\n", raw_gx, raw_gy, raw_gz);

            // printf(">x_axis_low:%4.5f|g\n", filtered_gx);
            // printf(">y_axis_low:%4.5f|g\n", filtered_gy);
            // printf(">z_axis_low:%4.5f|g\n", filtered_gz);

            // Example 3: Apply simple high-pass filter with the lpf (by eliminating low freq elements)
            // to be used with example 2 (together)
            high_pass_gx = gx - filtered_gx;
            high_pass_gy = gy - filtered_gy;
            high_pass_gz = gz - filtered_gz;

            printf(">x_axis_high:%4.5f|g\n", high_pass_gx);
            printf(">y_axis_high:%4.5f|g\n", high_pass_gy);
            printf(">z_axis_high:%4.5f|g\n", high_pass_gz);

            // record data samples in array
            gx_final[i] = high_pass_gx;
            gy_final[i] = high_pass_gy;
            gz_final[i] = high_pass_gz;
            

            //----Angular Velocity----
            //float angularVelocityX, angularVelocityY, angularVelocityZ;

            //conv to vel
            //angularVelocityX = gx_final[i];
            //angularVelocityY = gy_final[i];
            //angularVelocityZ = gz_final[i];

            //vel per sec
            //float scaleFactor = 0.07;  //replace
            //float angularVelocityX_DPS = angularVelocityX * scaleFactor;
            //float angularVelocityY_DPS = angularVelocityY * scaleFactor;
            //float angularVelocityZ_DPS = angularVelocityZ * scaleFactor;


            //-----Conversion----
            //Z axis is used as device will be attached to side
            Velocity[i] = conv(radius, gz_final[i]);
            dist(Velocity[i], &Distance);


            //increment array 
            i++;

            //sleep for 500ms = 0.5 second readings
            thread_sleep_for(500);
            
        }
        printf("Distance: %f\n", Distance*1.3);
        led.write(0);
        lcd.ClearStringLine(LINE(8));
        snprintf(display_buf[0], 60, "Distance Travelled:");
        snprintf(display_buf[1], 60, "%.2f cm", Distance*1.3);
        lcd.SelectLayer(FOREGROUND);
        lcd.DisplayStringAt(5, LINE(8), (uint8_t *)display_buf[0], CENTER_MODE);
        lcd.DisplayStringAt(5, LINE(9), (uint8_t *)display_buf[1], CENTER_MODE);
        
    }
}
