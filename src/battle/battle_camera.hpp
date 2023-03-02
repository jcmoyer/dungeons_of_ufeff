#pragma once

#include <glm/vec2.hpp>
#include <glm/gtx/compatibility.hpp>
#include <glm/gtx/transform.hpp>

struct battle_camera {
    static constexpr float TRACK_RATE_OPENING = 0.02f;
    static constexpr float TRACK_RATE_NORMAL = 0.2f;

    glm::vec2 center{};
    glm::vec2 prev_center{};
    glm::vec2 target{};
    float tracking_rate = TRACK_RATE_NORMAL;

    void center_on(glm::vec2 pos) {
        prev_center = center = pos;
    }

    void set_target(glm::vec2 pos) {
        target = pos;
    }

    void update() {
        prev_center = center;
        center = glm::lerp(center, target, tracking_rate);
    }

    glm::mat4 get_view(double a) {
        glm::vec2 interp_center = glm::lerp(prev_center, center, (float)a);
        return glm::lookAt(glm::vec3(interp_center, 150) + glm::vec3(0, 10, 0), glm::vec3(interp_center, 0), glm::vec3(0, 1, 0));
    }
};
