#include "stdafx.h"
#include "IOSkills.h"

namespace IO {

bool IOSkills::Load(Game::SkillManager& manager, const std::string fileName)
{
    pugi::xml_document doc;
    const pugi::xml_parse_result& result = doc.load_file(fileName.c_str());
    if (result.status != pugi::status_ok)
        return false;
    const pugi::xml_node& skillsNode = doc.child("skills");
    if (!skillsNode)
        return false;

    for (pugi::xml_node_iterator it = skillsNode.begin(); it != skillsNode.end(); ++it)
    {
        uint32_t id = 0;
        if (const pugi::xml_attribute& _attr = (*it).attribute("id"))
        {
            id = _attr.as_uint();
        }
        else
            return false;
        if (const pugi::xml_attribute& _attr = (*it).attribute("script"))
        {
            manager.skills_[id] = _attr.value();
        }
        else
            return false;
    }

    return true;
}

}

