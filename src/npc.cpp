#include "npc.hpp"

#include <cassert>
#include <fmt/format.h>

#include "gamestate.hpp"
#include "world.hpp"

void hub_spawnroom_npc(npc_context& context)
{
    context.say("Something isn't right...");
    context.say("The elder went into the East Tower\na while ago, but he hasn't come back yet.");
    context.say("I think someone should check on him.");
    //context.say("By the way, did you know that you can\nattack by pressing X and jump by pressing ALT?");
    //context.say("At least, I read that somewhere, but I\nhaven't figured out what it means yet.");
}

void generic_door(npc_context& context)
{
    if (!context.self()->open)
    {
        context.play_sound("door");
    }
    context.self()->set_doorstate(true);
}

void generic_metal_door(npc_context& context)
{
    if (!context.self()->open)
    {
        context.play_sound("metal_door");
    }
    context.self()->set_doorstate(true);
}

void east_tower_basement_door(npc_context& context)
{
    if (context.get_flag("east_tower_basement_door") == 1)
    {
        if (!context.self()->open)
        {
            context.play_sound("metal_door");
        }
        context.self()->set_doorstate(true);
    }
    else
    {
        context.say("This door won't open without a key.");
    }
}

void east_tower_key_npc(npc_context& context)
{
    if (context.get_flag("east_tower_basement_door"))
    {
        context.say("I already gave you the key. Did you hit your head?");
    }
    else
    {
        context.say("You're trying to get into the East Tower?\nI have the key here. You may use it.");
        context.set_flag("east_tower_basement_door", 1);
    }
}

void change_to_light(npc_context& context)
{
    if (context.self()->type != et_light)
    {
        context.self()->type = et_light;
        context.self()->set_sprite_id(10);
        context.play_sound("torch");
    }
}

void dead_body(npc_context& context)
{
    context.say("The corpse is still warm.\nThis person was killed recently.");
}

void placeholder(npc_context& context)
{
    context.say("You found a corndog.");
}

void switch_test(npc_context& context)
{
    context.play_sound("switch");
    context.self()->set_switchstate(!context.self()->get_switchstate());
}

void puzzle_door(npc_context& context)
{
    if (!context.self()->open)
    {
        context.say("This door is firmly shut. There's no way to open it manually.");
    }
}

bool is_puzzle_door_solved(npc_context& context)
{
    return context.get_flag("puzzle_door_switch1") == 1 && context.get_flag("puzzle_door_switch2") == 0 && context.get_flag("puzzle_door_switch3") == 1;
}

void check_puzzle_door_solution(npc_context& context)
{
    if (is_puzzle_door_solved(context))
    {
        entity* door = context.get_entity("puzzle_door");
        assert(door);
        door->set_doorstate(true);
        context.play_sound("chain_mechanism");
    }
}

void puzzle_door_switch(npc_context& context, const char* name)
{
    if (!is_puzzle_door_solved(context))
    {
        context.self()->toggle_switchstate();
        context.set_flag(name, context.self()->get_switchstate());
        check_puzzle_door_solution(context);
        context.play_sound("switch");
    }
}

void puzzle_door_switch1(npc_context& context)
{
    puzzle_door_switch(context, "puzzle_door_switch1");
}

void puzzle_door_switch2(npc_context& context)
{
    puzzle_door_switch(context, "puzzle_door_switch2");
}

void puzzle_door_switch3(npc_context& context)
{
    puzzle_door_switch(context, "puzzle_door_switch3");
}

void fj_room_torch(npc_context& context)
{
    if (context.self()->type != et_light)
    {
        context.self()->type = et_light;
        context.self()->set_sprite_id(10);
        context.play_sound("torch");
        context.set_flag("fj_room_torch", context.get_flag("fj_room_torch") + 1);
        if (context.get_flag("fj_room_torch") == 6)
        {
            entity* chest = context.get_entity("fj_chest");
            assert(chest);
            chest->set_active(true);
        }
    }
}

void fj_chest(npc_context& context)
{
    // chests are actually one-time switches lol
    if (!context.self()->get_switchstate())
    {
        context.session()->has_flashjump = true;
        context.say("Learned Flash Jump!");
        context.say("Press JUMP again when you're near the\napex of your jump to propel yourself forward.");
        context.self()->toggle_switchstate();
        context.play_sound("chest");
        context.schedule(13, 5);
        context.set_encounter_state(false);
    }
}

