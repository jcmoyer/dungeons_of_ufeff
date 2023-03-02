#include "quad_renderer.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "texture_manager.hpp"

constexpr auto default_vssrc = R"(#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec4 aColor;

uniform mat4 uTransform;

out vec4 fColor;

void main() {
    gl_Position = uTransform * vec4(aPos, 0.0, 1.0);
    fColor = aColor;
}
)";

constexpr auto default_fssrc = R"(#version 330 core

in vec4 fColor;

out vec4 FragColor;

void main() {
    FragColor = fColor;
}
)";

quad_renderer::quad_renderer() : quad_renderer(default_vssrc, default_fssrc) {
}

quad_renderer::quad_renderer(std::string_view vssrc, std::string_view fssrc) {
  prog = create_program_from_source(vssrc, fssrc);

  glGenVertexArrays(1, &vao);
  glGenBuffers(1, &buffer);

  glBindVertexArray(vao);

  glBindBuffer(GL_ARRAY_BUFFER, buffer);

  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(quad_vertex), (void*)0);
  glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(quad_vertex), (void*)8);
  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);
}

void quad_renderer::draw_quad(const rectangle& dest) {
  float r = 1, g = 1, b = 1, a = 1;
  draw_quad(dest, r, g, b, a);
}

void quad_renderer::draw_quad(const rectangle& dest, float r, float g, float b, float a) {
  quad_vertex vertices[] = {
      {dest.x + dest.w, dest.y, r, g, b, a},
      {dest.x + dest.w, dest.y + dest.h, r, g, b, a},
      {dest.x,  dest.y, r, g, b, a},
      {dest.x + dest.w, dest.y + dest.h, r, g, b, a},
      {dest.x, dest.y + dest.h, r, g, b, a},
      {dest.x, dest.y, r, g, b, a}
  };

  // TODO: index this
  batch.push_back(vertices[0]);
  batch.push_back(vertices[1]);
  batch.push_back(vertices[2]);
  batch.push_back(vertices[3]);
  batch.push_back(vertices[4]);
  batch.push_back(vertices[5]);
}

void quad_renderer::draw_quad(const rectangle& dest, uint32_t rgba) {
    float r = ((rgba & 0xff000000) >> 24) / 255.f;
    float g = ((rgba & 0x00ff0000) >> 16) / 255.f;
    float b = ((rgba & 0x0000ff00) >>  8) / 255.f;
    float a = ((rgba & 0x000000ff) >>  0) / 255.f;
    draw_quad(dest, r, g, b, a);
}

void quad_renderer::begin() {
  glUseProgram(prog.get_handle());
  glBindVertexArray(vao);
  glBindBuffer(GL_ARRAY_BUFFER, buffer);

  auto uTransform = glGetUniformLocation(prog.get_handle(), "uTransform");
  glUniformMatrix4fv(uTransform, 1, GL_FALSE, glm::value_ptr(transform));
}

void quad_renderer::end() {
  glBufferData(GL_ARRAY_BUFFER, batch.size() * sizeof(quad_vertex), batch.data(), GL_STREAM_DRAW);
  glDrawArrays(GL_TRIANGLES, 0, batch.size());
  batch.clear();
}

void quad_renderer::set_output_dimensions(int w, int h) {
  transform = glm::ortho<float>(0, w, h, 0);
}
