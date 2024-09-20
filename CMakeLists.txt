cmake_minimum_required(VERSION 3.16)
project(dungeons VERSION 1.1)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

add_subdirectory(thirdparty EXCLUDE_FROM_ALL)

set(dungeons_srcs
  "src/main.cpp"
  "src/game.cpp"
  "src/camera.cpp"
  "src/texture_manager.cpp"
  "src/bmfont.cpp"
  "src/imm_renderer.cpp"
  "src/tooltip.cpp"
  "src/audio.cpp"
  "src/spritebatch.cpp"
  "src/water_renderer.cpp"
  "src/quad_renderer.cpp"
  "src/random.cpp"
  "src/dialoguebox.cpp"
  "src/st_play.cpp"
  "src/st_mainmenu.cpp"
  "src/npc.cpp"
  "src/animation_data.cpp"
  "src/st_battle.cpp"
  "src/battle_object_renderer.cpp"
  "src/battle_field_renderer.cpp"
  "src/screen_renderer.cpp"
  "src/battle/skill.cpp"
  "src/battle/controller.cpp"
  "src/battle/battle_character.cpp"
  "src/battle/battle_character_info.cpp"
  "src/battle/encounters.cpp"
  "src/st_gameover.cpp"
  "src/st_battlestats.cpp"
  "src/ui.cpp"
  "src/st_gamewin.cpp"
  "src/global_services.cpp"
  "src/st_options.cpp"
  "assets/extra/resources.rc")

add_executable(dungeons ${dungeons_srcs})

target_link_libraries(dungeons SDL2 SDL2main gl3w glm::glm stb_vorbis stb_image)
set_property(TARGET dungeons PROPERTY CXX_STANDARD 23)

if(MSVC)
    target_compile_options(dungeons PRIVATE /W4 /WX /external:W0 /external:anglebrackets)
else()
    target_compile_options(dungeons PRIVATE -Wall -Wextra -pedantic -Werror)
endif()

install(TARGETS dungeons SDL2 RUNTIME DESTINATION .)

target_compile_definitions(dungeons PRIVATE NOMINMAX)

##############################################################################
# build and install maps
##############################################################################
function(add_map mapname)
    add_custom_command(
        OUTPUT "${CMAKE_BINARY_DIR}/assets/maps/${mapname}.bin"
        DEPENDS "${CMAKE_SOURCE_DIR}/assets/maps/${mapname}.tmx"
        COMMAND tiled --export-map json "${CMAKE_SOURCE_DIR}/assets/maps/${mapname}.tmx" "${CMAKE_BINARY_DIR}/assets/maps/${mapname}.json"
        COMMAND python "${CMAKE_SOURCE_DIR}/scripts/tiled2map.py" "${CMAKE_BINARY_DIR}/assets/maps/${mapname}.json" "${CMAKE_BINARY_DIR}/assets/maps/${mapname}.bin"
    )
    add_custom_target(UMAP_${mapname} ALL DEPENDS "assets/maps/${mapname}.bin")
    install(FILES
        "${CMAKE_BINARY_DIR}/assets/maps/${mapname}.bin"
        DESTINATION assets/maps
    )
endfunction()

add_map("hub")
add_map("east_tower_basement")
add_map("sanctum")
add_map("lair")

##############################################################################
# textures
##############################################################################
install(FILES
  assets/amalgamation.png
  assets/water_base.png
  assets/water_foam.png
  assets/waterfall.png
  assets/dialogue_background.png
  assets/mask.png
  assets/lava_base.png
  assets/lava_blend.png

  DESTINATION assets)

##############################################################################
# sounds
##############################################################################
install(FILES
  assets/sound/door.ogg
  assets/sound/flashjump.ogg
  assets/sound/torch.ogg
  assets/sound/switch.ogg
  assets/sound/throw1.ogg
  assets/sound/stab.ogg
  assets/sound/avenger.ogg
  assets/sound/ambush.ogg
  assets/sound/chain_mechanism.ogg
  assets/sound/chest.ogg
  assets/sound/metal_door.ogg
  assets/sound/start.ogg
  assets/sound/slime0.ogg
  assets/sound/slime1.ogg
  assets/sound/slime2.ogg
  assets/sound/bone.ogg
  assets/sound/gameover.ogg
  assets/sound/fanfare.ogg
  assets/sound/blip.ogg
  assets/sound/levelup.ogg
  assets/sound/knifedraw.ogg
  assets/sound/warpspell.ogg
  assets/sound/die.ogg

  DESTINATION assets/sound)

##############################################################################
# music
##############################################################################
install(FILES
  assets/music/ambient_ominous.ogg
  assets/music/make_your_stand.ogg
  assets/music/quagmire_of_lost_souls.ogg
  assets/music/sanctum_of_thought.ogg

  DESTINATION assets/music)

##############################################################################
# models
##############################################################################
function(add_model modelname)
    add_custom_command(
        OUTPUT "${CMAKE_BINARY_DIR}/assets/models/${modelname}.bin"
        DEPENDS "${CMAKE_SOURCE_DIR}/assets/models/${modelname}.blend"
        COMMAND blender "${CMAKE_SOURCE_DIR}/assets/models/${modelname}.blend" --python-exit-code 1 -b -P "${CMAKE_SOURCE_DIR}/scripts/blender_export.py" -- "${CMAKE_BINARY_DIR}/assets/models/${modelname}.bin"
    )
    add_custom_target(UMDL_${modelname} ALL DEPENDS "assets/models/${modelname}.bin")
    install(FILES
        "${CMAKE_BINARY_DIR}/assets/models/${modelname}.bin"
        DESTINATION assets/models
    )
endfunction()

add_model("bf_dungeon")
add_model("bf_sanctum")
add_model("bf_lair")

##############################################################################
# misc
##############################################################################
install(FILES
  attribution.txt
  license.txt
  controls.txt

  DESTINATION .
)
