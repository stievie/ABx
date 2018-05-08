#include "stdafx.h"
#include "EffectManager.h"
#include "Effect.h"
#include "DataProvider.h"
#include "DataClient.h"

namespace Game {

EffectManager EffectManager::Instance;

std::shared_ptr<Effect> EffectManager::Get(uint32_t index)
{
    std::shared_ptr<Effect> result;
    auto it = effects_.find(index);
    if (it != effects_.end())
    {
        result = std::make_shared<Effect>((*it).second);
    }
    else
    {
        IO::DataClient* client = Application::Instance->GetDataClient();
        AB::Entities::Effect effect;
        effect.index = index;
        if (!client->Read(effect))
        {
            LOG_ERROR << "Error reading effect with index " << index << std::endl;
            return std::shared_ptr<Effect>();
        }
        result = std::make_shared<Effect>(effect);
        // Move to cache
        effects_.emplace(index, effect);
    }

    if (result)
    {
        if (result->LoadScript(IO::DataProvider::Instance.GetDataFile(result->data_.script)))
            return result;
    }

    return std::shared_ptr<Effect>();
}

}
