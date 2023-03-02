#pragma once

#include <algorithm>
#include <cstdint>
#include <deque>
#include <optional>
#include <set>
#include <unordered_map>
#include <unordered_set>

#include "mathutil.hpp"
#include "world.hpp"

struct point
{
    int x, y;
    bool operator==(const point& rhs) const
    {
        return x == rhs.x && y == rhs.y;
    }
    point left() const { return {x - 1, y}; }
    point right() const { return {x + 1, y}; }
    point up() const { return {x, y - 1}; }
    point down() const { return {x, y + 1}; }
};
namespace std
{
template <>
struct hash<point>
{
    size_t operator()(const point& p) const
    {
        return p.x + (p.y << 16);
    }
};
} // namespace std

struct tile_info
{
    int dist = INT_MAX;
    std::optional<point> prev;
};

struct score_map : private std::unordered_map<point, int>
{
    int get(const point& p)
    {
        if (auto it = find(p); it != end())
        {
            return it->second;
        }
        else
        {
            return INT_MAX - 10000;
        }
    }

    void set(const point& p, int val)
    {
        operator[](p) = val;
    }
};

using path_map = std::unordered_map<point, point>;
using path_seq = std::deque<point>;

inline path_seq reconstruct_path(const path_map& came_from, point current)
{
    path_seq total_path{current};
    path_map::const_iterator it;
    for (;;)
    {
        it = came_from.find(current);
        if (it == came_from.end())
            break;
        current = it->second;
        total_path.push_front(current);
    }
    return total_path;
}

inline path_seq find_path(world& wor, int x0, int y0, int x1, int y1)
{
    int width = (int)wor.tiles.get_width();

    point start{x0, y0};
    point end{x1, y1};

    std::unordered_set<point> frontier_set;
    std::vector<point> frontier;
    frontier.push_back(start);

    std::unordered_map<point, point> came_from;
    score_map g_score;
    g_score.set(start, 0);
    score_map f_score;
    f_score.set(start, 0);

    auto check_adjacent_point = [&](const point& from, const point& to) {
        if (!wor.tiles.is_valid_index(to.x, to.y))
        {
            return;
        }

        // d(current, neighbor) is always 1
        int tentative_g_score = g_score.get(from) + 1;
        if (tentative_g_score < g_score.get(to))
        {
            came_from[to] = from;
            g_score.set(to, tentative_g_score);
            f_score.set(to, tentative_g_score + manhattan(to.x, to.y, end.x, end.y));

            if (!frontier_set.count(to))
            {
                frontier.push_back(to);
                frontier_set.insert(to);
                std::push_heap(frontier.begin(), frontier.end(), [&](const point& left, const point& right) {
                    return f_score.get(left) > f_score.get(right);
                });
            }
        }
    };

    while (frontier.size())
    {
        // std::sort(frontier.begin(), frontier.end(), [&](const point& left, const point& right) {
        //     return f_score.get(left) > f_score.get(right);
        // });

        point current = frontier.front();
        std::pop_heap(frontier.begin(), frontier.end(), [&](const point& left, const point& right) {
            return f_score.get(left) > f_score.get(right);
        });
        frontier.pop_back();

        if (current == end)
        {
            return reconstruct_path(came_from, current);
        }

        // inspect adjacent tiles
        point L = current.left();
        check_adjacent_point(current, L);

        point R = current.right();
        check_adjacent_point(current, R);

        point U = current.up();
        check_adjacent_point(current, U);

        point D = current.down();
        check_adjacent_point(current, D);
    }

    // should probably log a warning here or something
    assert(false);
    return {};
}