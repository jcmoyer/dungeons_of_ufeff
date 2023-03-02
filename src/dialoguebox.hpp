#pragma once

#include "rectangle.hpp"
#include <string_view>

struct texture;
class spritebatch;

constexpr rectangle DIALOGUE_RECTS[8] = {
  // top left
  { 92, 188, 4, 4 },
  // top right
  { 112, 188, 4, 4 },
  // bottom left
  { 92, 208, 4, 4 },
  // bottom right
  { 112, 208, 4, 4 },

  // top
  { 96, 188, 16, 4 },
  // bottom
  { 96, 208, 16, 4 },
  // left
  { 92, 192, 4, 16 },
  // right
  { 112, 192, 4, 16 },
};

// clang-format off
constexpr size_t DIA_TOPLEFT     = 0;
constexpr size_t DIA_TOPRIGHT    = 1;
constexpr size_t DIA_BOTTOMLEFT  = 2;
constexpr size_t DIA_BOTTOMRIGHT = 3;
constexpr size_t DIA_TOP         = 4;
constexpr size_t DIA_BOTTOM      = 5;
constexpr size_t DIA_LEFT        = 6;
constexpr size_t DIA_RIGHT       = 7;

struct dialogue_info {
  const texture* sides;
  const texture* fill;
  
};

rectangle measure_dialogue_box(const dialogue_info& di, std::string_view text);
void render_dialogue_box(const dialogue_info& di, spritebatch& batch, std::string_view text, int x, int y);
