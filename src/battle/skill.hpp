#pragma once

#include <string>
#include <cstdint>
#include "../tilemap.hpp"

struct battle_field;
struct battle_character;

struct skill {
    virtual std::string name() const = 0;
    virtual uint32_t cooldown() const = 0;
    virtual uint32_t cast_time() const = 0;

    virtual void update(battle_field& field, battle_character& owner) = 0;
    virtual bool can_use(battle_character& owner) const = 0;
    virtual void use(battle_field& field, battle_character& owner) = 0;
};

struct sk_flash_jump : public skill {
    std::string name() const override {
        return "Flash Jump";
    }

    uint32_t cooldown() const override {
        return 0;
    }

    uint32_t cast_time() const override {
        return 0;
    }

    virtual bool can_use(battle_character& owner) const override;

    virtual void use(battle_field& field, battle_character& owner) override;

    virtual void update(battle_field& field, battle_character& owner) override;

    uint32_t remaining = 0;
};

struct sk_avenger : public skill {
    std::string name() const override {
        return "Avenger";
    }

    uint32_t cooldown() const override {
        return 0;
    }

    uint32_t cast_time() const override {
        return 30;
    }

    virtual bool can_use(battle_character& owner) const override {
        return true;
    }

    virtual void use(battle_field& field, battle_character& owner) override;

    virtual void update(battle_field& field, battle_character& owner) override;

    uint32_t windup_counter = 0;
    bool thrown = true;
};

struct sk_double_throw : public skill {
    std::string name() const override {
        return "Double Throw";
    }

    uint32_t cooldown() const override {
        return 0;
    }

    uint32_t cast_time() const override {
        return 12;
    }

    virtual bool can_use(battle_character& owner) const override {
        return true;
    }

    virtual void use(battle_field& field, battle_character& owner) override;

    virtual void update(battle_field& field, battle_character& owner) override;

    void spawn_projectile(battle_field& field, battle_character& owner, bool primary);

    uint32_t proj_thrown = 0;
    uint32_t proj2_timer = 0;
    direction use_direction = left;
};

struct ragworm_meteor : public skill {
    std::string name() const override {
        return "meteor";
    }

    uint32_t cooldown() const override {
        return 30 * 5;
    }

    uint32_t cast_time() const override {
        return 30 * 5;
    }

    virtual bool can_use(battle_character& owner) const override {
        return true;
    }

    virtual void use(battle_field& field, battle_character& owner) override;

    virtual void update(battle_field& field, battle_character& owner) override;
};

struct ragworm_burst : public skill {
    uint32_t knives = 0;

    std::string name() const override {
        return "burst";
    }

    uint32_t cooldown() const override {
        return 30 * 5;
    }

    uint32_t cast_time() const override {
        return 90;
    }

    virtual bool can_use(battle_character& owner) const override {
        return true;
    }

    virtual void use(battle_field& field, battle_character& owner) override;
    virtual void update(battle_field& field, battle_character& owner) override;
};

struct ragworm_teleport : public skill {

    std::string name() const override {
        return "teleport";
    }

    uint32_t cooldown() const override {
        return 0;
    }

    uint32_t cast_time() const override {
        return 0;
    }

    virtual bool can_use(battle_character& owner) const override {
        return true;
    }

    virtual void use(battle_field& field, battle_character& owner) override;
    virtual void update(battle_field& field, battle_character& owner) override;

};