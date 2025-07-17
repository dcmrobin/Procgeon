#pragma once
#include <iostream>

class LoopGuard {
    int count;
    int limit;
    const char* label;
public:
    LoopGuard(int maxIters, const char* name) : count(0), limit(maxIters), label(name) {}
    void tick() {
        if (++count == limit) {
            std::cout << "[ALERT] Loop '" << label << "' exceeded " << limit << " iterations!" << std::endl;
        }
    }
};
