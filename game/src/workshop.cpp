/*
 * Workshop class implementation
 *
 * Author: Tiamat
 * Date: 21/06/2015
 * License: LGPL. No copyright.
 */
#include "colony.h"
#include "workshop.h"
#include <core/font.h>
#include <core/rect.h>
#include <core/settings.h>

#define W 1024.0
#define H 768.0

using std::to_string;

Workshop::Workshop(int slot, const string& next)
    : Level("workshop", next), m_slot(slot), m_screen(CHAT), m_chat_text(nullptr)
{
    Environment *env = Environment::get_instance();
    string path = "res/images/colony/";
    m_textures["scenario"] = env->resources_manager->get_texture(path +
        "workshop/chat_scenario.png");
    m_textures["right_bracket"] = env->resources_manager->get_texture(path + "right_bracket.png");
    m_textures["left_bracket"] = env->resources_manager->get_texture(path + "left_bracket.png");

    Colony *colony = new Colony(slot, this, "workshop");
    colony->add_observer(this);
    add_child(colony);

    welcome();
    create_buttons();
}

void
Workshop::welcome()
{
    Environment *env = Environment::get_instance();
    shared_ptr<Font> font = env->resources_manager->get_font("res/fonts/exo-2/Exo2.0-Regular.otf");
    env->canvas->set_font(font);
    font->set_size(18);

    Color color(170, 215, 190);
    double scale_w = env->canvas->w() / W;
    double scale_h = env->canvas->h() / H;

    shared_ptr<Settings> settings = env->resources_manager->get_settings("res/datas/slot" +
        to_string(m_slot) + "/colony.sav");
    auto sections = settings->sections();
    string text = sections["Workshop"]["welcome"];

    Rect area(305 * scale_w, 605 * scale_h, +670 * scale_w, 116 * scale_h);
    m_chat_text = new TextBox(this, area, text, color);
    m_chat_text->set_colors(color);
}

void
Workshop::draw_self()
{
    Environment *env = Environment::get_instance();
    env->canvas->clear();

    double scale_w = env->canvas->w() / W;
    double scale_h = env->canvas->h() / H;

    env->canvas->draw(m_textures["scenario"].get(), 275 * scale_w, 173 * scale_h);
    env->canvas->draw(m_textures["right_bracket"].get(), 275 * scale_w, 173 * scale_h);
    env->canvas->draw(m_textures["left_bracket"].get(), 28 * scale_w, 175 * scale_h);

    switch (m_screen)
    {
        case CHAT:
            change_to_chat();
            break;

        case DRONE:
            change_to_drone();
            break;

        case VEHICLE:
            change_to_vehicle();
            break;

        case ARSENAL:
            change_to_arsenal();
            break;
    }
}

bool
Workshop::on_message(Object *sender, MessageID id, Parameters)
{
    Button *button = dynamic_cast<Button *>(sender);

    if (id != Button::clickedID or not button)
    {
        Colony *colony = dynamic_cast<Colony *>(sender);
        if (not colony)
        {
            return false;
        }

        set_next(id);
        finish();
    }
    else if (button->id() != "workshop")
    {
        Environment *env = Environment::get_instance();
        string path = "res/images/colony/workshop/";

        m_textures["scenario"] = env->resources_manager->get_texture(path + "scenario.png");
        change_buttons();

        if (button->id() == "chat")
        {
            m_screen = CHAT;
            m_textures["scenario"] = env->resources_manager->get_texture(path +
                "chat_scenario.png");
        }
        else if (button->id() == "drone")
        {
            m_screen = DRONE;
        }
        else if (button->id() == "vehicle")
        {
            m_screen = VEHICLE;
        }
        else if (button->id() == "arsenal")
        {
            m_screen = ARSENAL;
        }

        button->change_state(Button::ACTIVE);
    }

    return true;
}

