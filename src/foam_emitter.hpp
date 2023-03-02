#pragma once

#include <glm/vec2.hpp>
#include "quad_renderer.hpp"
#include <vector>
#include "rectangle.hpp"
#include "camera.hpp"
#include "random.hpp"

struct foam_particle {
  glm::vec2 world_pos;
  glm::vec2 vel;
  float scale;
  float initial_scale;
  bool alive;
};

struct foam_emitter {
  std::vector<foam_particle> particles;

  quad_renderer* rend;
  foam_emitter(quad_renderer* r) : rend{ r } {}

  void render(const camera& cam) {
    rend->begin();

    for (foam_particle& p : particles) {
      if (!cam.get_view().contains(p.world_pos.x, p.world_pos.y)) {
        continue;
      }
      rectangle dest;
      dest.x = p.world_pos.x - cam.left() - p.scale / 2.0f;
      dest.y = p.world_pos.y - cam.top() - p.scale / 2.0f;
      dest.w = p.scale;
      dest.h = p.scale;
      rend->draw_quad(dest, 1, 1, 1, p.scale / p.initial_scale);
    }

    rend->end();
  }

  void emit(glm::vec2 world_pos, glm::vec2 vel, float scale_max = 3.f) {
    foam_particle& p = particles.emplace_back();
    p.world_pos = world_pos;
    p.initial_scale = random::rand_real(1, scale_max);
    p.scale = p.initial_scale;
    p.vel = vel;
  }

  void update() {
    for (size_t i = 0; i < particles.size(); ++i) {
      particles[i].scale -= 0.1f;
      particles[i].alive = particles[i].scale > 0;
      particles[i].world_pos += particles[i].vel;
      particles[i].vel *= 0.8f;
      particles[i].vel.y += 0.1f;
    }

    auto first_dead = std::remove_if(particles.begin(), particles.end(), [](const foam_particle& p) {
      return !p.alive;
    });

    particles.erase(first_dead, particles.end());
  }
};