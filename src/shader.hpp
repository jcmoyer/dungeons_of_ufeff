#pragma once

#include <GL/gl3w.h>
#include <fmt/format.h>
#include <string_view>

class shader_base
{
public:
    GLuint get_handle() const { return handle; }

protected:
    shader_base(GLenum type, std::string_view src)
    {
        handle = glCreateShader(type);
        const char* ptr = src.data();
        glShaderSource(handle, 1, &ptr, nullptr);
        glCompileShader(handle);

        GLint status;
        glGetShaderiv(handle, GL_COMPILE_STATUS, &status);

        if (!status)
        {
            GLchar info[1024];
            glGetShaderInfoLog(handle, sizeof(info), nullptr, info);
            fmt::print("failed to compile shader: {}", info);
            std::exit(EXIT_FAILURE);
        }
    }

    virtual ~shader_base()
    {
        if (handle)
        {
            glDeleteShader(handle);
            handle = 0;
        }
    }

    GLuint handle = 0;
};

class frag_shader : public shader_base
{
public:
    frag_shader(std::string_view src)
        : shader_base(GL_FRAGMENT_SHADER, src) {}
};

class vert_shader : public shader_base
{
public:
    vert_shader(std::string_view src)
        : shader_base(GL_VERTEX_SHADER, src) {}
};

class shader_program
{
public:
    shader_program() = default;

    shader_program(const vert_shader& vs, const frag_shader& fs)
    {
        handle = glCreateProgram();
        glAttachShader(handle, vs.get_handle());
        glAttachShader(handle, fs.get_handle());
        glLinkProgram(handle);

        GLint status;
        glGetProgramiv(handle, GL_LINK_STATUS, &status);

        if (!status)
        {
            GLchar info[1024];
            glGetProgramInfoLog(handle, sizeof(info), nullptr, info);
            fmt::print("failed to link program: {}", info);
            std::exit(EXIT_FAILURE);
        }
    }

    ~shader_program()
    {
        if (handle)
        {
            glDeleteProgram(handle);
            handle = 0;
        }
    }

    // non-copyable
    shader_program(const shader_program&) = delete;
    shader_program& operator=(const shader_program&) = delete;

    // moveable
    shader_program(shader_program&& x)
        : handle(x.handle)
    {
        x.handle = 0;
    }

    shader_program& operator=(shader_program&& x)
    {
        if (handle)
        {
            glDeleteProgram(handle);
        }
        handle = x.handle;
        x.handle = 0;
        return *this;
    }

    GLuint get_handle() const
    {
        return handle;
    }

    GLuint handle = 0;
};

inline shader_program create_program_from_source(std::string_view vssrc, std::string_view fssrc)
{
    vert_shader vs(vssrc);
    frag_shader fs(fssrc);
    return shader_program(vs, fs);
}