void light_path(npc_context& context)
{
    uint32_t current_torch = context.get_flag("light_path");
    context.set_flag("light_path", current_torch + 1);

    if (current_torch == 6)
    {
        context.set_encounter_state(true);
        return;
    }

    std::string current_torch_name = fmt::format("maze_torch_{}", current_torch);
    entity* torch = context.get_entity(current_torch_name.c_str());
    assert(torch);
    torch->type = et_light;
    torch->set_sprite_id(10);
    context.play_sound("torch");
    context.schedule(13, 3);
}

void dungeon_elder(npc_context& context)
{
    if (context.get_flag("elder_pendant") == 0)
    {
        context.say("You must stop Ragworm...");
        context.say("He broke out of the tower and went north to the Sanctum of Thought.");
        context.say("Take this pendant, it will allow passage to the temple...");
        context.set_flag("elder_pendant", 1);
    }
    else
    {
        context.say("The elder has passed on.");
    }
}

void barely_breathing(npc_context& context)
{
    context.say("She's barely breathing.");
}

void elder_door(npc_context& context)
{
    if (context.get_flag("elder_pendant") == 0)
    {
        context.say("I should speak to the elder first.");
    }
    else if (!context.self()->open)
    {
        context.self()->set_doorstate(true);
        context.play_sound("metal_door");
    }
}

void coffee_man(npc_context& context)
{
    if (context.get_flag("elder_pendant") == 0)
    {
        context.say("I wish I had some coffee, man.");
    }
    else
    {
        context.say("Coffee...");
    }
}

void sanctum_hub_door(npc_context& context)
{
    if (context.get_flag("elder_pendant") == 0)
    {
        context.say("This door is protected by an ancient power.");
    }
    else if (!context.self()->open)
    {
        context.self()->set_doorstate(true);
        context.play_sound("metal_door");
    }
}

void avenger_chest(npc_context& context)
{
    if (!context.self()->get_switchstate())
    {
        context.session()->has_avenger = true;
        context.say("Learned Avenger!");
        context.say("Press C to throw a giant fiery star.");
        context.self()->toggle_switchstate();
        context.play_sound("chest");
    }
}

void oracle_1(npc_context& context)
{
    context.say("Welcome to the Sanctum of Thought, where memories\nof all origins converge.");
    context.say("I am aware that you've come here following a monster\ncalled Ragworm. You must navigate the amalgamation\nof thought to find what you seek.");
}

void oracle_2(npc_context& context)
{
    if (context.get_flag("oracle_2") == 0)
    {
        context.say("It seems you are a very capable fighter to make it this far.\nI shall grant you my power for the coming fight.");
        context.session()->has_shadowpartner = true;
        context.set_flag("oracle_2", 1);
    }
    else
    {
        context.say("I have nothing more to give you.");
    }
}

void ragworm(npc_context& context)
{
    context.say("AM I YOU? ARE YOU ME?");
    context.say("ARE WE ALL RAGWORM?");
    context.say("HAHAHAHAHAHAHAHA!!");
    // the final hack
    // this initiates a scripted encounter
    context.say("#ragworm");
}

void devs_reference(npc_context& context)
{
    context.say("-points-");
    context.say("Ture che qutatu.");
}

npc_interact_script get_npc_interact_script(uint32_t id)
{
    switch (id)
    {
    case 0:
        return hub_spawnroom_npc;
    case 1:
        return generic_door;
    case 2:
        return east_tower_basement_door;
    case 3:
        return east_tower_key_npc;
    case 4:
        return change_to_light;
    case 5:
        return switch_test;
    case 6:
        return dead_body;
    case 7:
        return puzzle_door;
    case 8:
        return puzzle_door_switch1;
    case 9:
        return puzzle_door_switch2;
    case 10:
        return puzzle_door_switch3;
    case 11:
        return fj_room_torch;
    case 12:
        return fj_chest;
    case 13:
        return light_path;
    case 14:
        return dungeon_elder;
    case 15:
        return barely_breathing;
    case 16:
        return elder_door;
    case 17:
        return coffee_man;
    case 18:
        return sanctum_hub_door;
    case 19:
        return avenger_chest;
    case 20:
        return oracle_1;
    case 21:
        return oracle_2;
    case 22:
        return ragworm;
    case 23:
        return devs_reference;
    case 24:
        return generic_metal_door;
    default:
        assert(0 && "invalid script ID");
    case 1337:
        return placeholder;
    }
}
