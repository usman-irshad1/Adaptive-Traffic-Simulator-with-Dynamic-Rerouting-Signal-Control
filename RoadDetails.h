#pragma once
#include <cmath>
#include <iostream>
#include <string>
#include "TrafficSignal.h"

using namespace std;

struct RoadDetails {
    float length, max_speed, capacity;
    int currentVehicles;
    int queueCount;
    bool signalState;
    float NonIdealWeight;
    float a, b;
    TrafficSignal light;

    int dischargeCapcity;
    bool operator<(const RoadDetails& other) const {
        return this->calculateWeight() < other.calculateWeight();
    }

    float calculateWeight() const {
        float best = length / max_speed;
        if (capacity <= 0)
            return 1000000; // Safety for division
        return best * (1.0f + a * pow((float)currentVehicles / capacity, b));
    }
    RoadDetails(float ab, float bc, float c, float aplha = 0.5, float beta = 4,
        int discharge = 5)
        : a(aplha), b(beta), dischargeCapcity(discharge) {
        currentVehicles = 0;
        length = ab;
        max_speed = bc;
        capacity = c;
        queueCount = 0;
        signalState = true;
    }
    float DischargeAllowed(int capacity_ofnext_road, int pop_of_next_road) {
        float allowed = queueCount;
        float avail = capacity_ofnext_road - pop_of_next_road;
        if (avail > allowed) {
            if (dischargeCapcity <= allowed) {
                return signalState * dischargeCapcity;
            }
            else {
                return signalState * queueCount;
            }
        }
        else if (dischargeCapcity > avail) {
            return avail * signalState;
        }
        else {
            return dischargeCapcity * signalState;
        }
    }
    float TimeCal() {
        if (capacity <= 0) {
            return bestTime();
        }

        NonIdealWeight = NonIdealtime();
        return NonIdealWeight;
    }

    float bestTime() { return length / max_speed; }

    float NonIdealtime() {
        if (capacity <= 0) {
            return bestTime();
        }
        NonIdealWeight = bestTime();
        NonIdealWeight =
            NonIdealWeight * (1 + a * pow((float)currentVehicles / capacity, b));
        return NonIdealWeight;
    }
    void vehicleEntersRoad() {
        if (currentVehicles < capacity) {
            currentVehicles++;
            NonIdealtime();
        }
    }
    void vehicleExitsRoad() {
        if (currentVehicles > 0) {
            currentVehicles--;
            NonIdealtime();
        }
    }

    void updateVehicles() {
        if (queueCount < capacity) 
            queueCount++;
    }

    void vehicleJoinsQueue() {
        queueCount++;
        NonIdealtime();
    }
    void vehicleExitsQueue() {
        if (queueCount > 0) {
            queueCount--;
        }
    }
    void decVehicles() {
        if (queueCount >
            0) 
            queueCount--;
    }

    float Congestion() const {
        if (capacity <= 0) {
            return 0;
        }
        return (float)currentVehicles / capacity;
    }

    void Time(int time) { light.Timer(signalState, time); }

    void changeState() {
        if (signalState) {
            light.turnRed(signalState);
        }
        else {
            light.turnGreen(signalState);
        }
    }
    void change_to_red() { light.turnRed(signalState); }
    void increment(int time) { light.Timer(signalState, time); }

    float choosing() {
        float currentCost =
            (light.pa * queueCount) + (light.pb * pow(Congestion(), 2));
        return currentCost;
    }
};
