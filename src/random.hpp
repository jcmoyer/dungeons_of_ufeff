#pragma once

#include <ctime>
#include <random>

struct random
{
public:
    random();

    static int rand_int(int min, int max);
    static float rand_real();
    static float rand_real(float min, float max);

    static bool chance(float rate);

private:
    static random& instance();

    //std::random_device dev;
    std::mt19937 dev{(std::mt19937::result_type)time(0)};
};
