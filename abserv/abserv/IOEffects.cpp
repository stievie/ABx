#include "stdafx.h"
#include "IOEffects.h"
#include <pugixml.hpp>

namespace IO {

bool IOEffects::Load(Game::EffectManager& manager, const std::string fileName)
{
    pugi::xml_document doc;
    const pugi::xml_parse_result& result = doc.load_file(fileName.c_str());
    if (result.status != pugi::status_ok)
        return false;
    const pugi::xml_node& skillsNode = doc.child("effects");
    if (!skillsNode)
        return false;

    for (pugi::xml_node_iterator it = skillsNode.begin(); it != skillsNode.end(); ++it)
    {
        uint32_t id = 0;
        std::string name = "";
        if (const pugi::xml_attribute& _attr = (*it).attribute("id"))
        {
            id = _attr.as_uint();
        }
        else
            return false;
        if (const pugi::xml_attribute& _attr = (*it).attribute("name"))
        {
            name = _attr.value();
        }
        else
            return false;
        if (const pugi::xml_attribute& _attr = (*it).attribute("script"))
        {
            manager.effects_[id] = _attr.value();
            manager.effectNames_[name] = id;
        }
        else
            return false;
    }

    return true;
}

}
