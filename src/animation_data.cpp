#include "animation_data.hpp"

// clang-format off

#define STANDARD_ANIM(name, x, y)        \
        {                                \
            name,                        \
            animation {{                 \
                {{x + 00, y, 16, 16}, 8},\
                {{x + 16, y, 16, 16}, 8},\
                {{x + 32, y, 16, 16}, 8},\
                {{x + 16, y, 16, 16}, 8},\
            }}                           \
        }

#define STANDARD_DIRECTIONAL(x, y)        \
        STANDARD_ANIM("down",  x, y + 00),\
        STANDARD_ANIM("left",  x, y + 16),\
        STANDARD_ANIM("right", x, y + 32),\
        STANDARD_ANIM("up",    x, y + 48) \

#define SINGLE_FRAME(name, x, y)\
    {                           \
        name,                   \
        animation {{            \
            {{x, y, 16, 16}, 8} \
        }}                      \
    }

const animation_set SPRITES[]{
    /* 00 */ {{ STANDARD_DIRECTIONAL(320, 0),  SINGLE_FRAME("dead", 320, 128), SINGLE_FRAME("dead2", 320, 144) }}, // human 1
    /* 01 */ {{ STANDARD_DIRECTIONAL(368, 0),  SINGLE_FRAME("dead", 336, 128), SINGLE_FRAME("dead2", 336, 144) }}, // human 2
    /* 02 */ {{ STANDARD_DIRECTIONAL(416, 0),  SINGLE_FRAME("dead", 352, 128), SINGLE_FRAME("dead2", 352, 144) }}, // human 3
    /* 03 */ {{ STANDARD_DIRECTIONAL(464, 0),  SINGLE_FRAME("dead" , 320, 160) }}, // skeleton
    /* 04 */ {{ STANDARD_DIRECTIONAL(320, 64), SINGLE_FRAME("dead" , 336, 160) }}, // slime
    /* 05 */ {{ STANDARD_DIRECTIONAL(368, 64), SINGLE_FRAME("dead" , 352, 160) }}, // bat
    /* 06 */ {{ STANDARD_DIRECTIONAL(416, 64), SINGLE_FRAME("dead" , 320, 176) }}, // ghost
    /* 07 */ {{ STANDARD_DIRECTIONAL(464, 64), SINGLE_FRAME("dead" , 336, 176) }}, // spider

    // wooden door
    /* 08 */ {{ SINGLE_FRAME("closed", 128, 0), SINGLE_FRAME("open", 128, 48), {"closed-open", animation{{ {{128,16,16,16}, 1}, {{128,32,16,16}, 1} }, "open" } } }},

    // metal door
    /* 09 */ {{ SINGLE_FRAME("closed", 176, 0), SINGLE_FRAME("open", 176, 48), {"closed-open", animation{{ {{176,16,16,16}, 1}, {{176,32,16,16}, 1} }, "open" } } }},

    // yellow torch
    /* 10 */ {{ {"", animation{{ {{128,64,16,16}, 2}, {{128 + 16,64,16,16}, 2}, {{128 + 32,64,16,16}, 2}, {{128 + 16,64,16,16}, 2} } } } }},

    // unlit torch
    /* 11 */ {{ SINGLE_FRAME("", 80, 160) }},

    // yellow switch
    /* 12 */ {{ SINGLE_FRAME("off", 176, 64), SINGLE_FRAME("on", 208, 64),
       {"off-on", animation{{ {{192,64,16,16}, 1} }, "on" }},
       {"on-off", animation{{ {{192,64,16,16}, 1} }, "off" }}
    }},

    // flame (battle sprite)
    /* 13 */ {{ {"", animation{{ {{112,160,16,16}, 2}, {{112 + 16,160,16,16}, 2}, {{112 + 32,160,16,16}, 2}, {{112 + 16,160,16,16}, 2} } } } }},

    // spike (battle sprite)
    {{ SINGLE_FRAME("", 96, 160) }},

    // double throw (battle sprite)
    {{ {"", animation{{ {{160,160,16,16}, 2}, {{176,160,16,16}, 2}, {{192,160,16,16}, 2},  {{208,160,16,16}, 2}, {{224,160,16,16}, 2}, {{240,160,16,16}, 2}, {{256,160,16,16}, 2},  }, "dead"}}, SINGLE_FRAME("dead", 0, 0) }},

    // blood (battle sprite, particle)
    {{ SINGLE_FRAME("", 16, 16) }},

    // double throw impact (battle sprite)
    {{ {"", animation{{ {{160,176,16,16}, 2}, {{176,176,16,16}, 2}, {{192,176,16,16}, 2}, }, "dead"}}, SINGLE_FRAME("dead", 0, 0) }},

    // flash jump (battle sprite)
    {{ {"", animation{{ {{160,192,16,16}, 2}, {{176,192,16,16}, 2}, {{192,192,16,16}, 2}, {{208,192,16,16}, 2}, {{224,192,16,16}, 2}, }, "dead"}}, SINGLE_FRAME("dead", 0, 0) }},
    
    // avenger (battle sprite) the magic of mixels... sixels.... or something...
    {{ {"", animation{{ {{160,208,17,17}, 2}, {{192,208,17,17}, 2}, } }} }},

    // chest (play state) - yes this is implemented as a switch...
    {{ SINGLE_FRAME("off", 224, 0), SINGLE_FRAME("on", 224, 48), {"off-on", animation{{ {{224,16,16,16}, 1}, {{224,32,16,16}, 1} }, "on" } } }},

    // slime blood
    {{ {"", animation{{ {{ {150, 150, 4, 4}, 2  }}} }} }},
    // bone particle
    { { {"", animation{{ {{ {160, 144, 4, 3}, 2  }}} }} }},
    // ghost particle
    { { {"", animation{{ {{ {176, 144, 3, 3}, 2  }}} }} }},

    // gold spike (battle sprite)
    {{ SINGLE_FRAME("", 192, 144) }},

    /* 25 */ {{ {"", animation{{ {{224,192,16,16}, 2}, {{224 + 16,192,16,16}, 2}, {{224 + 32,192,16,16}, 2}, {{224 + 16,192,16,16}, 2} } } } }},



};

const animation_set* get_animation_set(uint32_t id) {
    if (id < sizeof(SPRITES) / sizeof(SPRITES[0])) {
        return &SPRITES[id];
    } else {
        return nullptr;
    }
}