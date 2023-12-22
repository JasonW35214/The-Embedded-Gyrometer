#include "mbed.h"
#include "math.h"

#define PI 3.141592653589793238462643383279502884L 
#define RADIUS 0.5
#define dt 0.5

float averageVelocity = 0.0f;

float calculateLinearVelocity(float omega){
    float omega_mps, linearVelocity; 
    // omega_mps = omega * (PI / 180); // IF the units of angular velocity are in degrees/second
    // linearVelocity = RADIUS * omega_mps;
    linearVelocity = RADIUS * omega;
    averageVelocity += linearVelocity; 
    return linearVelocity;
}

void calculateLinearDistance(float linearVelocity, float *DISTANCE){
    float instantaneousDistance = abs(linearVelocity) * dt;
    *DISTANCE += instantaneousDistance;    
}
