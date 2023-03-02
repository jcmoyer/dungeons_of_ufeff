#pragma once

#include <SDL.h>

struct texture;
class imm_renderer;

class tooltip {
public:
    void set_texture(const texture* tex_) { tex = tex_; }

    void draw(imm_renderer* renderer, const SDL_Rect& dest);

private:
    const texture* tex;
};