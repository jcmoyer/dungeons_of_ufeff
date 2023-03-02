#pragma once

#include <glm/vec2.hpp>

#include "random.hpp"

inline glm::vec2 rand_vec2(float min, float max)
{
    return {random::rand_real(min, max), random::rand_real(min, max)};
}

inline glm::vec2 rand_vec2(float minx, float maxx, float miny, float maxy)
{
    return {random::rand_real(minx, maxx), random::rand_real(miny, maxy)};
}

inline glm::vec2 rand_vec2_x(float minx, float maxx)
{
    return {random::rand_real(minx, maxx), 0};
}