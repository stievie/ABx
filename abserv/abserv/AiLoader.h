#pragma once

#pragma warning(push)
#pragma warning(disable: 4100)
#include <ai/tree/loaders/lua/LUATreeLoader.h>
#pragma warning(pop)

namespace AI {

class AiLoader
{
private:
    ai::AIRegistry& registry_;
    // add your own tasks and conditions here
    ai::LUATreeLoader loader_;
public:
    explicit AiLoader(ai::AIRegistry& registry) :
        registry_(registry),
        loader_(registry)
    {
    }

    ai::TreeNodePtr load(const std::string& name)
    {
        return loader_.load(name);
    }

    operator ai::AIRegistry& ()
    {
        return registry_;
    }

    void getTrees(std::vector<std::string>& trees) const
    {
        loader_.getTrees(trees);
    }

    bool init(const std::string& filename)
    {
        std::ifstream btStream(filename);
        if (!btStream)
        {
            LOG_ERROR << "could not load " << filename << std::endl;
            return false;
        }

        std::stringstream buffer;
        buffer << btStream.rdbuf();
        const std::string& str = buffer.str();

        if (!loader_.init(str))
        {
            LOG_ERROR << "could not load the tree: " << loader_.getError() << std::endl;
            return false;
        }
        return true;
    }

    inline std::string getError() const
    {
        return loader_.getError();
    }
};

}
