#pragma once

#include <SDL.h>
#include <string_view>

#include <unordered_map>

#include <GL/gl3w.h>

struct texture {
    GLuint tex;
    int width, height;

    // default constructible
    texture() : width{ 0 }, height{ 0 } {
        glGenTextures(1, &tex);
    }

    // non-copyable
    texture(const texture&) = delete;
    texture& operator=(const texture&) = delete;

    // moveable
    texture(texture&& x) noexcept : tex{x.tex}, width{x.width}, height{x.height} {
        x.tex = 0;
    }
    texture& operator=(texture&& x) noexcept {
        tex = x.tex;
        x.tex = 0;
        width = x.width;
        height = x.height;
        return *this;
    }

    // releases texture on destruct
    ~texture() {
        glDeleteTextures(1, &tex);
    }

    void set_wrap(bool wrap) {
        glBindTexture(GL_TEXTURE_2D, tex);
        if (wrap) {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        } else {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        }
    }
};

class texture_manager {
public:
    texture_manager() = default;

    const texture* get(const std::string& filename);

    // releases all textures
    void clear();

private:
    std::unordered_map<std::string, texture> cache;

};