#include "mbed.h"
#include "gyroscope.h"

// EventFlags object declaration
EventFlags flags;

// DigitalIn button(PA_0);
InterruptIn button(BUTTON1);

DigitalOut led(LED1);

//spi initialization
SPI spi(PF_9, PF_8, PF_7, PC_1, use_gpio_ssel);
uint8_t write_buf[32], read_buf[32];


// spi callback function
void spi_cb(int event) {
    flags.set(SPI_FLAG);
}

//spi format and frequency
void setSPIformat(){
    spi.format(8, 3);
    spi.frequency(1'000'000);
}


// Write to control registers --> spi transfer
void setupWriteRegister(uint8_t CTRL_REG, uint16_t CTRL_REG_CONFIG){
    write_buf[0] = CTRL_REG;
    write_buf[1] = CTRL_REG_CONFIG;
    // spi.transfer(write_buf, 2, read_buf, 2, spi_cb);
    

    // write_buf[0] = CTRL_REG1;
    // write_buf[1] = CTRL_REG1_CONFIG;
    // spi.transfer(write_buf, 2, read_buf, 2, spi_cb);
    // flags.wait_all(SPI_FLAG);

    // write_buf[0] = CTRL_REG4;
    // write_buf[1] = CTRL_REG4_CONFIG;
    // spi.transfer(write_buf, 2, read_buf, 2, spi_cb);
    // flags.wait_all(SPI_FLAG);

    // write_buf[0] = CTRL_REG3;
    // write_buf[1] = CTRL_REG3_CONFIG;
    // spi.transfer(write_buf, 2, read_buf, 2, spi_cb);
    // flags.wait_all(SPI_FLAG);

    // write_buf[1] = 0xFF;
}


// button pushed callback function
void button_pushed_cb(){
    led = !led;
    flags.set(BUTTON_PRESSED_FLAG);
}

// data ready callback function
void data_cb() {
    flags.set(DATA_READY_FLAG);
}

GyroscopeRawData *gyroRawData;
GyroscopeScaled *scaledData;
GyroscopeFiltered *filteredData;



void readData(GyroscopeRawData *rawData){
    rawData->x_raw = (((uint16_t)read_buf[2]) << 8) | ((uint16_t)read_buf[1]);
    rawData->y_raw = (((uint16_t)read_buf[4]) << 8) | ((uint16_t)read_buf[3]);
    rawData->z_raw = (((uint16_t)read_buf[6]) << 8) | ((uint16_t)read_buf[5]);
}

void scaleData(GyroscopeRawData *rawData){
    scaledData->scaled_x = ((float)rawData->x_raw) * SCALING_FACTOR;
    scaledData->scaled_y = ((float)rawData->y_raw) * SCALING_FACTOR;
    scaledData->scaled_z = ((float)rawData->z_raw) * SCALING_FACTOR;
}

void filterData(GyroscopeScaled *scaledData){
    float filtered_gx, filtered_gy, filtered_gz;
    filtered_gx = FILTER_COEFFICIENT * scaledData->scaled_x + (1 - FILTER_COEFFICIENT) * filtered_gx;
    filtered_gy = FILTER_COEFFICIENT * scaledData->scaled_y + (1 - FILTER_COEFFICIENT) * filtered_gy;
    filtered_gz = FILTER_COEFFICIENT * scaledData->scaled_z + (1 - FILTER_COEFFICIENT) * filtered_gz;

    // Applying simple high-pass filter with the lpf (by eliminating low freq elements)
    filteredData->filtered_x = scaledData->scaled_x - filtered_gx;
    filteredData->filtered_y = scaledData->scaled_y - filtered_gy;  
    filteredData->filtered_z = scaledData->scaled_z - filtered_gz;  
}

void checkPolling(){
    if (!(flags.get() & DATA_READY_FLAG) && (int2.read() == 1)) {
        flags.set(DATA_READY_FLAG);
    }
}

