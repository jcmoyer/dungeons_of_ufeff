#pragma once

#include <glm/mat4x4.hpp>

#include "rectangle.hpp"
#include "shader.hpp"

struct texture;

class screen_renderer
{
public:
    screen_renderer();

    void set_output_dimensions(int w, int h);

    void begin(float mask_effect);

    void draw_quad(const rectangle& dest);

private:
    shader_program prog;
    GLuint vao;
    GLuint buffer;

    glm::mat4 transform;

    GLint uTransform;
    GLint uSampler;
    GLint uMask;
    GLint uMaskEffect;
};
