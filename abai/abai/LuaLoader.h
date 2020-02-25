/**
 * Copyright 2017-2020 Stefan Ascher
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#pragma once

#include "Loader.h"
#include <kaguya/kaguya.hpp>
#include "AiDefines.h"

namespace AI {

class Node;
class Condition;
class Filter;
class Root;

class LuaLoader : public Loader
{
private:
    void RegisterLua(kaguya::State& state);
    std::shared_ptr<Node> CreateNode(const std::string& type);
    std::shared_ptr<Condition> CreateCondition(const std::string& type);
    std::shared_ptr<Filter> CreateFilter(const std::string& type);
    std::shared_ptr<Node> CreateNodeWidthArgs(const std::string& type, const ArgumentsType& arguments);
    std::shared_ptr<Condition> CreateConditionWidthArgs(const std::string& type, const ArgumentsType& arguments);
    std::shared_ptr<Filter> CreateFilterWidthArgs(const std::string& type, const ArgumentsType& arguments);
    // Create a whole tree from a file and return the root node.
    std::shared_ptr<Root> CreateTree(const std::string& name, const std::string& filename);
protected:
    // Subclasses should override it to get the full filename of an include file.
    virtual std::string GetScriptFile(const std::string file) { return file; }
    // Can be overwritten if the script is somehow cached
    virtual bool ExecuteScript(kaguya::State& state, const std::string& file);
    // Script load error handler
    virtual void LoadError(const std::string&) { }
public:
    explicit LuaLoader(Registry& reg) :
        Loader(reg)
    { }
    ~LuaLoader() override;
    std::shared_ptr<Root> LoadFile(const std::string& fileName) override;
    std::shared_ptr<Root> LoadString(const std::string& value) override;
    // Initialize the cache from an init script
    bool InitChache(const std::string& initScript, BevaviorCache& cache) override;
};

}
