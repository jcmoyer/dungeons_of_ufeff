#pragma once

enum direction {
    right, up, left, down
};

inline direction invert(direction d) {
    switch (d) {
    case right: return left;
    case left: return right;
    case up: return down;
    case down: return up;
    default:
        assert(0 && "bad direction value");
        return right;
    }
}
