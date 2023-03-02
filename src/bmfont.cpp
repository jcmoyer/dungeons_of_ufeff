#include "bmfont.hpp"
#include "spritebatch.hpp"
#include <algorithm>
#include "texture_manager.hpp"

enum charclass {
    invalid,
    letter,
    number,
    special,
};

bool is_lowercase_letter(char ch) {
    return ch >= 'a' && ch <= 'z';
}
bool is_letter(char ch) {
    return ch >= 'A' && ch <= 'Z';
}
bool is_number(char ch) {
    return ch >= '0' && ch <= '9';
}
bool is_special(unsigned char ch) {
    return ch >= 0xF0;
}

charclass classify(char ch) {
    if (is_letter(ch)) {
        return letter;
    } else if (is_number(ch)) {
        return number;
    } else if (is_special(ch)) {
        return special;
    } else {
        return invalid;
    }
}

int map_cell(char ch) {
    // TODO: remove hack when font has lowercase glyphs
    if (is_lowercase_letter(ch)) {
        ch = 'A' + (ch - 'a');
    }

    switch (classify(ch)) {
    case letter: return ch - 'A';
    case number: return 32 + ch - '0';
    case special: return 48 + (unsigned char)ch - 0xF0;
    case invalid: return -1;
    }
    return -1;
}

void bmfont::begin(spritebatch* rend) {
    renderer = rend;
    renderer->begin();
    glBindTexture(GL_TEXTURE_2D, tex->tex);
}

void bmfont::end() {
    renderer->end();
    renderer = nullptr;
}

void bmfont::draw_string(std::string_view text, int x, int y) {
    assert(renderer != nullptr);
    assert(tex != nullptr);

    int offs_x = 0;
    int offs_y = 0;

    for (char ch : text) {
        int cell = map_cell(ch);
        if (cell >= 0) {
            int cell_x = cell & 0x0F;
            int cell_y = (cell & 0xF0) >> 4;

            rectangle src;
            src.x = cell_x * 16;
            src.y = cell_y * 24;
            src.w = 16;
            src.h = 24;
            rectangle dest;
            dest.x = x + offs_x;
            dest.y = y + offs_y;
            dest.w = 16;
            dest.h = 24;

            renderer->draw_quad(tex, src, dest);
        }
        offs_x += 16;

        if (ch == '\n') {
            offs_x = 0;
            offs_y += 24;
        }
    }
}

void bmfont::draw_string(const bmfont_glyph_map& map, std::string_view text, int x, int y) {
    draw_string(map, text, x, y, 1, 1, 1);
}

void bmfont::draw_string(const bmfont_glyph_map& map, std::string_view text, int x, int y, float r, float g, float b, float a) {
    assert(renderer != nullptr);
    assert(tex != nullptr);

    int offs_x = 0;
    int offs_y = 0;

    for (char ch : text) {
        if (ch == '\n') {
            offs_x = 0;
            offs_y += map.space().h + map.line_spacing();
            continue;
        } else if (ch == ' ') {
            offs_x += map.space().w;
            continue;
        }

        rectangle src = map.map(ch);
        rectangle dest;
        dest.x = x + offs_x;
        dest.y = y + offs_y;
        dest.w = src.w;
        dest.h = src.h;
        renderer->draw_quad(tex, src, dest, r, g, b, a);
        offs_x += src.w + 2;
    }
}

bmfont_measurement bmfont::measure_string(std::string_view text) {
    int longest_line = 0;
    int current_line = 0;
    int num_lines = 1;

    for (char ch : text) {
        if (ch == '\n') {
            ++num_lines;
            longest_line = std::max(current_line, longest_line);
            current_line = 0;
        } else {
            ++current_line;
        }
    }
    longest_line = std::max(current_line, longest_line);

    return {
        longest_line * 16,
        num_lines * 24
    };
}

