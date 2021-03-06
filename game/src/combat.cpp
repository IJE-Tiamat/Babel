/*
 * Combat class implementation
 *
 * Author: Tiamat
 * Date: 21/06/2015
 * License: LGPL. No copyright.
 */
#include "action.h"
#include "combat.h"
#include "character.h"
#include <core/font.h>
#include <core/rect.h>
#include <core/settings.h>
#include <core/text.h>
#include <core/keyboardevent.h>
#include <memory>

using std::to_string;
using std::shared_ptr;

#define W 1024.0
#define H 768.0
#define DELAY 1000

Combat::Combat(int slot, const string& next)
    : Level("combat", next), m_slot(slot), m_attacker(""), m_state(ENEMY_ATTACK),
        m_enemy_turn(nullptr), m_text(nullptr), m_last(0), m_data(0)
{
    Environment *env = Environment::get_instance();

    env->events_manager->register_listener(this);
    m_texture = env->resources_manager->get_texture("res/images/combat/arena.png");
    m_turn = env->resources_manager->get_texture("res/images/combat/enemy_turn.png");

    env->sfx->play("res/sfx/uiBattle_Turn1.ogg", 1);

    load_team();
    load_enemies();

    m_action = new Action(m_slot, this);
    m_action->add_observer(this);
    add_child(m_action);
    m_action->set_visible(false);

    current_action.first = "";
    current_action.second = "";
}

Combat::~Combat()
{
    if (m_text)
    {
        delete m_text;
        m_text = nullptr;
    }

    Environment *env = Environment::get_instance();
    env->events_manager->unregister_listener(this);
}

void
Combat::update_self(unsigned long elapsed)
{
    Environment *env = Environment::get_instance();

    if (m_state == FINISHED_COMBAT)
    {
        return;
    }
    else if (m_characters.empty())
    {
        set_next("base");
        m_result = env->resources_manager->get_texture("res/images/combat/you-lose.png");
        m_state = FINISHED_COMBAT;

        for (auto it : m_enemies)
        {
            it.second->set_active(false);
            it.second->set_visible(false);
        }

        return;
    }
    else if (m_enemies.empty())
    {
        set_next("dungeon");
        m_result = env->resources_manager->get_texture("res/images/combat/you-win.png");
        m_state = FINISHED_COMBAT;

        for (auto it : m_characters)
        {
            it.second->set_visible(false);
        }

        return;
    }

    if (not m_last or m_state == CHARACTER_ATTACK)
    {
        m_last = elapsed;
        return;
    }
    else if (elapsed - m_last < DELAY and m_state != EXECUTE)
    {
        return;
    }
    else if (elapsed - m_last < DELAY * 2 and m_state != EXECUTE)
    {
        m_state = ENEMY_ATTACK;
        return;
    }

    m_last = elapsed;
    m_attacker = m_attackers.begin()->second;
    
    Character *enemy = (m_enemies.find(m_attacker) != m_enemies.end() ?
        m_enemies[m_attacker] : nullptr);
    Character *character = (m_characters.find(m_attacker) != m_characters.end() ?
        m_characters[m_attacker] : nullptr);

    if (enemy)
    {
        m_action->set_visible(false);

        set_initial_position();
        
        enemy_attack(enemy);
        m_enemy_turn = enemy;
    }
    else if (character)
    {
        character->set_defense_mode(false);
        m_action->set_visible(true);
        m_action->set_current_character(character);

        for (auto character : m_characters)
        {
            character.second->set_active(false);
            character.second->set_visible(false);
        }

        set_attacker_position(character);

        m_state = CHARACTER_ATTACK;
        m_enemy_turn = nullptr;
    }
}

