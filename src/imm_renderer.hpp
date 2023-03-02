#pragma once

#include "shader.hpp"
#include <glm/mat4x4.hpp>
#include "rectangle.hpp"

struct texture;

class imm_renderer {
public:
    imm_renderer();

    void set_output_dimensions(int w, int h);

    void begin();

    void draw_quad(int x, int y, int w, int h);
    void draw_quad(int x, int y, int w, int h, float r, float g, float b);

    void draw_quad(const texture* tex, const rectangle& src, const rectangle& dest);

private:
    shader_program prog;
    GLuint vao;
    GLuint buffer;

    glm::mat4 transform;
};