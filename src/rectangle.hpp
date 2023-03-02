#pragma once

#include <SDL_rect.h>
#include <algorithm>

// TODO: templates I guess, don't have time right now tho
struct rectanglef {
    float x, y, w, h;
};

struct rectangle {
    int x, y, w, h;

    int left() const { return x; }
    int top() const { return y; }
    int right() const { return x + w; }
    int bottom() const { return y + h; }

    void align_left(int to) { x = to; }
    void align_right(int to) { x = to - w; }
    void align_top(int to) { y = to; }
    void align_bottom(int to) { y = to - h; }

    void center_on(int on_x, int on_y) {
        x = on_x - w / 2;
        y = on_y - h / 2;
    }

    void translate(int dx, int dy) {
        x += dx;
        y += dy;
    }

    bool contains(int px, int py) const {
        return px >= left() && px < right() && py >= top() && py <= bottom();
    }

    void inflate(int dx, int dy) {
        x -= dx;
        w += 2 * dx;
        y -= dy;
        h += 2 * dy;
    }

    bool intersect(const rectangle& rhs, rectangle& subrect) {
        int xmin = std::max(x, rhs.x);
        int ymin = std::max(y, rhs.y);
        int xmax = std::min(right(), rhs.right());
        int ymax = std::min(bottom(), rhs.bottom());
        if (xmax < xmin || ymax < ymin) {
            return false;
        } else {
            subrect = { xmin, ymin, xmax - xmin, ymax - ymin };
            return true;
        }
    }
};

inline SDL_Rect to_sdl_rect(const rectangle& rect) {
    return SDL_Rect{rect.x, rect.y, rect.w, rect.h};
}


