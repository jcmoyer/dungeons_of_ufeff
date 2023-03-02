#pragma once

#include <cstdint>
#include <glm/mat4x4.hpp>
#include <vector>

#include "rectangle.hpp"
#include "shader.hpp"

struct texture;

struct quad_vertex
{
    float x, y;
    float r, g, b, a;
};

// why yes this is also a copy of spritebatch, how did you guess?
class quad_renderer
{
public:
    quad_renderer();

    void set_output_dimensions(int w, int h);

    void begin();
    void end();

    void draw_quad(const rectangle& dest);
    void draw_quad(const rectangle& dest, float r, float g, float b, float a);
    void draw_quad(const rectangle& dest, uint32_t rgba);

protected:
    quad_renderer(std::string_view vssrc, std::string_view fssrc);

    shader_program prog;

private:
    std::vector<quad_vertex> batch;

    GLuint vao;
    GLuint buffer;

    glm::mat4 transform;
};