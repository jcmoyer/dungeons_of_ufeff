#include "battle_field_renderer.hpp"

#include <GL/gl3w.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "texture_manager.hpp"

constexpr auto default_vssrc = R"(#version 330 core
layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in vec3 aNormal;

uniform mat4 uView;
uniform mat4 uProjection;

out vec2 fTexCoord;
out vec3 fNormal;

void main() {
    gl_Position = uProjection * uView * vec4(aPosition, 1.0);
    fTexCoord   = aTexCoord;
    fNormal     = aNormal;
}
)";

constexpr auto default_fssrc = R"(#version 330 core

in vec2 fTexCoord;
in vec3 fNormal;

uniform sampler2D uSampler;
uniform vec3 uLightDirection;

out vec4 FragColor;

void main() {
    // and here we apply babby's first linear algebra lesson
    // normal should point towards lightDir to be lit, so we want the dot product to be negative;
    // closer to -1 is more intense
    float lightMag = 0.3 - clamp(dot(fNormal, uLightDirection), -0.7, 0.0);

    vec2 invTexCoord = vec2(fTexCoord.x, 1.0 - fTexCoord.y);
    vec4 color = texture(uSampler, invTexCoord);

    FragColor = vec4(color.rgb * lightMag, color.a);
}
)";

battle_field_renderer::battle_field_renderer()
{
    prog = create_program_from_source(default_vssrc, default_fssrc);

    uView = glGetUniformLocation(prog.get_handle(), "uView");
    uProjection = glGetUniformLocation(prog.get_handle(), "uProjection");
    uLightDirection = glGetUniformLocation(prog.get_handle(), "uLightDirection");

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &buffer);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, buffer);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(battle_field_vertex), (void*)0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(battle_field_vertex), (void*)12);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(battle_field_vertex), (void*)20);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
}

void battle_field_renderer::set_mesh(const battle_field_mesh& mesh)
{
    glBindVertexArray(vao); // don't think this is necessary?
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, mesh.vertices.size() * sizeof(battle_field_vertex), mesh.vertices.data(), GL_STATIC_DRAW);
    mesh_size = (GLsizei)mesh.vertices.size();
}

void battle_field_renderer::render(const glm::mat4& view, const glm::mat4& projection)
{
    glUseProgram(prog.get_handle());
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, buffer); // don't think this is necessary, should be part of vao state?

    glUniformMatrix4fv(uView, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(uProjection, 1, GL_FALSE, glm::value_ptr(projection));
    glUniform3fv(uLightDirection, 1, glm::value_ptr(light_direction));

    glDrawArrays(GL_TRIANGLES, 0, mesh_size);
}

void battle_field_renderer::set_light_direction(const glm::vec3& dir)
{
    light_direction = dir;
}