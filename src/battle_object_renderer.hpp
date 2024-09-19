#pragma once

#include <glm/mat4x4.hpp>
#include <vector>

#include "shader.hpp"

struct texture;
struct rectangle;

struct battle_object_vertex
{
    float x, y, z;
    float u, v;
    float color_mult;
    float flash;
};

// implements 2.5d rendering of objects
class battle_object_renderer
{
public:
    battle_object_renderer();

    void draw_quad(const texture* tex, const rectangle& src, float x, float y, float w, float h, bool flash, bool flip_x = false, float color_mult = 1.f);
    void draw_quad_rotated(const texture* tex, const rectangle& src, float x, float y, float w, float h, float angle, bool flash);
    void begin(const glm::mat4& view, const glm::mat4& projection);
    void end();

private:
    shader_program prog;

    std::vector<battle_object_vertex> batch;

    GLuint vao;
    GLuint buffer;

    GLint uView;
    GLint uProjection;
};