void
Combat::draw_self()
{
    Environment *env = Environment::get_instance();
    env->canvas->clear();
    env->canvas->draw(m_texture.get());

    env->canvas->set_blend_mode(Canvas::BLEND);
    env->canvas->fill(Rect(0, 0, env->canvas->w(),  env->canvas->h()), Color(0, 0, 0, 128));
    env->canvas->set_blend_mode(Canvas::NONE);

    if (m_state == SHOW_DAMAGE)
    {
        m_text->draw();
    }
    else if (m_state == FINISHED_COMBAT)
    {
        set_data_result();
        int x = ((env->canvas->w() - m_result->w()) / 2 / W) * env->canvas->w();
        int y = ((env->canvas->h() - m_result->h()) / 2 / H) * env->canvas->h();

        env->canvas->draw(m_result.get(), x, y);
        m_text->draw();

    }
    
    if (m_enemy_turn and m_state != FINISHED_COMBAT)
    {
        double scale_w = env->canvas->w() / W;
        double scale_h = env->canvas->h() / H;
        double x = m_enemy_turn->x() - 10 * scale_w;
        double y = m_enemy_turn->y() + m_enemy_turn->h() - 15 * scale_h;
        
        env->canvas->draw(m_turn.get(), x, y);
    }

    double x = 30 * env->canvas->w() / W;
    auto it = m_attackers.begin();
    for (int i = 1; i <= 12; ++i)
    {
        string attacker = "";

        if (m_enemies.find(it->second) != m_enemies.end())
        {
            string name = it->second;
            name.pop_back();

            attacker = name + "_icon.png";
        }
        else
        {
            attacker = it->second + "_icon.png";
        }

        m_attacker_icon = env->resources_manager->get_texture("res/images/characters/" +
            attacker);

        env->canvas->draw(m_attacker_icon.get(), x, 25 * env->canvas->h() / H);

        if (i > 1)
        {
            double w = m_attacker_icon->w();
            double h= m_attacker_icon->h();
            Rect rect { x, 25 * env->canvas->h() / H, w, h };

            env->canvas->set_blend_mode(Canvas::BLEND);
            env->canvas->fill(rect, Color(0, 0, 0, 128));
            env->canvas->set_blend_mode(Canvas::NONE);
        }
        x += 82 * env->canvas->w() / W;
        ++it;
    }
}

void
Combat::set_data_result()
{
    if (m_text)
    {
        delete m_text;
        m_text = nullptr;
    }

    Environment *env = Environment::get_instance();
    shared_ptr<Font> font = env->resources_manager->get_font("res/fonts/exo-2/Exo2.0-Regular.otf");
    env->canvas->set_font(font);
    font->set_size(20);

    m_text = new Text(this, "You earned " + to_string(m_data) + " Data",
        Color(170, 215, 190));

    double scale_h = env->canvas->h() / H;
    m_text->set_position(env->canvas->w() / 2 - m_text->w() / 2,
        env->canvas->w() / 2 - m_text->h() / 2 - 60 * scale_h);
}

bool
Combat::on_message(Object *sender, MessageID id, Parameters p)
{
    Character *character = dynamic_cast<Character *>(sender);
    Action *action = dynamic_cast<Action *>(sender);

    if (character and m_state == CHARACTER_ATTACK)
    {
        character_message(id);
    }
    else if (action)
    {
        action_message(id, p);
    }

    return true;
}

void
Combat::load_team()
{
    Environment *env = Environment::get_instance();
    string path = "res/datas/slot" + to_string(m_slot) + "/squad.sav";
    shared_ptr<Settings> settings = env->resources_manager->get_settings(path);
    auto heroes = settings->sections()["Squad"];

    for (auto h : heroes)
    {
        if (h.second == "")
        {
            break;
        }
        Character *character = new Character(m_slot, this, h.second, h.second + "_small.png");
        m_characters[character->id()] = character;
    }

    set_initial_position();

    for (auto it : m_characters)
    {
        it.second->add_observer(this);
        it.second->set_active(false);
        add_child(it.second);

        for (int i = 1; i <= 12; ++i)
        {
            it.second->set_attacks_quantity(i);

            int new_cooldown = it.second->cooldown() * i;

            m_attackers.insert(pair<int, string>(new_cooldown, it.second->id()));
        }
    }
}

void
Combat::clear_team(string hero)
{
    Environment *env = Environment::get_instance();
    string path = "res/datas/slot" + to_string(m_slot) + "/squad.sav";
    shared_ptr<Settings> settings = env->resources_manager->get_settings(path);
    auto heroes = settings->sections()["Squad"];

    for (auto h : heroes)
    {
        if (h.second == hero)
        {
            settings->write<string>("Squad", h.first, "");
            break;
        }
    }

    settings->save("res/datas/slot" + to_string(m_slot) + "/squad.sav");
}

