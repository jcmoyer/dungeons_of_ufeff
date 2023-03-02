add_library(stb_image stb_image.c stb_image.h)
target_include_directories(stb_image INTERFACE .)

add_library(stb_vorbis stb_vorbis.c)
target_include_directories(stb_vorbis INTERFACE .)