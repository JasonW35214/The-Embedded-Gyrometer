#include "mbed.h"
#include "math.h"

#define PI 3.141592653589793238462643383279502884L 
#define RADIUS 0.5
#define dt 0.5

float averageVelocity = 0.0f;
float calculateLinearVelocity(float omega);
void calculateLinearDistance(float linearVelocity, float *DISTANCE);



