#include <cstdlib>
#include <string>
#include <sstream>

int random(int min, int max) {
    if (max <= min) return min;
    return min + std::rand() % (max - min);
}

int random(int max) {
    if (max <= 0) return 0;
    return std::rand() % max;
}

float constrain(float val, float min, float max) {
    if (val < min) return min;
    if (val > max) return max;
    return val;
}