bmfont_measurement bmfont::measure_string(std::string_view text, const bmfont_glyph_map& map) {
    int longest_width = 0;
    int current_width = 0;
    int current_height = 0;
    int total_height = 0;
    int num_lines = 1;


    int offs_x = 0;
    int offs_y = 0;

    for (char ch : text) {
        if (ch == '\n') {
            // chop off trailing extra space
            current_width -= 2;
            total_height += map.space().h + map.line_spacing();
            longest_width = std::max(current_width, longest_width);
            current_width = 0;
            current_height = 0;
            continue;
        } else if (ch == ' ') {
            current_width += map.space().w;
            continue;
        }

        rectangle src = map.map(ch);
        current_width += src.w + 2;
        current_height = std::max(current_height, src.h);
    }

    // chop off trailing extra space
    current_width -= 2;
    total_height += current_height;
    longest_width = std::max(current_width, longest_width);

    return {
        longest_width,
        total_height
    };
}




const glyph_map_font_blue_large& glyph_map_font_blue_large::instance() {
  static glyph_map_font_blue_large inst;
  return inst;
}

rectangle glyph_map_font_blue_large::map(char ch) const {
    std::unordered_map<char, rectangle> m{
      {'A', {0, 240, 16, 16}},
      {'B', {17, 240, 14, 16}},
      {'C', {33, 240, 14, 16}},
      {'D', {49, 240, 14, 16}},
      {'E', {65, 240, 13, 16}},
      {'F', {81, 240, 13, 16}},
      {'G', {97, 240, 14, 16}},
      {'H', {113, 240, 14, 16}},
      {'I', {133, 240, 6, 16}},
      {'J', {145, 240, 13, 16}},
      {'K', {161, 240, 14, 16}},
      {'L', {177, 240, 13, 16}},
      {'M', {192, 240, 16, 16}},

      {'N', {1, 258, 14, 16}},
      {'O', {17, 258, 15, 16}},
      {'P', {34, 258, 12, 16}},
      {'Q', {48, 258, 16, 16}},
      {'R', {65, 258, 15, 16}},
      {'S', {81, 258, 14, 16}},
      {'T', {97, 258, 14, 16}},
      {'U', {113, 258, 14, 16}},
      {'V', {129, 258, 14, 16}},
      {'W', {144, 258, 16, 16}},
      {'X', {161, 258, 14, 16}},
      {'Y', {177, 258, 14, 16}},
      {'Z', {193, 258, 14, 16}},

      {'a', {1, 276, 14, 16}},
      {'b', {17, 276, 14, 16}},
      {'c', {33, 276, 13, 16}},
      {'d', {49, 276, 14, 16}},
      {'e', {65, 276, 13, 16}},
      {'f', {81, 276, 10, 16}},
      {'g', {97, 276, 14, 23}},
      {'h', {113, 276, 15, 16}},
      {'i', {133, 276, 6, 16}},
      {'j', {145, 276, 13, 23}},
      {'k', {161, 276, 14, 16}},
      {'l', {181, 276, 6, 16}},
      {'m', {193, 276, 15, 16}},

      {'n', {1, 300, 15, 16}},
      {'o', {17, 300, 14, 16}},
      {'p', {33, 300, 14, 23}},
      {'q', {49, 300, 14, 23}},
      {'r', {65, 300, 13, 16}},
      {'s', {81, 300, 14, 16}},
      {'t', {99, 300, 10, 16}},
      {'u', {113, 300, 14, 16}},
      {'v', {129, 300, 14, 16}},
      {'w', {144, 300, 16, 16}},
      {'x', {162, 300, 11, 16}},
      {'y', {177, 300, 14, 23}},
      {'z', {193, 300, 14, 16}},
    };

    return m[ch];
}

rectangle glyph_map_font_blue_large::space() const {
  return { 0, 0, 16, 16 };
}

const glyph_map_font_yellow_large& glyph_map_font_yellow_large::instance() {
  static glyph_map_font_yellow_large inst;
  return inst;
}

