# Embedded Gyrometer as Speedomoter

# Required Parts
1. STM32F429 Discovery Board with built in gyroscope
2. Power supply or USB power bank
3. A way to store 20 seconds of velocity data, sampled at 0.5 second intervals or lower

# Objective
1. Attach the STM32F429 and power supply to any suitable part of your body
2. Interface the gyro using SPI to capture the angular velocities
3. Write the code to sample the angular velocities every 0.5s or lower.
4. Convert those measurements to linear forward velocity.
5. Record 20 seconds worth of data that can be exported through the USB or extracted from memory somehow.
6. Calculate the overall distance traveled during the 20 seconds of measurements.

# Constraints
1. No other components may be used other than those specified above. 
2. The PlatformIO programming environment must be used.
3. You will be allowed to use any drivers/HAL functions available through the IDE
