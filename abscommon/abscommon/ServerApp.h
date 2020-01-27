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

#include <atomic>
#include <vector>
#include <AB/Entities/Service.h>
#include "UuidUtils.h"
#include <sa/ArgParser.h>

namespace IO {
class DataClient;
}

namespace Net {
class MessageClient;
}

class ServerApp
{
private:
    void Init();
    static std::string GetMachineName();
protected:
    std::atomic<bool> running_;
    AB::Entities::ServiceType serverType_;
    std::string serverId_;
    std::string machine_;
    std::string serverName_;
    std::string serverLocation_;
    std::string configFile_;
    std::string logDir_;
    std::string serverHost_;
    std::string serverIp_;
    uint16_t serverPort_;
    sa::arg_parser::cli cli_;
    sa::arg_parser::values parsedArgs_;
    /// Get a generic currently unique server name
    std::string GetFreeName(IO::DataClient* client);
    virtual bool ParseCommandLine();
    void ShowHelp();
    virtual void ShowVersion() = 0;
    void ShowCommandlineError(const sa::arg_parser::result& err);
    void UpdateService(AB::Entities::Service& service);
public:
    ServerApp();
    virtual ~ServerApp() = default;
    bool InitializeW(int argc, wchar_t** argv);
    bool InitializeA(int argc, char** argv);
    virtual bool Initialize(const std::vector<std::string>& args);
    virtual void Run()
    { }
    virtual void Stop()
    { }
    /// Returns the Server UUID from the config file
    const std::string& GetServerId() const
    {
        return serverId_;
    }
    const std::string& GetServerName() const
    {
        return serverName_;
    }
    bool SendServerJoined(Net::MessageClient* client, const AB::Entities::Service& service);
    bool SendServerLeft(Net::MessageClient* client, const AB::Entities::Service& service);
    void Spawn(const std::string& additionalArguments);

    std::string path_;
    std::string exeFile_;
    std::vector<std::string> arguments_;
};
