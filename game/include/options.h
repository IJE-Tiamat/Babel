/*
 * Class that represents the Options
 *
 * Author: Tiamat
 * Date: 21/06/2015
 * License: LGPL. No copyright.
 */
#ifndef OPTIONS_H
#define OPTIONS_H

#include "button.h"
#include "file.h"
#include <core/level.h>
#include <memory>
#include <map>
#include <vector>

using std::map;
using std::vector;

class Texture;

class Options : public Level
{
public:
    Options(const string& next = "menu");

private:
    map<ObjectID, shared_ptr<Texture>> m_textures;
    map<ObjectID, Button *> m_buttons;

    vector<int> m_resolutions = {800, 1024};

    void draw_self();
    bool on_message(Object *sender, MessageID id, Parameters p);
};

#endif
