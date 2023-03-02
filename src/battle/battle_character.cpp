#include "battle_character.hpp"

void battle_character::use_skill(battle_field& b_field, skill* sk) {
    if (sk->can_use(*this)) {
        sk->use(b_field, *this);

        if (sk->cast_time() > 0) {
            current_skill = sk;
        }

        frames_since_skill_start = 0;
    }
}

void battle_character::use_skill_unconditionally(battle_field& b_field, skill* sk) {
    sk->use(b_field, *this);
}

void battle_character::update(float floor) {
    prev_pos = pos;
    pos += vel;

    if (!alive) {
        flying = false;
    }

    if (!flying) {
        vel.y -= 0.4f;
    }

    if (pos.y <= floor) {
        pos.y = floor;
        vel.y = 0;
        grounded = true;
    } else {
        grounded = false;
    }

    if (flying) {
        grounded = false;
    }

    if (grounded) {
        vel.x *= info->friction;
    }

    if (grounded) {
        frames_since_jump = 0;
    } else {
        ++frames_since_jump;
    }

    if (alive) {
        anim.update();
    }

    if (hitstun_frames) {
        --hitstun_frames;
    }

    ++frames_since_skill_start;
    if (current_skill && frames_since_skill_start == current_skill->cast_time()) {
        current_skill = nullptr;
    }
}