#include "texture_manager.hpp"

#include <cassert>

#include "stb_image.h"

const texture* texture_manager::get(const std::string& filename)
{
    stbi_set_flip_vertically_on_load(true);

    if (auto it = cache.find(filename); it != cache.end())
    {
        // references to elements within a map are guaranteed to not be invalidated on insert/erase
        return &it->second;
    }
    else
    {
        int x, y, n;
        unsigned char* data = stbi_load(filename.c_str(), &x, &y, &n, 4);
        assert(data != nullptr);

        texture t;
        t.width = x;
        t.height = y;

        glBindTexture(GL_TEXTURE_2D, t.tex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, x, y, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        //glGenerateMipmap(GL_TEXTURE_2D_ARRAY);

        stbi_image_free(data);

        auto [new_it, success] = cache.emplace(std::make_pair(filename, std::move(t)));
        return &new_it->second;
    }
}

void texture_manager::clear()
{
    return cache.clear();
}