#pragma once

#include "rectangle.hpp"

class camera {
public:
    camera() {}

    camera(const camera&) = default;
    camera& operator=(const camera&) = default;

    void set_x(int to) { view.x = to; }
    void set_y(int to) { view.y = to; }
    void set_width(int to) { view.w = to; }
    void set_height(int to) { view.h = to; }

    void move(int dx, int dy) { view.translate(dx, dy); }

    void set_bounds(const rectangle& b);
    void set_bounds(int left, int top, int right, int bottom);

    int left() const { return view.left(); }
    int top() const { return view.top(); }
    int right() const { return view.right(); }
    int bottom() const { return view.bottom(); }

    void align_left(int to) { view.align_left(to); }
    void align_right(int to) { view.align_right(to); }
    void align_top(int to) { view.align_top(to); }
    void align_bottom(int to) { view.align_bottom(to); }

    void center_on(int on_x, int on_y) {
        view.center_on(on_x, on_y);
    }

    void clamp_to_bounds() {
        if (left() < b_left) {
            align_left(b_left);
        }
        if (top() < b_top) {
            align_top(b_top);
        }
        if (right() > b_right) {
            align_right(b_right);
        }
        if (bottom() > b_bottom) {
            align_bottom(b_bottom);
        }
    }

    int bound_left() { return b_left; }
    int bound_top() { return b_top; }
    int bound_right() { return b_right; }
    int bound_bottom() { return b_bottom; }

    const rectangle& get_view() const { return view; }
    rectangle get_bounds() const { return { b_left, b_top, b_right - b_left, b_bottom - b_top }; }

private:
    rectangle view;
    int b_left, b_top, b_right, b_bottom;
};