rectangle glyph_map_font_yellow_large::map(char ch) const {
  rectangle rect = glyph_map_font_blue_large::instance().map(ch);
  rect.x += 208;
  return rect;
}

rectangle glyph_map_font_yellow_large::space() const {
  return glyph_map_font_blue_large::instance().space();
}

const glyph_map_font_white_small& glyph_map_font_white_small::instance() {
    static glyph_map_font_white_small inst;
    return inst;
}

rectangle glyph_map_font_white_small::map(char ch) const {
    std::unordered_map<char, rectangle> m{
        {'A', {0, 325, 4, 7}},
        {'B', {5, 325, 4, 7}},
        {'C', {10, 325, 4, 7}},
        {'D', {15, 325, 4, 7}},
        {'E', {20, 325, 3, 7}},
        {'F', {24, 325, 3, 7}},
        {'G', {28, 325, 4, 7}},
        {'H', {33, 325, 4, 7}},
        {'I', {38, 325, 3, 7}},
        {'J', {42, 325, 4, 7}},
        {'K', {47, 325, 4, 7}},
        {'L', {52, 325, 3, 7}},
        {'M', {56, 325, 5, 7}},
        {'N', {62, 325, 4, 7}},
        {'O', {67, 325, 4, 7}},
        {'P', {72, 325, 4, 7}},
        {'Q', {77, 325, 5, 7}},
        {'R', {83, 325, 4, 7}},
        {'S', {88, 325, 4, 7}},
        {'T', {93, 325, 5, 7}},
        {'U', {99, 325, 4, 7}},
        {'V', {104, 325, 5, 7}},
        {'W', {110, 325, 7, 7}},
        {'X', {118, 325, 5, 7}},
        {'Y', {124, 325, 4, 7}},
        {'Z', {129, 325, 5, 7}},

        {'a', {0, 333, 4, 7}},
        {'b', {5, 333, 4, 7}},
        {'c', {10, 333, 3, 7}},
        {'d', {14, 333, 4, 7}},
        {'e', {19, 333, 4, 7}},
        {'f', {24, 333, 2, 7}},
        {'g', {27, 333, 4, 10}},
        {'h', {32, 333, 4, 7}},
        {'i', {37, 333, 1, 7}},
        {'j', {38, 333, 2, 10}},
        {'k', {41, 333, 3, 7}},
        {'l', {45, 333, 1, 7}},
        {'m', {47, 333, 5, 7}},
        {'n', {53, 333, 3, 7}},
        {'o', {57, 333, 4, 7}},
        {'p', {62, 333, 4, 10}},
        {'q', {67, 333, 5, 10}},
        {'r', {72, 333, 3, 7}},
        {'s', {76, 333, 3, 7}},
        {'t', {80, 333, 3, 7}},
        {'u', {84, 333, 3, 7}},
        {'v', {88, 333, 3, 7}},
        {'w', {92, 333, 5, 7}},
        {'x', {98, 333, 3, 7}},
        {'y', {102, 333, 4, 10}},
        {'z', {107, 333, 4, 7}},

        // not in original font texture
        {'.', {135, 325, 1, 7}},
        {'!', {137, 325, 1, 7}},
        {',', {139, 325, 2, 8}},
        {'\'', {142, 325, 1, 8}},
        {'?', {145, 325, 4, 8}},
        {'1', {150, 325, 3, 7}},
        {'2', {154, 325, 4, 7}},
        {'3', {159, 325, 4, 7}},
        {'4', {164, 325, 4, 7}},
        {'5', {169, 325, 4, 7}},
        {'6', {174, 325, 4, 7}},
        {'7', {179, 325, 4, 7}},
        {'8', {184, 325, 4, 7}},
        {'9', {189, 325, 4, 7}},
        {'0', {194, 325, 4, 7}},
        {'/', {199, 325, 7, 7}},
        {'-', {208, 325, 3, 7}}
    };

    return m[ch];
}

rectangle glyph_map_font_white_small::space() const {
    return { 0, 0, 4, 7 };
}
