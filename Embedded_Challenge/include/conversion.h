#include "mbed.h"
#include "math.h"
#include "vector.h"

#define PI 3.141592653589793238462643383279502884L 
#define RADIUS 0.5
#define dt 0.5

std::vector<float> instanteousDistance[40];
std::vector<float> velocity[40];

float calculateLinearVelocity(float omega){
    float omega_mps, linearVelocity; 
    // omega_mps = omega * (PI / 180); // IF the units of angular velocity are in degrees/second
    // linearVelocity = RADIUS * omega_mps;
    linearVelocity = RADIUS * omega;
    velocity.push_back(linearVelocity);

    return linearVelocity;
}

void calculateLinearDistance(float linearVelocity, float *DISTANCE){
    float instDist = abs(linearVelocity) * dt;  
    instantaneousDistance.push_back(instDist);
    *DISTANCE += instDistance;    
}
