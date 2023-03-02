#pragma once

#include <string_view>
#include <GL/gl3w.h>
#include <SDL.h>
#include "rectangle.hpp"

class spritebatch;
struct texture;

struct bmfont_measurement {
    int width, height;
};

class bmfont_glyph_map {
public:
  virtual rectangle map(char ch) const = 0;
  virtual rectangle space() const = 0;
  virtual int line_spacing() const = 0;
};

class bmfont {
public:
    void set_texture(const texture* tex_) {
        tex = tex_;
    }

    void begin(spritebatch* renderer);
    void end();

    void draw_string(std::string_view text, int x, int y);
    void draw_string(const bmfont_glyph_map& map, std::string_view text, int x, int y);
    void draw_string(const bmfont_glyph_map& map, std::string_view text, int x, int y, float r, float g, float b, float a = 1.0f);
    bmfont_measurement measure_string(std::string_view text);
    bmfont_measurement measure_string(std::string_view text, const bmfont_glyph_map& map);

private:
    const texture* tex = nullptr;
    spritebatch* renderer = nullptr;
};


class glyph_map_font_blue_large : public bmfont_glyph_map {
public:
    static const glyph_map_font_blue_large& instance();
    rectangle map(char ch) const override;
    rectangle space() const override;
    int line_spacing() const override { return 2; }
};

class glyph_map_font_yellow_large : public bmfont_glyph_map {
public:
    static const glyph_map_font_yellow_large& instance();
    rectangle map(char ch) const override;
    rectangle space() const override;
    int line_spacing() const override { return 2; }
};

class glyph_map_font_white_small : public bmfont_glyph_map {
public:
    static const glyph_map_font_white_small& instance();
    rectangle map(char ch) const override;
    rectangle space() const override;
    int line_spacing() const override { return 2; }
};