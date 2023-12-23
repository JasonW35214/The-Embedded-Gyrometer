#include "mbed.h"
#include "conversion.h"

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
