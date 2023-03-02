#include "imm_renderer.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "texture_manager.hpp"

constexpr auto vssrc = R"(#version 330 core
layout (location = 0) in vec4 aPosTex;
layout (location = 1) in vec3 aColor;

uniform mat4 uTransform;

out vec2 fTex;
out vec3 fColor;

void main() {
    gl_Position = uTransform * vec4(aPosTex.xy, 0.0, 1.0);
    fTex = aPosTex.zw;
    fColor = aColor;
}
)";

constexpr auto fssrc = R"(#version 330 core

in vec2 fTex;
in vec3 fColor;

uniform sampler2D uSampler;

out vec4 FragColor;

void main() {
    vec2 invTexCoord = vec2(fTex.x, 1.0 - fTex.y);
    FragColor = texture(uSampler, invTexCoord) * vec4(fColor.rgb, 1.0);
}
)";

struct imm_vertex
{
    float x, y, u, v;
    float r, g, b;
};

imm_renderer::imm_renderer()
{
    prog = create_program_from_source(vssrc, fssrc);

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &buffer);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, buffer);

    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(imm_vertex), (void*)0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(imm_vertex), (void*)16);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
}

void imm_renderer::draw_quad(int x, int y, int w, int h)
{
    draw_quad(x, y, w, h, 1, 1, 1);
}

void imm_renderer::draw_quad(int x, int y, int w, int h, float r, float g, float b)
{
    float left = static_cast<float>(x);
    float right = static_cast<float>(x + w);
    float top = static_cast<float>(y);
    float bottom = static_cast<float>(y + h);

    imm_vertex vertices[] = {
        {left, top, 0, 0, r, g, b},
        {left, bottom, 0, 1, r, g, b},
        {right, bottom, 1, 1, r, g, b},
        {right, top, 1, 0, r, g, b}};

    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STREAM_DRAW);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

void imm_renderer::draw_quad(const texture* tex, const rectangle& src, const rectangle& dest)
{
    float r = 1, g = 1, b = 1;

    int s_left = src.x;
    int s_right = src.x + src.w;
    int s_top = src.y;
    int s_bottom = src.y + src.h;

    float uv_left = s_left / (float)tex->width;
    float uv_right = s_right / (float)tex->width;
    float uv_top = s_top / (float)tex->height;
    float uv_bottom = s_bottom / (float)tex->height;

    float left = static_cast<float>(dest.x);
    float right = static_cast<float>(dest.x + dest.w);
    float top = static_cast<float>(dest.y);
    float bottom = static_cast<float>(dest.y + dest.h);

    imm_vertex vertices[] = {
        {right, top, uv_right, uv_top, r, g, b},
        {right, bottom, uv_right, uv_bottom, r, g, b},
        {left, top, uv_left, uv_top, r, g, b},
        {right, bottom, uv_right, uv_bottom, r, g, b},
        {left, bottom, uv_left, uv_bottom, r, g, b},
        {left, top, uv_left, uv_top, r, g, b}};

    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STREAM_DRAW);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void imm_renderer::begin()
{
    glUseProgram(prog.get_handle());
    glBindVertexArray(vao);

    auto uTransform = glGetUniformLocation(prog.get_handle(), "uTransform");
    glUniformMatrix4fv(uTransform, 1, GL_FALSE, glm::value_ptr(transform));
}

void imm_renderer::set_output_dimensions(int w, int h)
{
    transform = glm::ortho<float>(0, w, h, 0);
}
