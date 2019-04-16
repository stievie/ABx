#pragma once

namespace Game {
class Npc;
class Map;
class Skill;
class Actor;
}

namespace AI {

class AiTask;

class AiCharacter : public ai::ICharacter
{
private:
    using Super = ai::ICharacter;
    Game::Npc& owner_;
    const Game::Map* map_;
public:
    explicit AiCharacter(Game::Npc& owner, const Game::Map* map);
    ~AiCharacter() = default;
    void update(int64_t deltaTime, bool debuggingActive) override;
    void setPosition(const glm::vec3& position) override;
    void setOrientation(float orientation) override;
    void setSpeed(float speed) override;

    inline Game::Npc& GetNpc() const {
        return owner_;
    };
    AiTask* currentTask_;
    std::weak_ptr<Game::Skill> currentSkill_;
    int64_t lastFoeAttack_;
    std::weak_ptr<Game::Actor> lastAttacker_;
    std::weak_ptr<Game::Actor> currentTarget_;
};

inline Game::Npc& getNpc(const ai::AIPtr& ai) {
    return ai::character_cast<AiCharacter>(ai->getCharacter()).GetNpc();
}

}
