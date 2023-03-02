#include "battle_object_renderer.hpp"

#include <GL/gl3w.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "rectangle.hpp"
#include "texture_manager.hpp"

constexpr auto default_vssrc = R"(#version 330 core
layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in vec2 aColorFlash;

uniform mat4 uView;
uniform mat4 uProjection;

out vec2 fTexCoord;
out float fFlash;
out float fColorMult;

void main() {
    gl_Position = uProjection * uView * vec4(aPosition, 1.0);
    fTexCoord   = aTexCoord;
    fColorMult  = aColorFlash.x;
    fFlash      = aColorFlash.y;
}
)";

constexpr auto default_fssrc = R"(#version 330 core

in vec2 fTexCoord;
in float fFlash;
in float fColorMult;

uniform sampler2D uSampler;

out vec4 FragColor;

void main() {
    vec2 invTexCoord = vec2(fTexCoord.x, 1.0 - fTexCoord.y);
    vec4 color = texture(uSampler, invTexCoord);

    if (color.a <= 0.5) {
        discard;
    }

    color.rgb *= fColorMult;

    FragColor = mix(
        color,
        vec4(1.0, 1.0, 1.0, 1.0),
        fFlash
    );
}
)";

battle_object_renderer::battle_object_renderer()
{
    prog = create_program_from_source(default_vssrc, default_fssrc);

    uView = glGetUniformLocation(prog.get_handle(), "uView");
    uProjection = glGetUniformLocation(prog.get_handle(), "uProjection");

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &buffer);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, buffer);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(battle_object_vertex), (void*)0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(battle_object_vertex), (void*)12);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(battle_object_vertex), (void*)20);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
}

void battle_object_renderer::draw_quad(const texture* tex, const rectangle& src, float x, float y, float w, float h, bool flash, bool flip_x, float color_mult)
{
    int s_left = src.x;
    int s_right = src.x + src.w;
    int s_top = src.y;
    int s_bottom = src.y + src.h;

    float uv_left = s_left / (float)tex->width;
    float uv_right = s_right / (float)tex->width;
    float uv_top = s_top / (float)tex->height;
    float uv_bottom = s_bottom / (float)tex->height;

    if (flip_x)
    {
        std::swap(uv_left, uv_right);
    }

    rectanglef dest;
    dest.x = x;
    dest.y = y;
    dest.w = w;
    dest.h = h;

    float flash_x = flash ? 1.f : 0.f;

    // we flip the Y coordinates here because Y is up
    // I still dont' understand this garbage, TODO: research
    battle_object_vertex vertices[] = {
        {dest.x + dest.w, dest.y + dest.h, 0, uv_right, uv_top, color_mult, flash_x},
        {dest.x, dest.y + dest.h, 0, uv_left, uv_top, color_mult, flash_x},
        {dest.x, dest.y, 0, uv_left, uv_bottom, color_mult, flash_x},

        {dest.x, dest.y, 0, uv_left, uv_bottom, color_mult, flash_x},
        {dest.x + dest.w, dest.y, 0, uv_right, uv_bottom, color_mult, flash_x},
        {dest.x + dest.w, dest.y + dest.h, 0, uv_right, uv_top, color_mult, flash_x},
    };

    // TODO: index this
    batch.push_back(vertices[0]);
    batch.push_back(vertices[1]);
    batch.push_back(vertices[2]);
    batch.push_back(vertices[3]);
    batch.push_back(vertices[4]);
    batch.push_back(vertices[5]);
}

#include "glm/gtx/rotate_vector.hpp"

void battle_object_renderer::draw_quad_rotated(const texture* tex, const rectangle& src, float x, float y, float w, float h, float angle, bool flash)
{
    int s_left = src.x;
    int s_right = src.x + src.w;
    int s_top = src.y;
    int s_bottom = src.y + src.h;

    float uv_left = s_left / (float)tex->width;
    float uv_right = s_right / (float)tex->width;
    float uv_top = s_top / (float)tex->height;
    float uv_bottom = s_bottom / (float)tex->height;

    rectanglef dest;
    dest.x = -w / 2.f;
    dest.y = -h / 2.f;
    dest.w = w;
    dest.h = h;

    glm::vec2 top_left{-w / 2.f, -h / 2.f};
    glm::vec2 top_right{w / 2.f, -h / 2.f};
    glm::vec2 bottom_left{-w / 2.f, h / 2.f};
    glm::vec2 bottom_right{w / 2.f, h / 2.f};
    top_left = glm::rotate(top_left, angle);
    top_right = glm::rotate(top_right, angle);
    bottom_left = glm::rotate(bottom_left, angle);
    bottom_right = glm::rotate(bottom_right, angle);

    const float dest_center_x = x + w / 2.f;
    const float dest_center_y = y + h / 2.f;

    top_left.x += dest_center_x;
    top_left.y += dest_center_y;
    top_right.x += dest_center_x;
    top_right.y += dest_center_y;
    bottom_left.x += dest_center_x;
    bottom_left.y += dest_center_y;
    bottom_right.x += dest_center_x;
    bottom_right.y += dest_center_y;

    float flash_x = flash ? 1.f : 0.f;

    // we flip the Y coordinates here because Y is up
    // I still dont' understand this garbage, TODO: research
    battle_object_vertex vertices[] = {
        {bottom_right.x, bottom_right.y, 0, uv_right, uv_top, 1, flash_x},
        {bottom_left.x, bottom_left.y, 0, uv_left, uv_top, 1, flash_x},
        {top_left.x, top_left.y, 0, uv_left, uv_bottom, 1, flash_x},
        {top_left.x, top_left.y, 0, uv_left, uv_bottom, 1, flash_x},
        {top_right.x, top_right.y, 0, uv_right, uv_bottom, 1, flash_x},
        {bottom_right.x, bottom_right.y, 0, uv_right, uv_top, 1, flash_x},
    };

    // TODO: index this
    batch.push_back(vertices[0]);
    batch.push_back(vertices[1]);
    batch.push_back(vertices[2]);
    batch.push_back(vertices[3]);
    batch.push_back(vertices[4]);
    batch.push_back(vertices[5]);
}

void battle_object_renderer::begin(const glm::mat4& view, const glm::mat4& projection)
{
    glUseProgram(prog.get_handle());
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);

    glUniformMatrix4fv(uView, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(uProjection, 1, GL_FALSE, glm::value_ptr(projection));
}

void battle_object_renderer::end()
{
    glBufferData(GL_ARRAY_BUFFER, batch.size() * sizeof(battle_object_vertex), batch.data(), GL_STREAM_DRAW);
    glDrawArrays(GL_TRIANGLES, 0, (GLsizei)batch.size());
    batch.clear();
}
