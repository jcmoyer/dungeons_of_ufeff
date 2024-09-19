#include "game.hpp"

#include <GL/gl3w.h>
#include <algorithm>
#include <print>

#include "dialoguebox.hpp"
#include "global_services.hpp"
#include "mathutil.hpp"
#include "random.hpp"
#include "rectangle.hpp"
#include "tilemap.hpp"
#include "tooltip.hpp"

constexpr int scales[] = {1, 2, 3, 4, 5};

const int UPDATE_RATE = 30;

game::game()
{
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
    {
        std::println("error initializing SDL");
        std::exit(EXIT_FAILURE);
    }

    if (SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3) != 0)
    {
        std::println("could not set SDL_GL_CONTEXT_MAJOR_VERSION=3");
        std::exit(EXIT_FAILURE);
    }

    if (SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3) != 0)
    {
        std::println("could not set SDL_GL_CONTEXT_MINOR_VERSION=3");
        std::exit(EXIT_FAILURE);
    }

    window = SDL_CreateWindow("dungeons of ufeff", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 2 * INTERNAL_WIDTH, 2 * INTERNAL_HEIGHT, SDL_WINDOW_OPENGL);

    if (!window)
    {
        std::println("error creating window");
        std::exit(EXIT_FAILURE);
    }

    context = SDL_GL_CreateContext(window);

    if (gl3wInit() != GL3W_OK)
    {
        std::println("error loading GL extensions");
        std::exit(EXIT_FAILURE);
    }
}

game::~game()
{
    texman.clear();

    if (context)
    {
        SDL_GL_DeleteContext(context);
        context = nullptr;
    }

    if (window)
    {
        SDL_DestroyWindow(window);
        window = nullptr;
    }

    SDL_Quit();
}

void game::init()
{
    audio.init();
    int w_width;
    int w_height;
    SDL_GetWindowSize(window, &w_width, &w_height);

    immr = std::make_unique<imm_renderer>();
    batch = std::make_unique<spritebatch>();
    quad_render = std::make_unique<quad_renderer>();
    screen_render = std::make_unique<screen_renderer>();

    sstate.batch = batch.get();
    sstate.frame_counter = 0;
    sstate.input = &input;
    sstate.texman = &texman;
    sstate.quad_render = quad_render.get();
    sstate.font = &font;
    sstate.audio = &audio;
    sstate.session = &session;

    g_audio = &audio;

    play = std::make_unique<st_play>(this, &sstate);
    mainmenu = std::make_unique<st_mainmenu>(this, &sstate);
    battle = std::make_unique<st_battle>(this, &sstate);
    gameover = std::make_unique<st_gameover>(this, &sstate);
    battlestats = std::make_unique<st_battlestats>(this, &sstate);
    gamewin = std::make_unique<st_gamewin>(this, &sstate);
    options = std::make_unique<st_options>(this, &sstate);

    play->init();
    mainmenu->init();
    battle->init();
    gameover->init();
    battlestats->init();
    gamewin->init();
    options->init();

    transition(transition_to::mainmenu);
    // transition(transition_to::gamewin);

    t_mask = texman.get("assets/mask.png");

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);
    // glDepthFunc(GL_LEQUAL);
    glDepthMask(GL_TRUE);

    perform_layout();

    glGenFramebuffers(1, &scene_framebuf);
    glBindFramebuffer(GL_FRAMEBUFFER, scene_framebuf);

    // allocate storage for framebuffer color
    glGenTextures(1, &scene_color);
    glBindTexture(GL_TEXTURE_2D, scene_color);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, INTERNAL_WIDTH, INTERNAL_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); // KRUNCHY
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);

    // attach to framebuffer
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, scene_color, 0);

    // allocate renderbuffer
    glGenRenderbuffers(1, &scene_renderbuf);
    glBindRenderbuffer(GL_RENDERBUFFER, scene_renderbuf);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, INTERNAL_WIDTH, INTERNAL_HEIGHT);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    // attach to framebuffer
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, scene_renderbuf);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        SDL_Log("FATAL: could not construct framebuffer");
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glGenFramebuffers(1, &mask_framebuf);
    glBindFramebuffer(GL_FRAMEBUFFER, mask_framebuf);

    // allocate storage for framebuffer color
    glGenTextures(1, &mask_color);
    glBindTexture(GL_TEXTURE_2D, mask_color);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, INTERNAL_WIDTH, INTERNAL_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); // KRUNCHY
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);

    // attach to framebuffer
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mask_color, 0);

    // allocate renderbuffer
    glGenRenderbuffers(1, &mask_renderbuf);
    glBindRenderbuffer(GL_RENDERBUFFER, mask_renderbuf);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, INTERNAL_WIDTH, INTERNAL_HEIGHT);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    // attach to framebuffer
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, mask_renderbuf);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        SDL_Log("FATAL: could not construct framebuffer");
    }

    // here we prepare a second scene color texture for cases where we want to apply the mask before presenting it to the screen
    glGenTextures(1, &scene_color_extra);
    glBindTexture(GL_TEXTURE_2D, scene_color_extra);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, INTERNAL_WIDTH, INTERNAL_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); // KRUNCHY
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // cull backfaces
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
}

