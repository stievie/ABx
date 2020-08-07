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


#include "Client.h"
#include "ProtocolLogin.h"
#include "ProtocolGame.h"
#include "Connection.h"
#include <sa/Compiler.h>
#include <iostream>
#include <fstream>
#include "Random.h"
#include <thread>
#define CPPHTTPLIB_OPENSSL_SUPPORT
#include <httplib.h>
#include <sa/ScopeGuard.h>
#include <sa/Compiler.h>
#include <sa/Assert.h>

#define PLAYER_INACTIVE_TIME_KICK (1000 * 15)

namespace Client {

const char* Client::GetProtocolErrorMessage(AB::ErrorCodes err)
{
    switch (err)
    {
    case AB::ErrorCodes::IPBanned:
        return "Your IP Address is banned.";
    case AB::ErrorCodes::TooManyConnectionsFromThisIP:
        return "Too many connection from this IP.";
    case AB::ErrorCodes::InvalidAccountName:
        return "Invalid Account name.";
    case AB::ErrorCodes::InvalidPassword:
        return "Invalid password.";
    case AB::ErrorCodes::NamePasswordMismatch:
        return "Name or password wrong.";
    case AB::ErrorCodes::AlreadyLoggedIn:
        return "You are already logged in.";
    case AB::ErrorCodes::ErrorLoadingCharacter:
        return "Error loading character.";
    case AB::ErrorCodes::AccountBanned:
        return "Your account is banned.";
    case AB::ErrorCodes::WrongProtocolVersion:
        return "Outdated client. Please update the game client.";
    case AB::ErrorCodes::InvalidEmail:
        return "Invalid Email.";
    case AB::ErrorCodes::InvalidAccountKey:
        return "Invalid Account Key.";
    case AB::ErrorCodes::UnknownError:
        return "Internal Error.";
    case AB::ErrorCodes::AccountNameExists:
        return "Login Name already exists.";
    case AB::ErrorCodes::InvalidCharacterName:
        return "Invalid character name.";
    case AB::ErrorCodes::InvalidProfession:
        return "Invalid profession.";
    case AB::ErrorCodes::PlayerNameExists:
        return "Character name already exists.";
    case AB::ErrorCodes::InvalidAccount:
        return "Invalid Account.";
    case AB::ErrorCodes::InvalidPlayerSex:
        return "Invalid character gender.";
    case AB::ErrorCodes::InvalidCharacter:
        return "Invalid character.";
    case AB::ErrorCodes::InvalidCharactersInString:
        return "The string contains invalid characters.";
    case AB::ErrorCodes::NoMoreCharSlots:
        return "You have no free character slots.";
    case AB::ErrorCodes::InvalidGame:
        return "Invalid Game.";
    case AB::ErrorCodes::AllServersFull:
        return "All Servers are full, please try again later.";
    case AB::ErrorCodes::ErrorException:
        return "Exception";
    case AB::ErrorCodes::TokenAuthFailure:
        return "Token authentication failure";
    case AB::ErrorCodes::AccountKeyAlreadyAdded:
        return "This account key was already added to the account";
    default:
        return "";
    }
}

const char* Client::GetNetworkErrorMessage(ConnectionError connectionError)
{
    switch (connectionError)
    {
    case ConnectionError::ResolveError:
        return "Resolve error";
    case ConnectionError::WriteError:
        return "Write error";
    case ConnectionError::ConnectError:
        return "Connect error";
    case ConnectionError::ReceiveError:
        return "Read error";
    case ConnectionError::ConnectTimeout:
        return "Connect timeout";
    case ConnectionError::ReadTimeout:
        return "Read timeout";
    case ConnectionError::WriteTimeout:
        return "Write timeout";
    case ConnectionError::DisconnectNoPong:
        return "Disconnect no Pong";
    default:
        return "Other Error";
    }
}

Client::Client(Receiver& receiver) :
    Client(receiver, std::make_shared<asio::io_service>())
{
}

Client::Client(Receiver& receiver, std::shared_ptr<asio::io_service> ioSerive) :
    receiver_(receiver),
    ioService_(ioSerive),
    loginHost_("127.0.0.1"),
    loginPort_(2748)
{
    Utils::Random::Instance.Initialize();
    // Always create new keys
    dhKeys_.GenerateKeys();
}

Client::~Client()
{
    Terminate();
}

void Client::ResetPoll()
{
    // Blocking!
    // Reset must always be called prior to poll
    ioService_->reset();
    ioService_->poll();
}

void Client::Poll()
{
    ioService_->poll();
}

void Client::Run()
{
#ifdef SA_PLATFORM_WIN
    ioService_->run();
#else
    if (state_ != State::World)
    {
        // WTF, why is this needed on Linux but not on Windows?
        if (ioService_->stopped())
            ioService_->reset();
    }
    ioService_->poll();
#endif // SA_PLATFORM_WIN
}

void Client::Terminate()
{
    ioService_->stop();
    Connection::Terminate();
}

httplib::SSLClient* Client::GetHttpClient()
{
    if (!httpClient_)
    {
        if (!fileHost_.empty() && filePort_ != 0)
            httpClient_ = std::make_unique<httplib::SSLClient>(fileHost_, filePort_);
    }
    if (httpClient_)
        return httpClient_.get();
    return nullptr;
}

void Client::OnLoggedIn(const std::string& accountUuid, const std::string& authToken, AB::Entities::AccountType accType)
{
    accountUuid_ = accountUuid;
    authToken_ = authToken;
    gamePort_ = protoLogin_->gamePort_;
    if (!protoLogin_->gameHost_.empty())
        gameHost_ = protoLogin_->gameHost_;
    else
        // If game host is empty use the login host
        gameHost_ = loginHost_;

    filePort_ = protoLogin_->filePort_;
    if (!protoLogin_->fileHost_.empty())
        fileHost_ = protoLogin_->fileHost_;
    else
        // If file host is empty use the login host
        fileHost_ = loginHost_;
    lastPongTick_ = 0;

    receiver_.OnLoggedIn(accountUuid_, authToken_, accType);
}

void Client::OnGetCharlist(const AB::Entities::CharList& chars)
{
    state_ = State::SelectChar;

    receiver_.OnGetCharlist(chars);

    // Get list of outposts
    GetOutposts();
}

void Client::OnGetOutposts(const std::vector<AB::Entities::Game>& games)
{

    receiver_.OnGetOutposts(games);
}

void Client::OnGetServices(const std::vector<AB::Entities::Service>& services)
{
    receiver_.OnGetServices(services);
}

void Client::OnAccountCreated()
{
    receiver_.OnAccountCreated();
}

void Client::OnPlayerCreated(const std::string& uuid, const std::string& mapUuid)
{
    receiver_.OnPlayerCreated(uuid, mapUuid);
}

void Client::OnAccountKeyAdded()
{
    receiver_.OnAccountKeyAdded();
}

void Client::OnCharacterDeleted(const std::string& uuid)
{
    receiver_.OnCharacterDeleted(uuid);;
}

void Client::OnLog(const std::string& message)
{
    receiver_.OnLog(message);
}

void Client::OnNetworkError(ConnectionError connectionError, const std::error_code& err)
{
    receiver_.OnNetworkError(connectionError, err);
}

void Client::OnProtocolError(AB::ErrorCodes err)
{
    receiver_.OnProtocolError(err);
}

void Client::OnPong(int lastPing)
{
    gotPong_ = true;
    lastPongTick_ = AbTick();
    pings_.Enqueue(lastPing);
}

ProtocolLogin& Client::GetProtoLogin()
{
    if (!protoLogin_)
    {
        protoLogin_ = std::make_shared<ProtocolLogin>(dhKeys_, *ioService_);
        protoLogin_->SetErrorCallback(std::bind(&Client::OnNetworkError, this, std::placeholders::_1, std::placeholders::_2));
        protoLogin_->SetProtocolErrorCallback(std::bind(&Client::OnProtocolError, this, std::placeholders::_1));
    }
    ASSERT(protoLogin_);
    return *protoLogin_;
}

void Client::Login(const std::string& name, const std::string& pass)
{
    if (!(state_ == State::Disconnected || state_ == State::CreateAccount))
        return;

    accountName_ = name;
    password_ = pass;
    lastPongTick_ = 0;

    // 1. Login to login server -> get character list
    GetProtoLogin().Login(loginHost_, loginPort_, name, pass,
        std::bind(&Client::OnLoggedIn, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3),
        std::bind(&Client::OnGetCharlist, this, std::placeholders::_1));
}

void Client::CreateAccount(const std::string& name, const std::string& pass,
    const std::string& email, const std::string& accKey)
{
    if (state_ != State::CreateAccount)
        return;

    accountName_ = name;
    password_ = pass;

    GetProtoLogin().CreateAccount(loginHost_, loginPort_, name, pass,
        email, accKey,
        std::bind(&Client::OnAccountCreated, this));
}

void Client::CreatePlayer(const std::string& charName, const std::string& profUuid,
    uint32_t modelIndex,
    AB::Entities::CharacterSex sex, bool isPvp)
{
    if (state_ != State::SelectChar)
        return;

    if (accountUuid_.empty() || authToken_.empty())
        return;

    GetProtoLogin().CreatePlayer(loginHost_, loginPort_, accountUuid_, authToken_,
        charName, profUuid, modelIndex, sex, isPvp,
        std::bind(&Client::OnPlayerCreated, this, std::placeholders::_1, std::placeholders::_2));
}

void Client::DeleteCharacter(const std::string& uuid)
{
    if (accountUuid_.empty() || authToken_.empty())
        return;
    GetProtoLogin().DeleteCharacter(loginHost_, loginPort_, accountUuid_, authToken_,
        uuid,
        std::bind(&Client::OnCharacterDeleted, this, std::placeholders::_1));
}

void Client::AddAccountKey(const std::string& newKey)
{
    if (state_ != State::SelectChar)
        return;

    if (accountUuid_.empty() || authToken_.empty())
        return;
    if (newKey.empty())
        return;
    GetProtoLogin().AddAccountKey(loginHost_, loginPort_, accountUuid_, authToken_,
        newKey,
        std::bind(&Client::OnAccountKeyAdded, this));
}

void Client::Logout()
{
    if (state_ != State::World)
        return;
    if (protoGame_)
    {
        state_ = State::Disconnected;
        protoGame_->Logout();
#ifdef SA_PLATFORM_WIN
        Run();
#else
        ioService_->run_one();
#endif
    }
    lastPongTick_ = 0;
    enterWorldMessage_ = 0;
}

void Client::GetOutposts()
{
    if (accountUuid_.empty() || authToken_.empty())
        return;

    GetProtoLogin().GetOutposts(loginHost_, loginPort_, accountUuid_, authToken_,
        std::bind(&Client::OnGetOutposts, this, std::placeholders::_1));
}

void Client::GetServers()
{
    if (accountUuid_.empty() || authToken_.empty())
        return;

    GetProtoLogin().GetServers(loginHost_, loginPort_, accountUuid_, authToken_,
        std::bind(&Client::OnGetServices, this, std::placeholders::_1));
}

void Client::EnterWorld(const std::string& charUuid, const std::string& mapUuid,
    const std::string& host /* = "" */, uint16_t port /* = 0 */,
    const std::string& instanceId /* = "" */)
{
    ASSERT(!accountUuid_.empty());
    ASSERT(!authToken_.empty());
    // Enter or changing the world
    if (state_ != State::SelectChar && state_ != State::World)
        return;

    if (state_ == State::World)
    {
        // We are already logged in to some world so we must logout
        Logout();
        gotPong_ = true;
    }

    // Maybe different server
    if (!host.empty())
        gameHost_ = host;
    if (port != 0)
        gamePort_ = port;

    // 2. Login to game server
    protoGame_ = std::make_shared<ProtocolGame>(*this, dhKeys_, *ioService_);

    protoGame_->Login(accountUuid_, authToken_, charUuid, mapUuid, instanceId,
        gameHost_, gamePort_);
}

void Client::Update(uint32_t timeElapsed, bool noRun)
{
    if (state_ == State::World)
    {
        if ((lastPing_ >= 1000 && gotPong_) || (lastPing_ > 5000))
        {
            // Send every second a Ping. If we didn't get a pong the last 5 seconds also send a ping.
            if (protoGame_)
            {
                gotPong_ = false;
                protoGame_->Ping();
            }
            lastPing_ = 0;
        }
        if (enterWorldMessage_ != 0 && AbTick() - enterWorldMessage_ > PLAYER_INACTIVE_TIME_KICK * 2)
        {
            // When we enter a Game it may take some time the client loaded everything.
            if ((lastPongTick_ != 0) && (lastPongTick_ + PLAYER_INACTIVE_TIME_KICK < AbTick()))
            {
                protoGame_->Disconnect();
                state_ = State::Disconnected;
                OnNetworkError(ConnectionError::DisconnectNoPong, {});
                return;
            }
        }
    }

    lastRun_ += timeElapsed;
    if (lastRun_ >= 16)
    {
        // Don't send more than ~60 updates to the server, it might DC.
        // If running @144Hz every 2nd Update. If running @60Hz every update
        lastRun_ = 0;
        if (!noRun)
            Run();
    }
    if (state_ == State::World)
        lastPing_ += timeElapsed;
}

bool Client::HttpRequest(const std::string& path, std::function<bool(const char* data, uint64_t size)>&& callback)
{
    auto* client = GetHttpClient();
    if (client == nullptr)
        return false;

    std::stringstream ss;
    ss << accountUuid_ << authToken_;
    httplib::Headers header = {
        { "Auth", ss.str() }
    };
    auto res = client->Get(path.c_str(), header, std::move(callback));
    return res && res->status == 200;
}

bool Client::HttpRequest(const std::string& path, std::ostream& out)
{
    return HttpRequest(path, [&out](const char* data, uint64_t size)
    {
        out.write(data, size);
        return true;
    });
}

bool Client::HttpDownload(const std::string& path, const std::string& outFile)
{
    std::remove(outFile.c_str());
    std::ofstream f;
    f.open(outFile);
    if (!f.is_open())
        return false;
    sa::ScopeGuard fileGuard([&f]()
    {
        f.close();
    });
    return HttpRequest(path, f);
}

uint32_t Client::GetIp() const
{
    if (protoGame_ && protoGame_->IsConnected())
        return protoGame_->GetIp();
    return 0;
}

int64_t Client::GetClockDiff() const
{
    if (protoGame_)
        return protoGame_->GetClockDiff();
    return 0;
}

std::pair<bool, uint32_t> Client::PingServer(const std::string& host, uint16_t port)
{
    // The login server should be able to reply within 500ms.
    static constexpr int64_t TIMEOUT = 500;

    asio::io_service ioService;
    asio::ip::udp::socket socket(ioService, asio::ip::udp::endpoint(asio::ip::udp::v4(), 0));
    asio::ip::udp::resolver resolver(ioService);
    asio::ip::udp::endpoint endpoint = *resolver.resolve(asio::ip::udp::resolver::query(asio::ip::udp::v4(), host, std::to_string(port)));
    char senddata[64] = "ablogin";

    bool result = false;

    socket.send_to(asio::buffer(senddata, 64), endpoint);

    char recvdata[64] = {};

    socket.async_receive_from(asio::buffer(recvdata, 64), endpoint, [&](const asio::error_code&, size_t)
    {
        if (strcmp(senddata, recvdata) == 0)
            result = true;
    });

    int64_t start = AbTick();

    while (!result && (AbTick() - start) < TIMEOUT)
    {
        ioService.poll();
        millisleep(1);
    }
    return { result, static_cast<uint32_t>(AbTick() - start) };
}

void Client::ChangeMap(const std::string& mapUuid)
{
    if (state_ == State::World)
        protoGame_->ChangeMap(mapUuid);
}

void Client::GetMailHeaders()
{
    if (state_ == State::World)
        protoGame_->GetMailHeaders();
}

void Client::GetMail(const std::string& mailUuid)
{
    if (state_ == State::World)
        protoGame_->GetMail(mailUuid);
}

void Client::GetInventory()
{
    if (state_ == State::World)
        protoGame_->GetInventory();
}

void Client::InventoryDestroyItem(uint16_t pos)
{
    if (state_ == State::World)
        protoGame_->InventoryDestroyItem(pos);
}

void Client::InventoryDropItem(uint16_t pos, uint32_t count)
{
    if (state_ == State::World)
        protoGame_->InventoryDropItem(pos, count);
}

void Client::SetItemPos(AB::Entities::StoragePlace currentPlace, uint16_t currentPos,
    AB::Entities::StoragePlace place, uint16_t newPos, uint32_t count)
{
    if (state_ == State::World)
        protoGame_->SetItemPos(currentPlace, currentPos, place, newPos, count);
}

void Client::GetChest()
{
    if (state_ == State::World)
        protoGame_->GetChest();
}

void Client::ChestDestroyItem(uint16_t pos)
{
    if (state_ == State::World)
        protoGame_->ChestDestroyItem(pos);
}

void Client::DepositMoney(uint32_t amount)
{
    if (state_ == State::World)
        protoGame_->DepositMoney(amount);
}

void Client::WithdrawMoney(uint32_t amount)
{
    if (state_ == State::World)
        protoGame_->WithdrawMoney(amount);
}

void Client::SellItem(uint32_t npcId, uint16_t pos, uint32_t count)
{
    if (state_ == State::World)
        protoGame_->SellItem(npcId, pos, count);
}

void Client::BuyItem(uint32_t npcId, uint32_t id, uint32_t count)
{
    if (state_ == State::World)
        protoGame_->BuyItem(npcId, id, count);
}

void Client::GetMerchantItems(uint32_t npcId, uint16_t itemType, const std::string& searchName, uint32_t page)
{
    if (state_ == State::World)
        protoGame_->GetMerchantItems(npcId, itemType, searchName, page);
}

void Client::GetCraftsmanItems(uint32_t npcId, uint16_t itemType, const std::string& searchName, uint32_t page)
{
    if (state_ == State::World)
        protoGame_->GetCraftsmanItems(npcId, itemType, searchName, page);
}

void Client::CraftItem(uint32_t npcId, uint32_t index, uint32_t count, uint32_t attributeIndex)
{
    if (state_ == State::World)
        protoGame_->CraftItem(npcId, index, count, attributeIndex);
}

void Client::SalvageItem(uint16_t kitPos, uint16_t pos)
{
    if (state_ == State::World)
        protoGame_->SalvageItem(kitPos, pos);
}

void Client::DeleteMail(const std::string& mailUuid)
{
    if (state_ == State::World)
        protoGame_->DeleteMail(mailUuid);
}

void Client::SendMail(const std::string& recipient, const std::string& subject, const std::string& body)
{
    if (state_ == State::World)
        protoGame_->SendMail(recipient, subject, body);
}

void Client::GetPlayerInfoByName(const std::string& name, uint32_t fields)
{
    if (state_ == State::World)
        protoGame_->GetPlayerInfoByName(name, fields);
}

void Client::GetPlayerInfoByAccount(const std::string& accountUuid, uint32_t fields)
{
    if (state_ == State::World)
        protoGame_->GetPlayerInfoByAccount(accountUuid, fields);
}

void Client::Move(uint8_t direction)
{
    if (state_ == State::World)
        protoGame_->Move(direction);
}

void Client::Turn(uint8_t direction)
{
    if (state_ == State::World)
        protoGame_->Turn(direction);
}

void Client::SetDirection(float rad)
{
    if (state_ == State::World)
        protoGame_->SetDirection(rad);
}

void Client::ClickObject(uint32_t sourceId, uint32_t targetId)
{
    if (state_ == State::World)
        protoGame_->ClickObject(sourceId, targetId);
}

void Client::SelectObject(uint32_t sourceId, uint32_t targetId)
{
    if (state_ == State::World)
        protoGame_->SelectObject(sourceId, targetId);
}

void Client::FollowObject(uint32_t targetId, bool ping)
{
    if (state_ == State::World)
        protoGame_->Follow(targetId, ping);
}

void Client::Command(AB::GameProtocol::CommandType type, const std::string& data)
{
    if (state_ == State::World)
        protoGame_->Command(type, data);
}

void Client::GotoPos(const Vec3& pos)
{
    if (state_ == State::World)
        protoGame_->GotoPos(pos);
}

void Client::PartyInvitePlayer(uint32_t targetId)
{
    if (state_ == State::World)
        protoGame_->PartyInvitePlayer(targetId);
}

void Client::PartyKickPlayer(uint32_t targetId)
{
    if (state_ == State::World)
        protoGame_->PartyKickPlayer(targetId);
}

void Client::PartyAcceptInvite(uint32_t inviterId)
{
    if (state_ == State::World)
        protoGame_->PartyAcceptInvite(inviterId);
}

void Client::PartyRejectInvite(uint32_t inviterId)
{
    if (state_ == State::World)
        protoGame_->PartyRejectInvite(inviterId);
}

void Client::PartyGetMembers(uint32_t partyId)
{
    if (state_ == State::World)
        protoGame_->PartyGetMembers(partyId);
}

void Client::PartyLeave()
{
    if (state_ == State::World)
        protoGame_->PartyLeave();
}

void Client::UseSkill(uint32_t index, bool ping)
{
    if (state_ == State::World)
        protoGame_->UseSkill(index, ping);
}

void Client::Attack(bool ping)
{
    if (state_ == State::World)
        protoGame_->Attack(ping);
}

void Client::Interact(bool ping)
{
    if (state_ == State::World)
        protoGame_->Interact(ping);
}

void Client::QueueMatch()
{
    if (state_ == State::World)
        protoGame_->QueueMatch();
}

void Client::UnqueueMatch()
{
    if (state_ == State::World)
        protoGame_->UnqueueMatch();
}

void Client::AddFriend(const std::string& name, AB::Entities::FriendRelation relation)
{
    if (state_ == State::World)
        protoGame_->AddFriend(name, relation);
}

void Client::RemoveFriend(const std::string& accountUuid)
{
    if (state_ == State::World)
        protoGame_->RemoveFriend(accountUuid);
}

void Client::RenameFriend(const std::string& accountUuid, const std::string& newName)
{
    if (state_ == State::World)
        protoGame_->RenameFriend(accountUuid, newName);
}

void Client::UpdateFriendList()
{
    if (state_ == State::World)
        protoGame_->UpdateFriendList();
}

void Client::Cancel()
{
    if (state_ == State::World)
        protoGame_->Cancel();
}

void Client::SetPlayerState(AB::GameProtocol::CreatureState newState)
{
    if (state_ == State::World)
        protoGame_->SetPlayerState(newState);
}

void Client::SetOnlineStatus(AB::Packets::Server::PlayerInfo::Status status)
{
    if (state_ == State::World)
        protoGame_->SetOnlineStatus(status);
}

void Client::SetSecondaryProfession(uint32_t profIndex)
{
    if (state_ == State::World)
        protoGame_->SetSecondaryProfession(profIndex);
}

void Client::SetAttributeValue(uint32_t attribIndex, uint8_t value)
{
    if (state_ == State::World)
        protoGame_->SetAttributeValue(attribIndex, value);
}

void Client::EquipSkill(uint32_t skillIndex, uint8_t pos)
{
    if (state_ == State::World)
        protoGame_->EquipSkill(skillIndex, pos);
}

void Client::LoadSkillTemplate(const std::string& templ)
{
    if (state_ == State::World)
        protoGame_->LoadSkillTemplate(templ);
}

void Client::TradeRequest(uint32_t targetId)
{
    if (state_ == State::World)
        protoGame_->TradeRequest(targetId);
}

void Client::TradeCancel()
{
    if (state_ == State::World)
        protoGame_->TradeCancel();
}

void Client::TradeOffer(uint32_t money, std::vector<std::pair<uint16_t, uint32_t>>&& items)
{
    if (state_ == State::World)
        protoGame_->TradeOffer(money, std::forward<std::vector<std::pair<uint16_t, uint32_t>>>(items));
}

void Client::TradeAccept()
{
    if (state_ == State::World)
        protoGame_->TradeAccept();
}

void Client::GetItemPrice(const std::vector<uint16_t>& items)
{
    if (state_ == State::World)
        protoGame_->GetItemPrice(items);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::ServerJoined& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::ServerLeft& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::ChangeInstance& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::EnterWorld& packet)
{
    lastPongTick_ = 0;
    enterWorldMessage_ = AbTick();
    state_ = State::World;
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::PlayerAutorun& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectSpawn& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectSpawnExisting& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::MailHeaders& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::MailComplete& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectDespawn& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectPositionUpdate& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectSpeedChanged& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::InventoryContent& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::InventoryItemUpdate& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::InventoryItemDelete& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::ChestContent& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::ChestItemUpdate& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::ChestItemDelete& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectRotationUpdate& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectTargetSelected& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectStateChanged& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::PlayerError& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectSkillFailure& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectUseSkill& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectSkillSuccess& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectAttackFailure& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectPingTarget& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectEffectAdded& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectEffectRemoved& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectDamaged& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectHealed& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectProgress& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectDroppedItem& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectForcePosition& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectGroupMaskChanged& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectSetAttackSpeed& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::ServerMessage& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::ChatMessage& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::PartyPlayerInvited& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::PartyPlayerRemoved& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::PartyPlayerAdded& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::PartyInviteRemoved& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::PartyResigned& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::PartyDefeated& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::PartyMembersInfo& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectResourceChanged& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::DialogTrigger& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::FriendList& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::PlayerInfo& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::FriendAdded& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::FriendRemoved& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::FriendRenamed& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::GuildInfo& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::GuildMemberList& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::QuestSelectionDialogTrigger& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::QuestDialogTrigger& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::NpcHasQuest& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::QuestDeleted& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::QuestRewarded& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::SetObjectAttributeValue& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectSecProfessionChanged& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectSetSkill& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::SkillTemplateLoaded& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::TradeDialogTrigger& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::TradeCancel& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::TradeOffer& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::TradeAccepted& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::MerchantItems& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::ItemPrice& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::CraftsmanItems& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

void Client::OnPacket(int64_t updateTick, const AB::Packets::Server::DropTargetChanged& packet)
{
    receiver_.OnPacket(updateTick, packet);
}

}
