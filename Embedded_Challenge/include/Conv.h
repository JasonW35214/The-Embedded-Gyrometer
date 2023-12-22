#include "mbed.h"
#include "math.h"

#define Pi acos(-1.0)
#define GYROSCOPE_ADDRESS 0x68  //rep
#define GYROSCOPE_DATA_REGISTER 0x43

// Converts angular velocity to linear velocity
// float R = radius of circle
// float A = angular velocity
float conv (float R, float A)
{
    float V;
    V = R*A; // Calculate linear velocity
    return V;
}

// Calculate distance travelled
// float V = linear velocity
// float *d = distance travelled
void dist (float V, float *d)
{
    *d += abs(V)*0.5;
    return;
}