void
Workshop::create_buttons()
{
    Environment *env = Environment::get_instance();
    string path = "res/images/colony/";

    shared_ptr<Font> font = env->resources_manager->get_font("res/fonts/exo-2/Exo2.0-Regular.otf");
    env->canvas->set_font(font);
    font->set_size(24);
    
    const int x = (28 / W) * env->canvas->w();
    const int w = (190 / W) * env->canvas->w();
    const int h = (180/3 / H) * env->canvas->h();

    Button *button = new Button(this, "workshop", path + "workshop_button.png",
        x, (218 / H) * env->canvas->h(), w, h);
    button->set_sprites(3);
    button->change_state(Button::ACTIVE);

    m_buttons[button->id()] = button;

    button = new Button(this, "chat", path + "colony_small_button.png",
        x, (322 / H) * env->canvas->h(), w, h);
    button->set_sprites(3);
    button->set_text("Chat");
    button->change_state(Button::ACTIVE);

    m_buttons[button->id()] = button;

    button = new Button(this, "drone", path + "colony_small_button.png",
        x, (427 / H) * env->canvas->h(), w, h);
    button->set_sprites(3);
    button->set_text("Drone");

    m_buttons[button->id()] = button;

    button = new Button(this, "vehicle", path + "colony_small_button.png",
        x, (531 / H) * env->canvas->h(), w, h);
    button->set_sprites(3);
    button->set_text("Vehicle");

    m_buttons[button->id()] = button;

    button = new Button(this, "arsenal", path + "colony_small_button.png",
        x, (635 / H) * env->canvas->h(), w, h);
    button->set_sprites(3);
    button->set_text("Arsenal");

    m_buttons[button->id()] = button;

    for (auto b : m_buttons)
    {
        b.second->add_observer(this);
        add_child(b.second);
    }
}

void
Workshop::change_buttons()
{
    for (auto b : m_buttons)
    {
        if (b.first != "workshop")
        {
            b.second->change_state(Button::IDLE);
        }
    }
}

void
Workshop::change_to_chat()
{
    m_chat_text->draw();
}

void
Workshop::change_to_drone()
{
}

void
Workshop::change_to_vehicle()
{
    string path = "res/images/colony/";
    Color color(170, 215, 190);

    Environment *env = Environment::get_instance();
    shared_ptr<Font> font = env->resources_manager->get_font("res/fonts/exo-2/Exo2.0-Regular.otf");
    env->canvas->set_font(font);
    font->set_size(18);

    env->canvas->draw("Name", (360 / W) * env->canvas->w(), (188 / H) * env->canvas->h(), color);
    shared_ptr<Texture> texture = env->resources_manager->get_texture(
        path + "icons/matter_energy.png");
    env->canvas->draw(texture.get(), (690 / W) * env->canvas->w(), (188 / H) * env->canvas->h());
    env->canvas->draw("Qnt.", (855 / W) * env->canvas->w(), (186 / H) * env->canvas->h(), color);

    shared_ptr<Settings> settings = env->resources_manager->get_settings("res/datas/slot" +
        to_string(m_slot) + "/vehicles.sav");
    auto sections = settings->sections();

    int y = 236;
    for (auto section : sections)
    {
        string name = section.first;
        string matter = section.second["matter"];
        string energy = section.second["energy"];
        string qnt = section.second["qnt"];

        env->canvas->draw(name, (360 / W) * env->canvas->w(), (y / H) * env->canvas->h(), color);
        if (not matter.empty())
        {
            env->canvas->draw(matter + "/" + energy, (690 / W) * env->canvas->w(),
                (y / H) * env->canvas->h(), color);
        }
        if (not qnt.empty())
        {
            env->canvas->draw(qnt, (855 / W) * env->canvas->w(),
                (y / H) * env->canvas->h(), color);
        }

        texture = env->resources_manager->get_texture(path + "icons/workshop.png");
        Rect clip = Rect(0, 25, 50, 50/2);
        env->canvas->draw(texture.get(), clip, (310 / W) * env->canvas->w(),
            (y / H) * env->canvas->h(), (50 / W) * env->canvas->w(), (25 / H) * env->canvas->h());

        texture = env->resources_manager->get_texture(path + "big_list.png");
        clip = Rect(0, 0, 602, 75/3);
        env->canvas->draw(texture.get(), clip, (310 / W) * env->canvas->w(),
            ((y+5) / H) * env->canvas->h(), (602 / W) * env->canvas->w(),
            (25 / H) * env->canvas->h());

        y += 64;
    }
}

void
Workshop::change_to_arsenal()
{
}
