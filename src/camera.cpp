#include "camera.hpp"

void camera::set_bounds(const rectangle& b) {
    b_left = b.x;
    b_top = b.y;
    b_right = b.x + b.w;
    b_bottom = b.y + b.h;
}

void camera::set_bounds(int left, int top, int right, int bottom) {
    b_left = left;
    b_top = top;
    b_right = right;
    b_bottom = bottom;
    clamp_to_bounds();
}
