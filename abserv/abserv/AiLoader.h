#pragma once

#pragma warning(push)
#pragma warning(disable: 4100)
#include <ai/tree/loaders/lua/LUATreeLoader.h>
#pragma warning(pop)

namespace IO {

class AiLoader
{
private:
    ai::AIRegistry _registry;
    // add your own tasks and conditions here
    ai::LUATreeLoader _loader;
public:
    AiLoader() :
        _loader(_registry) {
    }

    ai::TreeNodePtr load(const std::string &name) {
        return _loader.load(name);
    }

    operator ai::AIRegistry& () {
        return _registry;
    }

    void getTrees(std::vector<std::string>& trees) const {
        _loader.getTrees(trees);
    }

    bool init(const std::string& filename) {
        std::ifstream btStream(filename);
        if (!btStream) {
            LOG_ERROR << "could not load " << filename << std::endl;
            return false;
        }

        std::stringstream buffer;
        buffer << btStream.rdbuf();
        const std::string& str = buffer.str();

        if (!_loader.init(str)) {
            LOG_ERROR << "could not load the tree: " << _loader.getError() << std::endl;
            return false;
        }
        return true;
    }

    inline std::string getError() const {
        return _loader.getError();
    }
};

}