void game::run()
{
    init();

    running = true;
    // Uint32 last = SDL_GetTicks();
    // Uint32 acc = 0;
    // Uint32 delay = 1000 / UPDATE_RATE;

    double last = SDL_GetTicks();
    double acc = 0;
    double delay = 1000 / (double)UPDATE_RATE;

    // Uint64 p_freq = SDL_GetPerformanceFrequency();

    const Uint32 MAX_SKIP_FRAMES = 4;

    while (running)
    {
        SDL_Event ev;

        while (SDL_PollEvent(&ev))
        {
            handle_event(ev);
        }

        double now = SDL_GetTicks();
        double elapsed = now - last;
        last = now;
        acc += elapsed;

        // if we get more than N frames of time to simulate, discard them
        // this happens in certain situations; perhaps the user is dragging the window...
        // ...or we're doing something dumb like loading assets on the main thread in st_battle ;)
        if (acc > MAX_SKIP_FRAMES * delay)
        {
            // acc = MAX_SKIP_FRAMES * delay + (acc % delay);
            acc = MAX_SKIP_FRAMES * delay;
        }

        while (acc >= delay)
        {
            update();
            acc -= delay;
        }

        const double alpha = acc / (double)delay;

        render(alpha);
    }
}

void game::transition(transition_to t)
{
    gamestate* old = current_st;
    if (current_st)
    {
        current_st->leave();
    }
    switch (t)
    {
    case transition_to::mainmenu:
        current_st = mainmenu.get();
        break;
    case transition_to::play:
        current_st = play.get();
        break;
    case transition_to::battle:
        current_st = battle.get();
        break;
    case transition_to::gameover:
        current_st = gameover.get();
        break;
    case transition_to::battlestats:
        current_st = battlestats.get();
        break;
    case transition_to::gamewin:
        current_st = gamewin.get();
        break;
    case transition_to::options:
        current_st = options.get();
        break;
    default:
        assert(0 && "bad transition_to value");
        break;
    }
    current_st->enter(old);
}

void game::transition(gamestate* t)
{
    if (current_st == t)
    {
        assert(false && "attempted to transition to same state");
        return;
    }

    gamestate* old = current_st;
    if (old)
    {
        old->leave();
    }
    current_st = t;
    current_st->enter(old);
}

void game::handle_event(const SDL_Event& ev)
{
    if (ev.type == SDL_QUIT)
    {
        running = false;
    }

    else if (ev.type == SDL_MOUSEMOTION)
    {
        input.mouse_x = ev.motion.x;
        input.mouse_y = ev.motion.y;
    }

    else if (ev.type == SDL_MOUSEBUTTONDOWN)
    {
        input.mouse_x = ev.button.x;
        input.mouse_y = ev.button.y;
    }

    else if (ev.type == SDL_MOUSEBUTTONUP)
    {
        input.mouse_x = ev.button.x;
        input.mouse_y = ev.button.y;
    }

    else if (ev.type == SDL_KEYDOWN)
    {
        const int KEY = ev.key.keysym.sym;
        input.keys[KEY] = true;

        if (KEY == SDLK_RETURN && (ev.key.keysym.mod & KMOD_LALT))
        {
            toggle_fullscreen();
        }
    }

    else if (ev.type == SDL_KEYUP)
    {
        const int KEY = ev.key.keysym.sym;
        input.keys[KEY] = false;
    }

    current_st->handle_event(ev);
}

