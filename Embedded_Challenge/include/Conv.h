#include "mbed.h"
#include "math.h"
#include "vector.h"

#define PI acos(-1.0)
#define RADIUS 0.5
#define dt 0.5

float calculateLinearVelocity(float omega){
    float omega_mps, linearVelocity; 
    // omega_mps = omega * (PI / 180); // IF the uints of angular velocity are in degrees/second
    // linearVelocity = RADIUS * omega_mps;
    linearVelocity = RADIUS * omega;

    return linearVelocity;
}

void calculateLinearDistance(float linearVelocity, float *DISTANCE){
    std::vector<flaot> instanteousDistance[40];
    float instDist = abs(linearVelocity) * dt;  
    instantaneousDistance.push_back(instDist);
    *DISTANCE += instDistance;    
}
