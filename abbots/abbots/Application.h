/**
 * Copyright 2020 Stefan Ascher
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

#include <abscommon/ServerApp.h>
#include <vector>
#include <memory>

class BotClient;

struct Account
{
    std::string name;
    std::string pass;
    std::string character;
};

class Application final : public ServerApp
{
private:
    std::shared_ptr<asio::io_service> ioService_;
    std::string loginHost_;
    uint16_t loginPort_{ 0 };
    std::unique_ptr<BotClient> client_;
    std::vector<Account> accounts_;
    int64_t lastUpdate_{ 0 };
    bool LoadMain();
    void ShowVersion() override;
    void ShowLogo();
    void Update();
    void MainLoop();
    void CreateBots();
    void StartBot();
    void Shutdown();
protected:
    bool ParseCommandLine() override;
public:
    Application();
    ~Application() override;

    bool Initialize(const std::vector<std::string>& args) override;
    void Run() override;
    void Stop() override;
};