void game::update()
{
    int mouse_x, mouse_y;
    SDL_GetMouseState(&mouse_x, &mouse_y);

    int window_w, window_h;
    SDL_GetWindowSize(window, &window_w, &window_h);

    current_st->update();

    ++frame_counter;
    sstate.frame_counter = frame_counter;
}

void game::render(double a)
{
    int window_width, window_height;
    SDL_GetWindowSize(window, &window_width, &window_height);

    glViewport(0, 0, INTERNAL_WIDTH, INTERNAL_HEIGHT);

    // default to scene for backwards comapt
    render_to_scene();
    current_st->render(a);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, window_width, window_height);

    glDisable(GL_DEPTH_TEST);

    screen_render->set_output_dimensions(window_width, window_height);
    screen_render->begin(mask_effect);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, scene_color);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, mask_color);

    screen_render->draw_quad({0, 0, window_width, window_height});

    glActiveTexture(GL_TEXTURE0);

    SDL_GL_SwapWindow(window);
}

void game::perform_layout()
{
    int window_width, window_height;
    SDL_GetWindowSize(window, &window_width, &window_height);

    immr->set_output_dimensions(INTERNAL_WIDTH, INTERNAL_HEIGHT);
    batch->set_output_dimensions(INTERNAL_WIDTH, INTERNAL_HEIGHT);
    // water_render->set_output_dimensions(INTERNAL_WIDTH, INTERNAL_HEIGHT);
    quad_render->set_output_dimensions(INTERNAL_WIDTH, INTERNAL_HEIGHT);
    // screen_render->set_output_dimensions(INTERNAL_WIDTH, INTERNAL_HEIGHT);
}

timer game::create_timer(double duration_sec)
{
    return {frame_counter, frame_counter + static_cast<uint32_t>(duration_sec * UPDATE_RATE)};
}

void game::render_to_scene()
{
    glBindFramebuffer(GL_FRAMEBUFFER, scene_framebuf);
}

void game::render_to_mask()
{
    glBindFramebuffer(GL_FRAMEBUFFER, mask_framebuf);
}

void game::clear_mask()
{
    render_to_mask();
    glClearColor(1, 1, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT);
}

void game::set_mask_effect(float amt)
{
    mask_effect = amt;
}

void game::apply_mask()
{
    render_to_scene();

    // we have to detach the current color attachment here to avoid UB (can't read/write simultaneously)
    // so we replace it with the backup texture
    // swap the two (from here, scene_color_extra contains the old buffer)
    std::swap(scene_color, scene_color_extra);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, scene_color, 0);

    glDisable(GL_DEPTH_TEST);

    screen_render->set_output_dimensions(INTERNAL_WIDTH, INTERNAL_HEIGHT);
    screen_render->begin(mask_effect);

    // use the old output as input to the shader
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, scene_color_extra);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, mask_color);

    screen_render->draw_quad({0, 0, INTERNAL_WIDTH, INTERNAL_HEIGHT});

    glActiveTexture(GL_TEXTURE0);
}

st_battle& game::get_battle_state()
{
    return *battle.get();
}

st_battlestats& game::get_battlestats_state()
{
    return *battlestats.get();
}

void game::select_next_resolution()
{
    ++scales_index;
    scales_index %= sizeof(scales) / sizeof(scales[0]);

    SDL_SetWindowSize(window, INTERNAL_WIDTH * scales[scales_index], INTERNAL_HEIGHT * scales[scales_index]);

    perform_layout();

    SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
}

void game::toggle_fullscreen()
{
    Uint32 flags = SDL_GetWindowFlags(window);
    if ((flags & SDL_WINDOW_FULLSCREEN) || (flags & SDL_WINDOW_FULLSCREEN_DESKTOP))
    {
        SDL_SetWindowFullscreen(window, 0);
    }
    else
    {
        SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
    }
}

int game::get_scale() const
{
    return scales[scales_index];
}

glm::ivec2 game::unproject(const glm::ivec2& vec)
{
    int w, h;
    SDL_GetWindowSize(window, &w, &h);

    double scale_x = w / (double)INTERNAL_WIDTH;
    double scale_y = h / (double)INTERNAL_HEIGHT;

    return {static_cast<int>(vec.x / scale_x), static_cast<int>(vec.y / scale_y)};
}

st_options& game::get_options_state()
{
    return *options;
}