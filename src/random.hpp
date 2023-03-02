#pragma once

#include <random>
#include <ctime>

struct random {
public:
  random();

  static int rand_int(int min, int max);
  static float rand_real();
  static float rand_real(float min, float max);

  static bool chance(float rate);

private:
  static random& instance();

  //std::random_device dev;
  std::mt19937 dev{ time(0) };
};
