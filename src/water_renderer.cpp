#include "water_renderer.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "texture_manager.hpp"

constexpr auto vssrc = R"(#version 330 core
layout (location = 0) in vec4 aPosTex;
layout (location = 1) in vec2 aWorldPos;

uniform mat4 uTransform;

out vec2 fPos;
out vec2 fTex;

void main() {
    gl_Position = uTransform * vec4(aPosTex.xy, 0.0, 1.0);
    fTex = aPosTex.zw;
    fPos = aWorldPos.xy;
}
)";

constexpr auto fssrc = R"(#version 330 core

in vec2 fPos;
in vec2 fTex;

uniform sampler2D uSamplerBase;
uniform sampler2D uSamplerBlend;
uniform float     uBlendAmount;
uniform float     uGlobalTime;
uniform vec2      uWaterDirection;
uniform float     uWaterSpeed;
uniform vec2      uWaterDriftRange; // good default is <0.3, 0.2>, set to 0 for waterfalls
uniform vec2      uWaterDriftScale; // default to 32, 16

out vec4 FragColor;

void main() {
    vec2 invTexCoord = vec2(fTex.x, 1.0 - fTex.y);

    invTexCoord.x += cos(uGlobalTime + fPos.y / uWaterDriftScale.x) * uWaterDriftRange.x + uWaterDirection.x * uGlobalTime * uWaterSpeed;
    invTexCoord.y += sin(uGlobalTime + fPos.x / uWaterDriftScale.y) * uWaterDriftRange.y + uWaterDirection.y * uGlobalTime * uWaterSpeed;

    vec4 base_color  = texture(uSamplerBase, invTexCoord);
    vec4 blend_color = texture(uSamplerBlend, invTexCoord);
    FragColor = mix(base_color, blend_color, uBlendAmount);
}
)";

water_renderer::water_renderer()
{
    prog = create_program_from_source(vssrc, fssrc);

    uTransform = glGetUniformLocation(prog.get_handle(), "uTransform");
    uGlobalTime = glGetUniformLocation(prog.get_handle(), "uGlobalTime");
    uWaterDirection = glGetUniformLocation(prog.get_handle(), "uWaterDirection");
    uWaterSpeed = glGetUniformLocation(prog.get_handle(), "uWaterSpeed");
    uWaterDriftRange = glGetUniformLocation(prog.get_handle(), "uWaterDriftRange");
    uSamplerBase = glGetUniformLocation(prog.get_handle(), "uSamplerBase");
    uSamplerBlend = glGetUniformLocation(prog.get_handle(), "uSamplerBlend");
    uBlendAmount = glGetUniformLocation(prog.get_handle(), "uBlendAmount");
    uWaterDriftScale = glGetUniformLocation(prog.get_handle(), "uWaterDriftScale");

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &buffer);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, buffer);

    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(water_vertex), (void*)0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(water_vertex), (void*)16);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
}

void water_renderer::begin(const water_render_parameters& opts)
{
    glUseProgram(prog.get_handle());
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);

    glUniformMatrix4fv(uTransform, 1, GL_FALSE, glm::value_ptr(transform));
    glUniform1f(uGlobalTime, opts.global_time);
    glUniform2fv(uWaterDirection, 1, glm::value_ptr(opts.water_direction));
    glUniform1f(uWaterSpeed, opts.water_speed);
    glUniform2fv(uWaterDriftRange, 1, glm::value_ptr(opts.water_drift_range));
    glUniform1i(uSamplerBase, 0);
    glUniform1i(uSamplerBlend, 1);
    glUniform1f(uBlendAmount, opts.blend_amount);
    glUniform2fv(uWaterDriftScale, 1, glm::value_ptr(opts.water_drift_scale));
}

void water_renderer::end()
{
    glBufferData(GL_ARRAY_BUFFER, batch.size() * sizeof(water_vertex), batch.data(), GL_STREAM_DRAW);
    glDrawArrays(GL_TRIANGLES, 0, batch.size());
    batch.clear();
}

void water_renderer::draw_quad(const texture* tex, const rectangle& src, const rectangle& dest, float world_x, float world_y, float square_size)
{
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

    water_vertex vertices[] = {
        {right, top, uv_right, uv_top, world_x + square_size, world_y},
        {left, top, uv_left, uv_top, world_x, world_y},
        {left, bottom, uv_left, uv_bottom, world_x, world_y + square_size},

        {left, bottom, uv_left, uv_bottom, world_x, world_y + square_size},
        {right, bottom, uv_right, uv_bottom, world_x + square_size, world_y + square_size},
        {right, top, uv_right, uv_top, world_x + square_size, world_y}};

    // TODO: index this
    batch.push_back(vertices[0]);
    batch.push_back(vertices[1]);
    batch.push_back(vertices[2]);
    batch.push_back(vertices[3]);
    batch.push_back(vertices[4]);
    batch.push_back(vertices[5]);
}

void water_renderer::set_output_dimensions(int w, int h)
{
    transform = glm::ortho<float>(0, w, h, 0);
}
