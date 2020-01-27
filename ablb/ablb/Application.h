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

#include "ServerApp.h"
#include "DataClient.h"
#include "Acceptor.h"
#include <AB/Entities/Service.h>

class Application final : public ServerApp
{
private:
    typedef std::pair<std::string, uint16_t> ServiceItem;
    std::vector<ServiceItem> serviceList_;
    asio::io_service ioService_;
    AB::Entities::ServiceType lbType_;
    std::unique_ptr<IO::DataClient> dataClient_;
    std::unique_ptr<Acceptor> acceptor_;
    void PrintServerInfo();
    bool LoadMain();
    bool GetServiceCallback(AB::Entities::Service& svc);
    bool GetServiceCallbackList(AB::Entities::Service& svc);
    bool ParseServerList(const std::string& fileName);
    void ShowLogo();
protected:
    void ShowVersion() override;
public:
    Application();
    ~Application() override;

    bool Initialize(const std::vector<std::string>& args) override;
    void Run() override;
    void Stop() override;
};

