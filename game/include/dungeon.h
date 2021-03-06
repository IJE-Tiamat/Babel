/*
 * Class that represents the Dungeon
 *
 * Author: Tiamat
 * Date: 21/06/2015
 * License: LGPL. No copyright.
 */
#ifndef DUNGEON_H
#define DUNGEON_H

#include "direction.h"
#include "room.h"
#include <core/level.h>
#include <core/listener.h>
#include <list>

using std::list;

class Bitmap;
class KeyboardEvent;
class Texture;

#define MAXT 20

class Dungeon : public Level, Listener
{
public:
    Dungeon(int slot, int steps = 0, int probability_combat = 10);
    ~Dungeon();

    void move_backward();
    void move_forward();

    void turn_left();
    void turn_right();

    bool on_event(const KeyboardEvent& event);

    typedef enum { MOVING, WAITING, DOOR } State;

private:
    void steps_to_foward();
    void steps_to_backward();
    void pass_door();

    void update_self(unsigned long);
    void draw_self();

    void load_map();
    void load_tiles();

    void calculate_probability_combat();
    void save_position();

    int m_x, m_y, m_w, m_h;
    int m_slot, m_steps, m_delta, m_probability_combat;
    unsigned long m_last;
    Direction m_direction;
    State m_state;
    int m_levels;
    bool m_front_blocked;
    Room **m_rooms;
    string m_actual_floor;
    pair<int, int> m_in, m_out;

    shared_ptr<Bitmap> m_tiles[MAXT + 1];
    shared_ptr<Settings> m_settings;
    int m_door;
    Bitmap *m_screen;
    
    list<Rect> planes(int w, int h, double& ratio);
    Rect center(int sw, int sh) const;
};

#endif
