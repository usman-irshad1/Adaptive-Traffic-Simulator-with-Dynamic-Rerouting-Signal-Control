#pragma once

struct TrafficSignal {
    float greentimer;
    float minGreenTime;
    float queueThreshold;
    float maxGreenTime;
    float redtimer;
    float pa, pb;
    TrafficSignal() {
        greentimer = 0;
        minGreenTime = 2; 
        queueThreshold = 3;
        maxGreenTime = 8;
        pa = 10;
        pb = 7; 
        redtimer = 0;
    }

    void Timer(bool state, float time) {
        if (state == true) {
            greentimer += time;
            redtimer = 0;
        }
        else {
            redtimer += time;
        }
    }
    float starvationCost() const { return redtimer * 3.5f; }
    void turnGreen(bool& state) {
        state = true;
        greentimer = 0;
    }
    void turnRed(bool& state) {
        state = false;
        redtimer = 0;
    }

    bool canSwitch(int currentQueue, int maxOtherQueue) const {
        if (greentimer < minGreenTime)
            return false;

        if (mustSwitch()) {
            return true;
        }

        float bottleneckRatio = (float)currentQueue / maxOtherQueue;
        if (bottleneckRatio > 3.0f) {  
            return false;  
        }

        return (currentQueue < maxOtherQueue) || (starvationCost() > 12.0f);
    }
    bool mustSwitch() const { return greentimer >= maxGreenTime; }
};