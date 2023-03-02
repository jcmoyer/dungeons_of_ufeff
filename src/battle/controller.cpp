#include "controller.hpp"
#include "battle_character.hpp"
#include "battle_field.hpp"
#include <fmt/format.h>
#include "../global_services.hpp"

void slime_controller::think(battle_character& self, battle_field& field) {
    if (!self.can_act()) {
        return;
    }

    if (jump_timer)
        --jump_timer;

    if (self.pos.x > field.player().pos.x) {
        self.move_left();
    } else {
        self.move_right();
    }

    if (self.grounded && jump_timer == 0) {
        int id = random::rand_int(0, 2);
        std::string jump_sound = fmt::format("assets/sound/slime{}.ogg", id);
        g_audio->play_sound(jump_sound.c_str());

        self.jump();
        jump_timer = random::rand_int(100, 200);
    }
}


void skeleton_controller::think(battle_character& self, battle_field& field) {
    if (!self.can_act()) {
        return;
    }

    // only choose a direction to move at 100 units
    if (distance(self.pos.x, self.pos.y, field.player().pos.x, field.player().pos.y) >= 100) {
        if (self.pos.x > field.player().pos.x) {
            self.move_left();
        } else {
            self.move_right();
        }
    } else {
        switch (self.facing) {
        case left:
            self.move_left();
            break;
        default:
            self.move_right();
            break;
        }
    }

    if (!field.player().grounded && self.grounded) {
        self.jump();
    }
}

void bat_controller::think(battle_character& self, battle_field& field) {
    self.set_fly_state(true);

    if (!self.can_act()) {
        return;
    }

    target = { field.player().pos.x, 0 };

    if (st == hover) {
        target += glm::vec2(0, 64.f);
        self.fly_towards(target);
        if (glm::distance(self.pos, target) >= 24.f) {
            if (random::chance(0.5f)) {
                //self.fly_towards(target);
            }
        }
    } else {
        self.fly_towards(target, 0.5f);
    }

    if (state_duration == 0) {
        if (st == hover) {
            st = chase;
            state_duration = 30;
        } else {
            st = hover;
            state_duration = 90;
        }
    } else {
        --state_duration;
    }
}


void ghost_controller::think(battle_character& self, battle_field& field) {
    self.set_fly_state(true);

    if (!self.can_act()) {
        return;
    }

    if (st == rush) {
        self.fly_towards(target);
    }

    if (glm::distance(self.pos, target) < 8.f) {
        self.vel.x = 0;
        self.vel.y = 0;
    }

    if (state_duration == 0) {
        if (st == idle) {
            target = rand_vec2(field.bounds.left, field.bounds.right, field.bounds.floor, 64.f);
            st = rush;
            state_duration = 30;
        } else {
            st = idle;
            state_duration = 90;
        }
    } else {
        --state_duration;
    }
}

void ragworm_controller::think(battle_character& self, battle_field& field) {
    sk_meteor.update(field, self);
    sk_burst.update(field, self);
    sk_teleport.update(field, self);

    if (exhaust_timer) {
        --exhaust_timer;
    } else {
        st = none;
    }

    if (st == exhausted) {
        return;
    }

    if (!self.can_act()) {
        could_act_last_frame = false;
        return;
    } else {
        // just got done using a skill
        if (!could_act_last_frame) {
            st = exhausted;
            exhaust_timer = 30;
        }

        could_act_last_frame = true;
    }

    if (st == walk_towards_player) {
        if (glm::distance(self.pos, walk_target) < 8.f) {
            st = exhausted;
            exhaust_timer = 30;
        }

        if (self.pos.x > walk_target.x) {
            self.move_left();
        } else {
            self.move_right();
        }

        ++frames_walking;

        if (frames_walking == 60) {
            self.use_skill(field, &sk_teleport);
            st = exhausted;
            exhaust_timer = 30;
        }
    } else {
        int choice;
        do {
            choice = random::rand_int(0, 3);
        } while (choice == last_choice);
        last_choice = choice;

        switch (choice) {
        case 0:
            self.use_skill(field, &sk_meteor);
            break;
        case 1:
            self.use_skill(field, &sk_burst);
            break;
        case 2:
            self.use_skill(field, &sk_teleport);
            break;
        case 3:
            st = walk_towards_player;
            frames_walking = 0;
            walk_target = field.player().pos;
            walk_target.y = field.bounds.floor;
            g_audio->play_sound("assets/sound/die.ogg");
            break;
        }
    }
}