void
Combat::load_enemies()
{
    Environment *env = Environment::get_instance();

    double scale_w = env->canvas->w() / W;
    double scale_h = env->canvas->h() / H;

    int w = 170 * scale_w;
    int h = 265 * scale_h;
    int i = 0;

    Character *enemy = new Character(m_slot, this, "Technopus" + to_string(++i), "Technopus.png",
        58 * scale_w, 200 * scale_h, w, h, Character::ENEMY);
    m_enemies[enemy->id()] = enemy;

    enemy = new Character(m_slot, this, "Technopus" + to_string(++i), "Technopus.png",
        194 * scale_w, 247 * scale_h, w, h, Character::ENEMY);
    m_enemies[enemy->id()] = enemy;

    enemy = new Character(m_slot, this, "Technopus" + to_string(++i), "Technopus.png",
        330 * scale_w, 200 * scale_h, w, h, Character::ENEMY);
    m_enemies[enemy->id()] = enemy;

    enemy = new Character(m_slot, this, "Technopus" + to_string(++i), "Technopus.png",
        530 * scale_w, 200 * scale_h, w, h, Character::ENEMY);
    m_enemies[enemy->id()] = enemy;

    enemy = new Character(m_slot, this, "Technopus" + to_string(++i), "Technopus.png",
        665 * scale_w, 247 * scale_h, w, h, Character::ENEMY);
    m_enemies[enemy->id()] = enemy;

    enemy = new Character(m_slot, this, "Technopus" + to_string(++i), "Technopus.png",
        800 * scale_w, 200 * scale_h, w, h, Character::ENEMY);
    m_enemies[enemy->id()] = enemy;

    for (auto it : m_enemies)
    {
        it.second->add_observer(this);
        add_child(it.second);

        for (int i = 1; i <= 12; ++i)
        {
            it.second->set_attacks_quantity(i);

            int new_cooldown = it.second->cooldown() * i;

            m_attackers.insert(pair<int, string>(new_cooldown, it.second->id()));
        }
    }
}

void
Combat::enemy_attack(Character* enemy)
{
    Character *character = m_characters.begin()->second;

    Environment *env = Environment::get_instance();
    
    int damage = character->receive_damage(enemy);
    set_text("-" + to_string(damage), Color::RED);
    m_text->set_position(character->x() + character->w() / 2 - m_text->w() / 2,
        character->y() - m_text->h() - ((10 / H) * env->canvas->h()));

    env->sfx->play("res/sfx/uiTavern_Enforcer.ogg", 1);


    if (character->life() <= 0)
    {
        character->remove_observer(this);
        remove_child(character);

        m_characters.erase(character->id());

        for (auto it = m_attackers.begin(); it != m_attackers.end(); ++it)
        {
            if (it->second == character->id())
            {
                m_attackers.erase(it);
            }
        }
        
        clear_team(character->id());
        delete character;

    }

    update_attackers(enemy);
    m_state = SHOW_DAMAGE;
}

void
Combat::update_attackers(Character* character)
{
    int attacks_quantity = character->attacks_quantity() + 1;
    character->set_attacks_quantity(attacks_quantity);

    int new_cooldown = character->cooldown() * attacks_quantity;
    
    m_attackers.erase(m_attackers.begin());

    m_attackers.insert(pair<int, string>(new_cooldown, character->id()));
}

bool
Combat::on_event(const KeyboardEvent& event)
{
    Environment *env = Environment::get_instance();
    shared_ptr<Settings> settings;
    int data;
    switch (event.state())
    {
        case KeyboardEvent::PRESSED:
            if (m_state == FINISHED_COMBAT)
            {
                env->sfx->play("res/sfx/uiBattle_Escape.ogg", 1);
                finish();
            }
            else if (event.key() == KeyboardEvent::ESCAPE)
            {
                env->sfx->play("res/sfx/uiConfirm1.ogg", 1);
                set_next("dungeon");
                finish();
            }

            settings = env->resources_manager->get_settings("res/datas/slot" +
                to_string(m_slot) + "/colony.sav");


            data = settings->read<int>("Colony", "data", 0);

            data += m_data;

            settings->write<int>("Colony", "data", data);

            settings->save("res/datas/slot" + to_string(m_slot) + "/colony.sav");

            return true;
        default:
            return false;
            break;
    }

    return false;
}

