#include "mbed.h"

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


typedef struct {
    float x_raw;
    float y_raw;
    float z_raw;
} GyroscopeRawData;

typedef struct {
    float scaled_x;
    float scaled_y;
    float scaled_z;
} GyroscopeScaled;

typedef struct {
    float filtered_x;
    float filtered_y;
    float filtered_z;
} GyroscopeFiltered;


void setupWriteRegister();
void setSPIformat();
void readData(GyroscopeRawData *rawData);
void scaleData(GyroscopeRawData *rawData);
void filterData(GyroscopeScaled *scaledData);


void button_pushed_cb();
void data_cb();
void spi_cb();

