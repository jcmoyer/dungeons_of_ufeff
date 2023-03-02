#include "skill.hpp"

#include "battle_character.hpp"
#include "battle_field.hpp"

void bpb_avenger_update(battle_field& field, battle_projectile& self)
{
    for (int i = 0; i < 4; ++i)
    {
        if (random::chance(0.25f))
        {
            field.particle_sys.emit(25, self.pos, -self.vel * 0.4f + rand_vec2(-1, 1), 1.f);
        }
    }
}

battle_projectile_behavior bpb_avenger{glm::vec2{0, 0.1}, bpb_avenger_update};
battle_projectile_behavior bpb_double_throw{glm::vec2(0, -0.06), nullptr};

void sk_double_throw::use(battle_field& field, battle_character& owner)
{
    proj_thrown = 0;
    proj2_timer = 0;
    use_direction = owner.facing;

    spawn_projectile(field, owner, true);

    glm::vec2 spawn_offset{};
    if (owner.facing == left)
    {
        spawn_offset.x = -16;
    }
    else
    {
        spawn_offset.x = 16;
    }

    field.fx_sys.spawn(15, owner.facing, spawn_offset).attach(field.player_index);
}

void sk_double_throw::update(battle_field& field, battle_character& owner)
{
    ++proj2_timer;

    if (proj_thrown == 1 && proj2_timer == 6)
    {
        spawn_projectile(field, owner, true);
    }
}

void sk_double_throw::spawn_projectile(battle_field& field, battle_character& owner, bool primary)
{
    if (primary)
    {
        ++proj_thrown;
    }

    battle_projectile& p = field.spawn_projectile(14);
    p.prev_pos = p.pos = owner.pos;
    p.owner = owner.id;
    p.pierce = 0;
    p.dir = use_direction;
    p.angle_to_velocity = true;
    p.behavior = &bpb_double_throw;
    p.power = owner.power;

    if (owner.facing == left)
    {
        p.vel = {-5, 0.5};
    }
    else
    {
        p.vel = {5, 0.5};
    }

    g_audio->play_sound("assets/sound/throw1.ogg");
}

bool sk_flash_jump::can_use(battle_character& owner) const
{
    return !owner.grounded && owner.frames_since_jump >= 3 && remaining > 0;
}

void sk_flash_jump::use(battle_field& field, battle_character& owner)
{
    --remaining;

    g_audio->play_sound("assets/sound/flashjump.ogg");

    switch (owner.facing)
    {
    case left:
        owner.vel.x = -7;
        break;
    case right:
        owner.vel.x = 7;
        break;
    default:
        break;
    }
    owner.vel.y += 2;

    glm::vec2 spawn_offset{};
    if (owner.facing == left)
    {
        spawn_offset.x = 8;
    }
    else
    {
        spawn_offset.x = -8;
    }
    field.fx_sys.spawn(18, owner.facing, owner.pos + spawn_offset);
}

void sk_flash_jump::update(battle_field& field, battle_character& owner)
{
    (void)field;

    if (owner.grounded)
    {
        remaining = 2;
    }
}

void sk_avenger::update(battle_field& field, battle_character& owner)
{
    if (thrown)
    {
        return;
    }

    if (windup_counter)
    {
        if (windup_counter % 2 == 0)
        {
            field.particle_sys.emit(25, owner.pos, rand_vec2(-2, 2), 1.f);
        }

        --windup_counter;
    }

    if (windup_counter == 0)
    {
        battle_projectile& p = field.spawn_projectile(19);
        p.prev_pos = p.pos = owner.pos;
        p.owner = owner.id;
        p.pierce = 999999;
        p.behavior = &bpb_avenger;
        if (owner.facing == left)
        {
            p.vel = {-6, 0};
        }
        else
        {
            p.vel = {6, 0};
        }
        p.dir = owner.facing;
        p.power = owner.power;
        thrown = true;
    }
}

void sk_avenger::use(battle_field& field, battle_character& owner)
{
    (void)field;
    (void)owner;

    g_audio->play_sound("assets/sound/avenger.ogg");
    windup_counter = 15;
    thrown = false;
}

void ragworm_meteor::use(battle_field& field, battle_character& owner)
{
    for (int i = 0; i < 10; ++i)
    {
        auto& p = field.spawn_projectile(13);
        p.prev_pos = p.pos = rand_vec2(field.bounds.left, field.bounds.right, 100, 200);
        p.vel = {0, -1.0 - random::rand_real()};
        p.owner = owner.id;
    }

    g_audio->play_sound("assets/sound/torch.ogg");
}

void ragworm_meteor::update(battle_field& field, battle_character& owner)
{
    if (owner.current_skill == this)
    {
        field.particle_sys.emit(13, owner.pos, rand_vec2(-1, 1, 0, 2), 1.f);
    }
}

void bpb_burst_update(battle_field& field, battle_projectile& self)
{
    // very hack
    ++self.pierce;

    if (self.pierce < 30)
    {
        self.vel *= 0.97f;
    }
    if (self.pierce == 30)
    {
        g_audio->play_sound("assets/sound/knifedraw.ogg");
        self.vel = field.player().pos - self.pos;
        self.vel = glm::normalize(self.vel);
    }
    if (self.pierce > 30)
    {
        self.vel *= 1.08f;
    }
}
battle_projectile_behavior bpb_burst{glm::vec2{0, 0}, bpb_burst_update};

void ragworm_burst::use(battle_field& field, battle_character& owner)
{
    (void)field;
    (void)owner;
    knives = 10;
}

void ragworm_burst::update(battle_field& field, battle_character& owner)
{
    if (knives)
    {
        --knives;
    }
    else
    {
        return;
    }

    auto& p = field.spawn_projectile(24);
    p.behavior = &bpb_burst;
    p.prev_pos = p.pos = owner.pos;
    p.vel = {random::rand_real(-0.5, 0.5), 2.5f};
    p.owner = owner.id;
    p.angle_to_velocity = true;
    p.pierce = 0;

    g_audio->play_sound("assets/sound/throw1.ogg");
}

void ragworm_teleport::use(battle_field& field, battle_character& owner)
{
    glm::vec2 src = owner.pos;
    glm::vec2 dst;

    do
    {
        dst = {
            (field.bounds.left + field.bounds.right) / 2 + random::rand_real(-100, 100),
            random::rand_real(field.bounds.floor, field.bounds.floor + 32)};
    } while (glm::distance(dst, field.player().pos) < 32.f);

    field.fx_sys.spawn(18, right, src);
    field.fx_sys.spawn(18, right, dst);
    owner.warp(dst);

    g_audio->play_sound("assets/sound/warpspell.ogg");
}

void ragworm_teleport::update(battle_field& field, battle_character& owner)
{
    (void)field;
    (void)owner;
}
