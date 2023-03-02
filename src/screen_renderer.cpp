#include "screen_renderer.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "texture_manager.hpp"

constexpr auto vssrc = R"(#version 330 core
layout (location = 0) in vec4 aPosTex;

uniform mat4 uTransform;

out vec2 fTex;

void main() {
    gl_Position = uTransform * vec4(aPosTex.xy, 0.0, 1.0);
    fTex = aPosTex.zw;
}
)";

constexpr auto fssrc = R"(#version 330 core

in vec2 fTex;

uniform sampler2D uSampler;
uniform sampler2D uMask;
uniform float     uMaskEffect;  

out vec4 FragColor;

void main() {
    vec2 invTexCoord = vec2(fTex.x, 1.0 - fTex.y);
    float maskValue = texture(uMask, invTexCoord).r;
    FragColor = texture(uSampler, invTexCoord) * mix(1, maskValue, uMaskEffect);
    FragColor.a = 1.0;
}
)";

struct screen_vertex
{
    float x, y, u, v;
};

screen_renderer::screen_renderer()
{
    prog = create_program_from_source(vssrc, fssrc);

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &buffer);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, buffer);

    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(screen_vertex), (void*)0);
    glEnableVertexAttribArray(0);

    uTransform = glGetUniformLocation(prog.get_handle(), "uTransform");
    uSampler = glGetUniformLocation(prog.get_handle(), "uSampler");
    uMask = glGetUniformLocation(prog.get_handle(), "uMask");
    uMaskEffect = glGetUniformLocation(prog.get_handle(), "uMaskEffect");
}

void screen_renderer::draw_quad(const rectangle& dest)
{
    const float left = static_cast<float>(dest.left());
    const float top = static_cast<float>(dest.top());
    const float right = static_cast<float>(dest.right());
    const float bottom = static_cast<float>(dest.bottom());

    screen_vertex vertices[] = {
        {left, top, 0, 0},
        {left, bottom, 0, 1},
        {right, bottom, 1, 1},
        {right, top, 1, 0}};

    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STREAM_DRAW);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

void screen_renderer::begin(float mask_effect)
{
    glUseProgram(prog.get_handle());
    glBindVertexArray(vao);

    glUniformMatrix4fv(uTransform, 1, GL_FALSE, glm::value_ptr(transform));
    glUniform1i(uSampler, 0);
    glUniform1i(uMask, 1);
    glUniform1f(uMaskEffect, mask_effect);
}

void screen_renderer::set_output_dimensions(int w, int h)
{
    transform = glm::ortho<float>(0, (float)w, (float)h, 0);
}
