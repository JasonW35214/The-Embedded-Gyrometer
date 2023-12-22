/*

Embedded Challenge Fall 2023

*/

#include "mbed.h"
#include "drivers/LCD_DISCO_F429ZI.h" // file to interact with LCD
#include "challenge.h"

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

#define FILTER_COEFFICIENT 0.1f // Adjust this value as needed. This is used as the coefficient for performing High/Low-Pass filter operations
#define ERROR_SCALING_FACTOR 1.6

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

//Timer function to keep track of the change in time while measuring the distance (used to end the while exactly after 20seconds).
Ticker timer;
int secs = 0;
void timer_cb(){
    secs++;
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
        
        // This line of code prints the statement which shows that the data is being recorded after clicking the button 
        // It can also be interpreted as another evidence that the gyroscope is enabled and started recording
        snprintf(display_buf[0], 60, "Movement in Progress");
        lcd.SelectLayer(FOREGROUND);
        lcd.DisplayStringAt(5, LINE(8), (uint8_t *)display_buf[0], CENTER_MODE);

        
        while(secs <= 20){

            flags.wait_all(DATA_READY_FLAG);
            write_buf[0] = OUT_X_L | 0x80 | 0x40;

            spi.transfer(write_buf, 7, read_buf, 7, spi_cb);
            flags.wait_all(SPI_FLAG);

            // Process raw data
            raw_gx = (((uint16_t)read_buf[2]) << 8) | ((uint16_t)read_buf[1]);
            raw_gy = (((uint16_t)read_buf[4]) << 8) | ((uint16_t)read_buf[3]);
            raw_gz = (((uint16_t)read_buf[6]) << 8) | ((uint16_t)read_buf[5]);
            
            // Convert the raw data to radians per second and multiply with Gain (Sensitivity)
            gx = ((float)raw_gx) * SCALING_FACTOR;
            gy = ((float)raw_gy) * SCALING_FACTOR;
            gz = ((float)raw_gz) * SCALING_FACTOR;

            // Applying Simple low-pass filter
            filtered_gx = FILTER_COEFFICIENT * gx + (1 - FILTER_COEFFICIENT) * filtered_gx;
            filtered_gy = FILTER_COEFFICIENT * gy + (1 - FILTER_COEFFICIENT) * filtered_gy;
            filtered_gz = FILTER_COEFFICIENT * gz + (1 - FILTER_COEFFICIENT) * filtered_gz;

            // Applying simple high-pass filter with the lpf (by eliminating low freq elements)
            high_pass_gx = gx - filtered_gx;
            high_pass_gy = gy - filtered_gy;
            high_pass_gz = gz - filtered_gz;

            printf(">x_axis_high:%4.5f|g\n", high_pass_gx);
            printf(">y_axis_high:%4.5f|g\n", high_pass_gy);
            printf(">z_axis_high:%4.5f|g\n", high_pass_gz);

            // Record data samples in the array according to their axes
            gx_final[i] = high_pass_gx;
            gy_final[i] = high_pass_gy;
            gz_final[i] = high_pass_gz;
        


            //-----Conversion of Angular Velocities to the Linenar Velocity and Calculating the Distance----
            //Z axis is used as the device will be calibrated along the Z-axis
            Velocity[i] = calculateLinearVelocity(radius, gz_final[i]); // Function to calculate the instantaneous velocity
            calculateLinearDistance(Velocity[i], &Distance); // Function to calculate the instantaneous Distance


            //increment array 
            i++;

            //sleep for 500ms as we're reading the data for every 0.5 seconds
            thread_sleep_for(500);
            
        }
        //Multiplying the Distance value with ERROR_SCALING_FACTOR to minimize the error rate.
  
        float finalDistance = Distance * ERROR_SCALING_FACTOR
        printf("Distance: %.2f\n", finalDistance);

        // Printing the data to LCD Screen
        led.write(0);
        lcd.ClearStringLine(LINE(8));
        snprintf(display_buf[0], 60, "Distance Travelled:");
        snprintf(display_buf[1], 60, "%.2f metres", finalDistance);
        lcd.SelectLayer(FOREGROUND);
        lcd.DisplayStringAt(5, LINE(8), (uint8_t *)display_buf[0], CENTER_MODE);
        lcd.DisplayStringAt(5, LINE(9), (uint8_t *)display_buf[1], CENTER_MODE);
        
    }
}
