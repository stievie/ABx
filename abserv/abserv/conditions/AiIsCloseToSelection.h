#pragma once

namespace AI {

/**
 * @ingroup AI
 */
class IsCloseToSelection : public ai::ICondition {
protected:
    int _distance;
public:
    IsCloseToSelection(const std::string& parameters) :
        ai::ICondition("IsCloseToSelection", parameters)
    {
        if (_parameters.empty())
            _distance = 1;
        else
            _distance = std::stoi(_parameters);
    }
    CONDITION_FACTORY(IsCloseToSelection)

    bool evaluate(const ai::AIPtr& entity) override
    {
        ai::Zone* zone = entity->getZone();
        if (zone == nullptr)
            return false;

        const ai::FilteredEntities& selection = entity->getFilteredEntities();
        if (selection.empty())
            return false;

        const glm::vec3& ownPos = entity->getCharacter()->getPosition();
        for (ai::CharacterId id : selection)
        {
            const ai::AIPtr& ai = zone->getAI(id);
            Game::Npc& npc = ai->getCharacterCast<AiCharacter>().GetNpc();
            Math::Vector3 _pos = npc.GetPosition();
            const glm::vec3 pos(_pos.x_, _pos.y_, _pos.z_);
            const float distance = glm::distance(pos, ownPos);
            if (distance > _distance)
                return false;
        }
        return true;
    }
};

}
