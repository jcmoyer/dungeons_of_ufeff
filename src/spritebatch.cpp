#include "spritebatch.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "texture_manager.hpp"

constexpr auto default_vssrc = R"(#version 330 core
layout (location = 0) in vec4 aPosTex;
layout (location = 1) in vec4 aColor;

uniform mat4 uTransform;

out vec2 fTex;
out vec4 fColor;

void main() {
    gl_Position = uTransform * vec4(aPosTex.xy, 0.0, 1.0);
    fTex = aPosTex.zw;
    fColor = aColor;
}
)";

constexpr auto default_fssrc = R"(#version 330 core

in vec2 fTex;
in vec4 fColor;

uniform sampler2D uSampler;

out vec4 FragColor;

void main() {
    vec2 invTexCoord = vec2(fTex.x, 1.0 - fTex.y);
    FragColor = texture(uSampler, invTexCoord) * fColor;
}
)";

spritebatch::spritebatch() : spritebatch(default_vssrc, default_fssrc) {
}

spritebatch::spritebatch(std::string_view vssrc, std::string_view fssrc) {
  prog = create_program_from_source(vssrc, fssrc);

  uTransform = glGetUniformLocation(prog.get_handle(), "uTransform");

  glGenVertexArrays(1, &vao);
  glGenBuffers(1, &buffer);

  glBindVertexArray(vao);

  glBindBuffer(GL_ARRAY_BUFFER, buffer);

  glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(spritebatch_vertex), (void*)0);
  glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(spritebatch_vertex), (void*)16);
  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);
}

void spritebatch::draw_quad(const texture* tex, const rectangle& src, const rectangle& dest) {
  float r = 1, g = 1, b = 1;
  draw_quad(tex, src, dest, r, g, b);
}

void spritebatch::draw_quad(const texture* tex, const rectangle& src, const rectangle& dest, float r, float g, float b, float a) {
  int s_left = src.x;
  int s_right = src.x + src.w;
  int s_top = src.y;
  int s_bottom = src.y + src.h;

  float uv_left = s_left / (float)tex->width;
  float uv_right = s_right / (float)tex->width;
  float uv_top = s_top / (float)tex->height;
  float uv_bottom = s_bottom / (float)tex->height;

  spritebatch_vertex vertices[] = {
      {dest.x + dest.w, dest.y, uv_right, uv_top, r, g, b, a},
      {dest.x + dest.w, dest.y + dest.h, uv_right, uv_bottom, r, g, b, a},
      {dest.x,  dest.y, uv_left, uv_top, r, g, b, a},
      {dest.x + dest.w, dest.y + dest.h, uv_right, uv_bottom, r, g, b, a},
      {dest.x, dest.y + dest.h, uv_left, uv_bottom, r, g, b, a},
      {dest.x, dest.y, uv_left, uv_top, r, g, b, a}
  };

  // TODO: index this
  batch.push_back(vertices[0]);
  batch.push_back(vertices[1]);
  batch.push_back(vertices[2]);
  batch.push_back(vertices[3]);
  batch.push_back(vertices[4]);
  batch.push_back(vertices[5]);
}

void spritebatch::draw_tiled_quad(const texture* tex, const rectangle& dest, float rx, float ry) {
  float r = 1, g = 1, b = 1, a = 1;

  float uv_left = 0;
  float uv_right = rx;
  float uv_top = 0;
  float uv_bottom = ry;

  spritebatch_vertex vertices[] = {
      {dest.x + dest.w, dest.y, uv_right, uv_top, r, g, b, a},
      {dest.x + dest.w, dest.y + dest.h, uv_right, uv_bottom, r, g, b, a},
      {dest.x,  dest.y, uv_left, uv_top, r, g, b, a},
      {dest.x + dest.w, dest.y + dest.h, uv_right, uv_bottom, r, g, b, a},
      {dest.x, dest.y + dest.h, uv_left, uv_bottom, r, g, b, a},
      {dest.x, dest.y, uv_left, uv_top, r, g, b, a}
  };

  // TODO: index this
  batch.push_back(vertices[0]);
  batch.push_back(vertices[1]);
  batch.push_back(vertices[2]);
  batch.push_back(vertices[3]);
  batch.push_back(vertices[4]);
  batch.push_back(vertices[5]);
}

void spritebatch::begin() {
  glUseProgram(prog.get_handle());
  glBindVertexArray(vao);
  glBindBuffer(GL_ARRAY_BUFFER, buffer);

  glUniformMatrix4fv(uTransform, 1, GL_FALSE, glm::value_ptr(transform));
}

void spritebatch::end() {
    flush();
}

void spritebatch::flush() {
    if (batch.size() == 0) {
        return;
    }

    glBufferData(GL_ARRAY_BUFFER, batch.size() * sizeof(spritebatch_vertex), batch.data(), GL_STREAM_DRAW);
    glDrawArrays(GL_TRIANGLES, 0, batch.size());
    batch.clear();
}

void spritebatch::set_output_dimensions(int w, int h) {
  transform = glm::ortho<float>(0, w, h, 0);
}
