#pragma once

#include <algorithm>
#include <cmath>

struct InterpolatedPosition {
    float x;
    float y;

    float oldX;
    float oldY;
    float targetX;
    float targetY;

    float time;
    float interpolateTime;

    InterpolatedPosition(float x, float y, float interpolateTime) : x(x), y(y), oldX(x), oldY(y), targetX(x), targetY(y), time(0), interpolateTime(interpolateTime) {}

    void set(float newX, float newY) {
        oldX = x;
        oldY = y;
        targetX = newX;
        targetY = newY;
        time = 0;
    }

    void update(float dt) {
        time += dt;
        time = std::clamp(time, 0.f, interpolateTime);

        x = std::lerp(oldX, targetX, time / interpolateTime);
        y = std::lerp(oldY, targetY, time / interpolateTime);
    }
};
