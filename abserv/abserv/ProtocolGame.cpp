#include "stdafx.h"
#include "ProtocolGame.h"
#include "Player.h"
#include "PlayerManager.h"
#include "OutputMessage.h"
#include "IOPlayer.h"
#include "IOAccount.h"
#include "Bans.h"
#include "Utils.h"
#include "GameManager.h"
#include "Logger.h"
#include "Game.h"

#include "DebugNew.h"

namespace Net {

void ProtocolGame::Login(const std::string& name, uint32_t accountId)
{
    std::shared_ptr<Game::Player> foundPlayer = Game::PlayerManager::Instance.GetPlayerByName(name);
    if (foundPlayer)
    {
        DisconnectClient("You are already logged in");
        return;
    }

    player_ = Game::PlayerManager::Instance.CreatePlayer(name, GetThis());
    if (!DB::IOPlayer::PreloadPlayer(player_.get(), name))
    {
        DisconnectClient("Your character could not be loaded");
        return;
    }

    if (Auth::BanManager::Instance.IsAccountBanned(accountId))
    {
        DisconnectClient("Your account has been banned");
        return;
    }

    if (!DB::IOPlayer::LoadPlayerByName(player_.get(), name))
    {
        DisconnectClient("Your character could not be loaded");
        return;
    }

    Connect(player_->id_);
    OutputMessagePool::Instance()->AddToAutoSend(shared_from_this());
}

void ProtocolGame::Logout()
{
    if (!player_)
        return;

    Game::PlayerManager::Instance.RemovePlayer(player_->id_);
    Disconnect();
    player_.reset();
}

void ProtocolGame::ParsePacket(NetworkMessage& message)
{
    if (!acceptPackets_ ||
        Game::GameManager::Instance.GetState() != Game::GameManager::Running ||
        message.GetSize() == 0)
        return;

    uint8_t recvByte = message.GetByte();

    switch (recvByte)
    {
    case PacketTypeLogout:
        Asynch::Dispatcher::Instance.Add(
            Asynch::CreateTask(std::bind(&ProtocolGame::Logout, GetThis()))
        );
        break;
    case PacketTypeMoveNorth:
        AddGameTask(&Game::Game::PlayerMove, player_->id_, Game::MoveDirectionNorth);
        break;
    case PacketTypeMoveNorthEast:
        AddGameTask(&Game::Game::PlayerMove, player_->id_, Game::MoveDirectionNorthEast);
        break;
    case PacketTypeMoveEast:
        AddGameTask(&Game::Game::PlayerMove, player_->id_, Game::MoveDirectionEast);
        break;
    case PacketTypeMoveSouthEast:
        AddGameTask(&Game::Game::PlayerMove, player_->id_, Game::MoveDirectionNorth);
        break;
    case PacketTypeMoveSouth:
        AddGameTask(&Game::Game::PlayerMove, player_->id_, Game::MoveDirectionSouth);
        break;
    case PacketTypeMoveSouthWest:
        AddGameTask(&Game::Game::PlayerMove, player_->id_, Game::MoveDirectionSouthWest);
        break;
    case PacketTypeMoveWest:
        AddGameTask(&Game::Game::PlayerMove, player_->id_, Game::MoveDirectionWest);
        break;
    case PacketTypeMoveNorthWest:
        AddGameTask(&Game::Game::PlayerMove, player_->id_, Game::MoveDirectionNorthWest);
        break;
    default:
        LOG_INFO << "Player " << player_->data_.name << " sent an unknown packet header: 0x" <<
            std::hex << static_cast<uint16_t>(recvByte) << std::dec << std::endl;
        break;
    }
}

void ProtocolGame::OnRecvFirstMessage(NetworkMessage& msg)
{
    std::string sessionKey = msg.GetString();
    std::vector<std::string> sessionArgs = Utils::Split(sessionKey, "\n");
    if (sessionArgs.size() != 4)
    {
        Disconnect();
        return;
    }

    std::string& accountName = sessionArgs[0];
    std::string& password = sessionArgs[1];

    if (accountName.empty())
    {
        DisconnectClient("You must enter your account name");
        return;
    }

    std::string characterName = msg.GetString();

    if (Auth::BanManager::Instance.IsIpBanned(GetIP()))
    {
        DisconnectClient("Your IP has been banned");
        return;
    }

    uint32_t accountId = DB::IOAccount::GameWorldAuth(accountName, password, characterName);
    if (accountId == 0)
    {
        DisconnectClient("Account name or password not correct");
        return;
    }

    Asynch::Dispatcher::Instance.Add(
        Asynch::CreateTask(
            std::bind(&ProtocolGame::Login, GetThis(), characterName, accountId)
        )
    );
}

void ProtocolGame::OnConnect()
{
}

void ProtocolGame::DisconnectClient(const std::string& message)
{
    std::shared_ptr<OutputMessage> output = OutputMessagePool::Instance()->GetOutputMessage();
    output->AddByte(0x14);
    output->AddString(message);
    Send(output);
    Disconnect();
}

void ProtocolGame::Connect(uint32_t playerId)
{
    if (IsConnectionExpired())
        // ProtocolGame::release() has been called at this point and the Connection object
        // no longer exists, so we return to prevent leakage of the Player.
        return;

    std::shared_ptr<Game::Player> foundPlayer = Game::PlayerManager::Instance.GetPlayerById(playerId);
    if (!foundPlayer)
    {
        DisconnectClient("You are already logged in");
        return;
    }

    player_ = foundPlayer;

    acceptPackets_ = true;


}

}
