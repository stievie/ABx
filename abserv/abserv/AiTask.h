#pragma once

#include <ai/SimpleAI.h>
#include "AiCharacter.h"

namespace AI {

/**
* @ingroup AI
*/
class AiTask : public ai::ITask
{
public:
    TASK_CLASS(AiTask)

    ai::TreeNodeStatus doAction(const ai::AIPtr& entity, int64_t deltaMillis) override
    {
        return doAction(ai::character_cast<AI::AiCharacter>(entity->getCharacter()), deltaMillis);
    }

    virtual ai::TreeNodeStatus doAction(AI::AiCharacter& chr, int64_t deltaMillis) = 0;
};

#define AI_TASK(TaskName) \
/** \
     * @ingroup AI \
      */ \
struct TaskName: public AiTask { \
	TaskName(const std::string& name, const std::string& parameters, const ai::ConditionPtr& condition) : \
			AiTask(name, parameters, condition) {} \
	virtual ~TaskName() {} \
	NODE_FACTORY(TaskName) \
	ai::TreeNodeStatus doAction(AI::AiCharacter& chr, int64_t deltaMillis) override; \
}; \
inline ai::TreeNodeStatus TaskName::doAction(AI::AiCharacter& chr, int64_t deltaMillis)

}

