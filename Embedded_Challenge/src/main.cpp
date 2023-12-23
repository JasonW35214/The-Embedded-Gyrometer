/* Embedded Challenge Fall 2023 */

/* Embedded Challenge Fall 2023 */

#include "mbed.h"
#include "challenge.h"
#include "gyroscope.h"
#include "LCD.h"



#define ERROR_SCALING_FACTOR 1.6


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

    //interrupt initialization
    InterruptIn int2(PA_2, PullDown);
    int2.rise(&data_cb);

    //button setup
    button.rise(&button_pushed_cb);
    
    setSPIformat();
    setupWriteRegister(CTRL_REG1, CTRL_REG1_CONFIG);
    spi.transfer(write_buf, 2, read_buf, 2, spi_cb);
    flags.wait_all(SPI_FLAG);

    setupWriteRegister(CTRL_REG3, CTRL_REG3_CONFIG);
    spi.transfer(write_buf, 2, read_buf, 2, spi_cb);
    flags.wait_all(SPI_FLAG);

    setupWriteRegister(CTRL_REG4, CTRL_REG4_CONFIG);
    spi.transfer(write_buf, 2, read_buf, 2, spi_cb);
    flags.wait_all(SPI_FLAG);


    write_buf[1] = 0xFF;

    //(polling for\setting) data ready flag
    // if (!(flags.get() & DATA_READY_FLAG) && (int2.read() == 1)) {
    //     flags.set(DATA_READY_FLAG);
    // }
    checkPolling();


    // LPF definitions
    float filtered_gx = 0.0f, filtered_gy = 0.0f, filtered_gz = 0.0f;

    // HPF definitions
    float high_pass_gx = 0.0f, high_pass_gy = 0.0f, high_pass_gz = 0.0f;
    
    //-----Conversion----//
    //uint16_t Angular_Z; //Axes needs to be finalised during calibration units?(deg/sec or rad/sec)
    // Radius needs to be measured based on the tester | unit = metres
    float Velocity[40]; // unit = m/s
    float Distance = 0; // metres

    led.write(0); // LED is set to off initially
    timer.attach(&timer_cb, 1000ms); // timer is being attached  to the secs variable

    while (1) {
        //----Raw Data----//
        // int16_t raw_gx, raw_gy, raw_gz;
        GyroscopeRawData *rawData;
        GyroscopeScaled *scaledData;
        GyroscopeFiltered *filteredData;
        float final_x_data[40], final_y_data[40], final_z_data[40];
        // float gx, gy, gz;
        int i = 0;

        
        //Push a button to initiate data recording for 20 seconds
        flags.wait_all(BUTTON_PRESSED_FLAG);
        
        // This line of code prints the statement which shows that the data is being recorded after clicking the button 
        // It can also be interpreted as another evidence that the gyroscope is enabled and started recording
        /*snprintf(display_buf[0], 60, "Movement in Progress");
        lcd.SelectLayer(FOREGROUND);
        lcd.DisplayStringAt(5, LINE(8), (uint8_t *)display_buf[0], CENTER_MODE);*/
        writeData(0, 60, "\"Movement in Progress\"", 8);

        
        while(secs <= 20){

            flags.wait_all(DATA_READY_FLAG);
            write_buf[0] = OUT_X_L | 0x80 | 0x40;

            spi.transfer(write_buf, 7, read_buf, 7, spi_cb);
            flags.wait_all(SPI_FLAG);

            // Process raw data
            // raw_gx = (((uint16_t)read_buf[2]) << 8) | ((uint16_t)read_buf[1]);
            // raw_gy = (((uint16_t)read_buf[4]) << 8) | ((uint16_t)read_buf[3]);
            // raw_gz = (((uint16_t)read_buf[6]) << 8) | ((uint16_t)read_buf[5]);
            readData(rawData);

            
            // Convert the raw data to radians per second and multiply with Gain (Sensitivity)
            // gx = ((float)raw_gx) * SCALING_FACTOR;
            // gy = ((float)raw_gy) * SCALING_FACTOR;
            // gz = ((float)raw_gz) * SCALING_FACTOR;
            scaleData(rawData);

            // Applying Simple low-pass filter
            // filtered_gx = FILTER_COEFFICIENT * gx + (1 - FILTER_COEFFICIENT) * filtered_gx;
            // filtered_gy = FILTER_COEFFICIENT * gy + (1 - FILTER_COEFFICIENT) * filtered_gy;
            // filtered_gz = FILTER_COEFFICIENT * gz + (1 - FILTER_COEFFICIENT) * filtered_gz;

            // Applying simple high-pass filter with the lpf (by eliminating low freq elements)
            // high_pass_gx = gx - filtered_gx;
            // high_pass_gy = gy - filtered_gy;
            // high_pass_gz = gz - filtered_gz;
            filterData(scaledData);

            printf(">x_axis_high:%4.5f|g\n", high_pass_gx);
            printf(">y_axis_high:%4.5f|g\n", high_pass_gy);
            printf(">z_axis_high:%4.5f|g\n", high_pass_gz);


            // Record data samples in the array according to their axes
            final_x_data[i] = filteredData->filtered_x;
            final_y_data[i] = filteredData->filtered_y;
            final_z_data[i] = filteredData->filtered_z;
        


            //-----Conversion of Angular Velocities to the Linenar Velocity and Calculating the Distance----//
            //Z axis is used as the device will be calibrated along the Z-axis
            Velocity[i] = calculateLinearVelocity(final_z_data[i]); // Function to calculate the instantaneous velocity
            calculateLinearDistance(Velocity[i], &Distance); // Function to calculate the instantaneous Distance


            //increment array 
            i++;

            //sleep for 500ms as we're reading the data for every 0.5 seconds
            thread_sleep_for(500);
            
        }
        //Multiplying the Distance value with ERROR_SCALING_FACTOR to minimize the error rate.
        float finalDistance = Distance * ERROR_SCALING_FACTOR
        
        printf("Distance: %.2f\n", finalDistance);
        led.write(0);

        // Printing the data to LCD Screen
        clearData(); // clearing the data in the line 8         
        writeDataVariable(1, 60, finalDistance, 9);
        writeDataVariable(2, 60, averageVelocity, 10);


        // snprintf(display_buf[0], 60, "Distance Travelled:");
        // snprintf(display_buf[1], 60, "%.2f metres", finalDistance);
        // lcd.SelectLayer(FOREGROUND);
        // lcd.DisplayStringAt(5, LINE(8), (uint8_t *)display_buf[0], CENTER_MODE);
        // lcd.DisplayStringAt(5, LINE(9), (uint8_t *)display_buf[1], CENTER_MODE);
        
    }

}
