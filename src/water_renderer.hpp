#pragma once

#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
#include <vector>

#include "rectangle.hpp"
#include "shader.hpp"

struct texture;

struct water_vertex
{
    float x, y, u, v;
    float world_x, world_y;
};

struct water_render_parameters
{
    glm::vec2 water_direction;
    glm::vec2 water_drift_scale{32, 16};
    glm::vec2 water_drift_range;
    double water_speed;
    double global_time;
    float blend_amount = 0.0f;
};

// this ended up being almost an exact copy of spritebatch because it turns out some things needed to be done slightly differently
// perhaps the correct thing to do would be to turn this into an ubershader for all terrain rendering so you can apply warp effects
// to arbitrary tile types
class water_renderer
{
public:
    water_renderer();

    void begin(const water_render_parameters& opts);
    void end();

    void draw_quad(const texture* tex, const rectangle& src, const rectangle& dest, float world_x, float world_y, float square_size);

    void set_output_dimensions(int w, int h);

private:
    shader_program prog;

    std::vector<water_vertex> batch;

    GLuint vao;
    GLuint buffer;

    glm::mat4 transform;

    GLint uTransform;
    GLint uGlobalTime;
    GLint uWaterDirection;
    GLint uWaterSpeed;
    GLint uWaterDriftRange;
    GLint uSamplerBase;
    GLint uSamplerBlend;
    GLint uBlendAmount;
    GLint uWaterDriftScale;
};
