#pragma once

#include <glm/mat4x4.hpp>
#include "shader.hpp"

struct texture;
struct rectangle;

struct battle_field_vertex {
    float x, y, z;
    float u, v;
    float nx, ny, nz;
};

struct battle_field_mesh {
    std::vector<battle_field_vertex> vertices;
};

class battle_field_renderer {
public:
    battle_field_renderer();

    void set_mesh(const battle_field_mesh& mesh);
    void render(const glm::mat4& view, const glm::mat4& projection);

    void set_light_direction(const glm::vec3& dir);

private:
    shader_program prog;
    GLuint vao;
    GLuint buffer;
    GLsizei mesh_size = 0;
    glm::vec3 light_direction;

    GLint uView;
    GLint uProjection;
    GLint uLightDirection;
};