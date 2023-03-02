#pragma once

#include <glm/mat4x4.hpp>
#include <vector>

#include "rectangle.hpp"
#include "shader.hpp"

struct texture;

struct spritebatch_vertex
{
    float x, y, u, v;
    float r, g, b, a;
};

class spritebatch
{
public:
    spritebatch();

    void set_output_dimensions(int w, int h);

    void begin();
    void end();
    void flush();

    void draw_quad(const texture* tex, const rectangle& src, const rectangle& dest);
    void draw_quad(const texture* tex, const rectangle& src, const rectangle& dest, float r, float g, float b, float a = 1.0f);

    void draw_tiled_quad(const texture* tex, const rectangle& dest, float rx, float ry);

protected:
    spritebatch(std::string_view vssrc, std::string_view fssrc);

    shader_program prog;

private:
    std::vector<spritebatch_vertex> batch;

    GLuint vao;
    GLuint buffer;

    glm::mat4 transform;

    GLint uTransform;
};