void
Combat::set_text(const string& str, const Color& color)
{
    Environment *env = Environment::get_instance();
    shared_ptr<Font> font = env->resources_manager->get_font("res/fonts/exo-2/Exo2.0-Regular.otf");
    env->canvas->set_font(font);
    font->set_size(30);

    if (m_text)
    {
        delete m_text;
        m_text = nullptr;
    }

    m_text = new Text(this, str, color);
}

void
Combat::set_initial_position()
{
    Environment *env = Environment::get_instance();

    int character_quantity = m_characters.size();

    int delta = (W - character_quantity * 222) / (character_quantity + 1);

    int x = delta;
    int y = (620 / H) * env->canvas->h();

    double scale_w = env->canvas->w() / W;

    for (auto character : m_characters)
    {
        character.second->set_position(x * scale_w, y);
        x += delta + 222;
        character.second->set_visible(true);
    }
}

void
Combat::set_attacker_position(Character *character)
{
    Environment *env = Environment::get_instance();

    int x = (30 / W) * env->canvas->w();
    int y = (620 / H) * env->canvas->h();

    character->set_position(x, y);

    character->set_visible(true);
}

void
Combat::character_message(MessageID id)
{
    if (current_action.first.empty())
    {
        return;
    }

    if (current_action.first == "attack")
    {
        Character *attacker = m_characters[m_attacker];
        Character *enemy = m_enemies[id];

        Environment *env = Environment::get_instance();

        int damage = enemy->receive_damage(attacker);
        set_text("-" + to_string(damage), Color::RED);
        
        m_text->set_position(enemy->x() + enemy->w() / 2 - m_text->w() / 2,
            enemy->y() + enemy->h() + ((10 / H) * env->canvas->h()));

        env->sfx->play("res/sfx/uiTavern_Ghost2.ogg", 1);

        if (enemy->life() <= 0)
        {
            enemy->remove_observer(this);
            remove_child(enemy);

            m_enemies.erase(id);

            for (auto it = m_attackers.begin(); it != m_attackers.end(); ++it)
            {
                if (it->second == id)
                {
                    m_attackers.erase(it);
                }
            }

            m_data += enemy->data();

            delete enemy;
        }
        m_state = SHOW_DAMAGE;
        update_attackers(attacker);
        m_action->set_state(Action::NONE);
        m_action->change_buttons();
        m_action->set_visible(false);
    }
    current_action.first = "";
    current_action.second = "";
}

void
Combat::action_message(MessageID id, Parameters p)
{
    current_action.first = id;
    current_action.second = p;

    if (current_action.first == "rest")
    {
        Character *attacker = m_characters[m_attacker];
        m_state = EXECUTE;
        update_attackers(attacker);
        m_action->set_state(Action::NONE);
    }
    else if (current_action.first == "defense")
    {
        Character *attacker = m_characters[m_attacker];
        attacker->set_defense_mode(true);
        m_state = EXECUTE;
        update_attackers(attacker);
        m_action->set_state(Action::NONE);
    }
    else if (current_action.first == "run")
    {

        for (auto it = m_characters.begin(); it != m_characters.end(); ++it)
        {
            Character *character = it->second;
            character->remove_observer(this);
            remove_child(character);
            m_characters.erase(character->id());
        }
        m_action->set_state(Action::NONE);
        m_action->set_visible(false);
    }
    else if (current_action.first == "item")
    {
        Environment *env = Environment::get_instance();
        shared_ptr<Settings> items = env->resources_manager->get_settings("res/datas/slot" +
            to_string(m_slot) + "/items.sav");

        string attribute = items->read<string>(current_action.second, "attribute", "");
        
        double value = items->read<double>(current_action.second, "value", 0);

        int qnt_earned = items->read<int>(current_action.second, "qnt_earned", 0);

        items->write<int>(current_action.second, "qnt_earned", --qnt_earned);

        items->save("res/datas/slot" + to_string(m_slot) + "/items.sav");

        Character *attacker = m_characters[m_attacker];

        if (attribute == "life")
        {
            value += attacker->life();

            value = value > attacker->max_life() ? attacker->max_life() : value;
            attacker->set_life(value);
        }
        else if (attribute == "might_attack")
        {
            value += attacker->might_attack();
            attacker->set_might_attack(value);
        }

        m_action->set_state(Action::NONE);
        update_attackers(attacker);
        m_state = EXECUTE;
    }
}
