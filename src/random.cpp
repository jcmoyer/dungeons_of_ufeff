#include "random.hpp"

// GLOBAL
random inst;

random::random()
{
}

random& random::instance()
{
    return inst;
}

int random::rand_int(int min, int max)
{
    std::uniform_int_distribution<int> dist{min, max};
    return dist(inst.dev);
}

float random::rand_real()
{
    std::uniform_real_distribution<float> dist{};
    return dist(inst.dev);
}

float random::rand_real(float min, float max)
{
    std::uniform_real_distribution<float> dist{min, max};
    return dist(inst.dev);
}

bool random::chance(float rate)
{
    return rand_real() <= rate;
}