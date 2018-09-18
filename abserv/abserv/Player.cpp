#include "stdafx.h"
#include "Player.h"
#include "Logger.h"
#include "Chat.h"
#include "Random.h"
#include "MailBox.h"
#include "PlayerManager.h"
#include "IOMail.h"
#include "IOGuild.h"
#include "StringUtils.h"
#include "Application.h"
#include <AB/Entities/Character.h>
#include "Profiler.h"
#include "GameManager.h"

#include "DebugNew.h"

namespace Game {

Player::Player(std::shared_ptr<Net::ProtocolGame> client) :
    Actor(),
    client_(std::move(client)),
    lastPing_(0),
    mailBox_(nullptr)
{
}

Player::~Player()
{
#ifdef DEBUG_GAME
//    LOG_DEBUG << std::endl;
#endif
}

void Player::SetGame(std::shared_ptr<Game> game)
{
    Actor::SetGame(game);
    std::shared_ptr<Party> party = GetParty();
    // Changing the instance also clears any invites. The client should check that we
    // leave the instance so don't send anything to invitees.
    party->ClearInvites();
    if (game)
    {
        if (party->IsLeader(this))
            party->SetPartySize(game->data_.partySize);
    }
}

void Player::Logout()
{
    if (auto g = GetGame())
        g->PlayerLeave(id_);
    GetParty()->Remove(GetThis());
    client_->Logout();
}

void Player::Ping()
{
    lastPing_ = Utils::AbTick();
    Net::NetworkMessage msg;
    msg.AddByte(AB::GameProtocol::GamePong);
    client_->WriteToOutput(msg);
}

void Player::UpdateMailBox()
{
    if (!mailBox_ && !data_.accountUuid.empty() && !uuids::uuid(data_.accountUuid).nil())
        mailBox_ = std::make_unique<MailBox>(data_.accountUuid);
    if (mailBox_)
    {
        mailBox_->Update();
    }
}

void Player::GetMailHeaders()
{
    UpdateMailBox();
    if (!mailBox_)
        return;
    Net::NetworkMessage msg;
    msg.AddByte(AB::GameProtocol::MailHeaders);
    msg.Add<uint16_t>(static_cast<uint16_t>(mailBox_->GetTotalMailCount()));
    const AB::Entities::MailList& mails = mailBox_->GetMails();
    for (const auto& mail : mails.mails)
    {
        msg.AddString(mail.uuid);
        msg.AddString(mail.fromName);
        msg.AddString(mail.subject);
        msg.Add<int64_t>(mail.created);
        msg.AddByte(mail.isRead ? 1 : 0);
    }
    client_->WriteToOutput(msg);
}

void Player::SendMail(const std::string recipient, const std::string subject, const std::string body)
{
    Net::NetworkMessage nmsg;
    nmsg.AddByte(AB::GameProtocol::ServerMessage);
    if (IO::IOMail::SendMailToPlayer(recipient, data_.accountUuid, GetName(), subject, body))
        nmsg.AddByte(AB::GameProtocol::ServerMessageTypeMailSent);
    else
        nmsg.AddByte(AB::GameProtocol::ServerMessageTypeMailNotSent);
    nmsg.AddString(recipient);
    nmsg.AddString("");                // Data
    client_->WriteToOutput(nmsg);
}

void Player::GetMail(const std::string mailUuid)
{
    UpdateMailBox();
    if (!mailBox_)
        return;

    // mailUuid must not be a reference!
    AB::Entities::Mail m;
    if (mailBox_->ReadMail(mailUuid, m))
    {
        Net::NetworkMessage msg;
        msg.AddByte(AB::GameProtocol::MailComplete);
        msg.AddString(m.fromAccountUuid);
        msg.AddString(m.fromName);
        msg.AddString(m.toName);
        msg.AddString(m.subject);
        msg.AddString(m.message);
        msg.Add<int64_t>(m.created);
        msg.AddByte(m.isRead ? 1 : 0);
        client_->WriteToOutput(msg);
    }
}

void Player::DeleteMail(const std::string mailUuid)
{
    // mailUuid must not be a reference!
    UpdateMailBox();
    if (!mailBox_)
        return;

    if (mailUuid.compare("all") == 0)
    {
        mailBox_->DeleteAll();
        Net::NetworkMessage msg;
        msg.AddByte(AB::GameProtocol::ServerMessage);
        msg.AddByte(AB::GameProtocol::ServerMessageTypeMailDeleted);
        msg.AddString(GetName());
        msg.AddString(mailUuid);
        client_->WriteToOutput(msg);
        return;
    }

    AB::Entities::Mail m;
    if (mailBox_->DeleteMail(mailUuid, m))
    {
        Net::NetworkMessage msg;
        msg.AddByte(AB::GameProtocol::ServerMessage);
        msg.AddByte(AB::GameProtocol::ServerMessageTypeMailDeleted);
        msg.AddString(GetName());
        msg.AddString(mailUuid);
        client_->WriteToOutput(msg);
    }
}

void Player::NotifyNewMail()
{
    UpdateMailBox();
    if (!mailBox_)
        return;

    if (mailBox_->GetTotalMailCount() > 0)
    {
        // Notify player there are new emails since last check.
        Net::NetworkMessage msg;
        msg.AddByte(AB::GameProtocol::ServerMessage);
        msg.AddByte(AB::GameProtocol::ServerMessageTypeNewMail);
        msg.AddString(GetName());
        msg.AddString(std::to_string(mailBox_->GetTotalMailCount()));
        client_->WriteToOutput(msg);
    }
    if (mailBox_->GetTotalMailCount() >= AB::Entities::Limits::MAX_MAIL_COUNT)
    {
        // Notify player that mailbox is full.
        Net::NetworkMessage msg;
        msg.AddByte(AB::GameProtocol::ServerMessage);
        msg.AddByte(AB::GameProtocol::ServerMessageTypeMailboxFull);
        msg.AddString(GetName());
        msg.AddString(std::to_string(mailBox_->GetTotalMailCount()));
        client_->WriteToOutput(msg);
    }
}

void Player::PartyInvitePlayer(uint32_t playerId)
{
    if (GetGame()->data_.type != AB::Entities::GameTypeOutpost)
        return;
    if (id_ == playerId)
        return;

    std::shared_ptr<Player> player = PlayerManager::Instance.GetPlayerById(playerId);
    if (player)
    {
        if (GetParty()->Invite(player))
        {
            Net::NetworkMessage nmsg;
            nmsg.AddByte(AB::GameProtocol::PartyPlayerInvited);
            nmsg.Add<uint32_t>(id_);
            nmsg.Add<uint32_t>(playerId);
            // Send us confirmation
            party_->WriteToMembers(nmsg);
            // Send player he was invited
            player->client_->WriteToOutput(nmsg);
        }
    }
}

void Player::PartyKickPlayer(uint32_t playerId)
{
    if (GetGame()->data_.type != AB::Entities::GameTypeOutpost)
        return;
    if (id_ == playerId)
        return;
    if (!party_)
        return;
    std::shared_ptr<Player> player = PlayerManager::Instance.GetPlayerById(playerId);
    if (!player)
        return;

    Net::NetworkMessage nmsg;
    nmsg.AddByte(AB::GameProtocol::PartyPlayerRemoved);
    nmsg.Add<uint32_t>(id_);
    nmsg.Add<uint32_t>(playerId);
    party_->WriteToMembers(nmsg);
    // Remove after
    party_->Remove(player);
}

void Player::PartyLeave()
{
    if (GetGame()->data_.type != AB::Entities::GameTypeOutpost)
        return;
    if (party_)
    {
        Net::NetworkMessage nmsg;
        nmsg.AddByte(AB::GameProtocol::PartyPlayerRemoved);
        nmsg.Add<uint32_t>(party_->GetLeader()->id_);
        nmsg.Add<uint32_t>(id_);
        party_->WriteToMembers(nmsg);
        party_->Remove(GetThis());
    }
}

void Player::PartyAccept(uint32_t playerId)
{
    if (GetGame()->data_.type != AB::Entities::GameTypeOutpost)
        return;

    std::shared_ptr<Player> player = PlayerManager::Instance.GetPlayerById(playerId);
    if (player)
    {
        if (player->GetParty()->Add(GetThis()))
        {
            Net::NetworkMessage nmsg;
            nmsg.AddByte(AB::GameProtocol::PartyPlayerAdded);
            nmsg.Add<uint32_t>(id_);
            nmsg.Add<uint32_t>(playerId);
            party_->WriteToMembers(nmsg);
        }
    }
}

void Player::HandleCommand(AB::GameProtocol::CommandTypes type,
    const std::string& command, Net::NetworkMessage& message)
{
    switch (type)
    {
    case AB::GameProtocol::CommandTypeChatGeneral:
    {
        HandleGeneralChatCommand(command, message);
        break;
    }
    case AB::GameProtocol::CommandTypeChatParty:
    {
        HandlePartyChatCommand(command, message);
        break;
    }
    case AB::GameProtocol::CommandTypeRoll:
    {
        HandleRollCommand(command, message);
        break;
    }
    case AB::GameProtocol::CommandTypeSit:
    {
        HandleSitCommand(command, message);
        break;
    }
    case AB::GameProtocol::CommandTypeStand:
    {
        HandleStandCommand(command, message);
        break;
    }
    case AB::GameProtocol::CommandTypeCry:
    {
        HandleCryCommand(command, message);
        break;
    }
    case AB::GameProtocol::CommandTypeAge:
    {
        HandleAgeCommand(command, message);
        break;
    }
    case AB::GameProtocol::CommandTypeChatWhisper:
    {
        HandleWhisperCommand(command, message);
        break;
    }
    case AB::GameProtocol::CommandTypeChatGuild:
    {
        HandleChatGuildCommand(command, message);
        break;
    }
    case AB::GameProtocol::CommandTypeChatTrade:
    {
        HandleChatTradeCommand(command, message);
        break;
    }
    case AB::GameProtocol::CommandTypeServerId:
    {
        HandleServerIdCommand(command, message);
        break;
    }
    }
}

void Player::HandleServerIdCommand(const std::string&, Net::NetworkMessage&)
{
    Net::NetworkMessage nmsg;
    nmsg.AddByte(AB::GameProtocol::ServerMessage);
    if (account_.type >= AB::Entities::AccountTypeGamemaster)
    {
        // Since it's more for debugging, it's only available for >= GM
        nmsg.AddByte(AB::GameProtocol::ServerMessageTypeServerId);
        nmsg.AddString(GetName());
        nmsg.AddString(Application::Instance->GetServerId());
    }
    else
    {
        nmsg.AddByte(AB::GameProtocol::ServerMessageTypeUnknownCommand);
        nmsg.AddString(GetName());
        nmsg.AddString("");
    }
    client_->WriteToOutput(nmsg);
}

void Player::HandleWhisperCommand(const std::string& command, Net::NetworkMessage&)
{
    size_t p = command.find(',');
    if (p == std::string::npos)
        return;

    const std::string name = command.substr(0, p);
    const std::string msg = Utils::LeftTrim(command.substr(p + 1, std::string::npos));
    std::shared_ptr<Player> target = PlayerManager::Instance.GetPlayerByName(name);
    if (target)
    {
        std::shared_ptr<ChatChannel> channel = Chat::Instance.Get(ChannelWhisper, target->id_);
        if (channel)
        {
            if (channel->Talk(this, msg))
            {
                Net::NetworkMessage nmsg;
                nmsg.AddByte(AB::GameProtocol::ServerMessage);
                nmsg.AddByte(AB::GameProtocol::ServerMessageTypePlayerGotMessage);
                nmsg.AddString(name);
                nmsg.AddString(msg);
                client_->WriteToOutput(nmsg);
            }
        }
        return;
    }

    AB_PROFILE;
    IO::DataClient* cli = Application::Instance->GetDataClient();
    AB::Entities::Character character;
    character.name = name;
    if (cli->Read(character) && (character.lastLogin > character.lastLogout))
    {
        // Is online
        std::shared_ptr<ChatChannel> channel = Chat::Instance.Get(ChannelWhisper, character.uuid);
        if (channel->Talk(this, msg))
        {
            Net::NetworkMessage nmsg;
            nmsg.AddByte(AB::GameProtocol::ServerMessage);
            nmsg.AddByte(AB::GameProtocol::ServerMessageTypePlayerGotMessage);
            nmsg.AddString(name);
            nmsg.AddString(msg);
            client_->WriteToOutput(nmsg);
            return;
        }
    }

    // Send not online message
    Net::NetworkMessage nmsg;
    nmsg.AddByte(AB::GameProtocol::ServerMessage);
    nmsg.AddByte(AB::GameProtocol::ServerMessageTypePlayerNotOnline);
    nmsg.AddString(GetName());
    nmsg.AddString(name);
    client_->WriteToOutput(nmsg);
}

void Player::HandleChatGuildCommand(const std::string& command, Net::NetworkMessage&)
{
    std::shared_ptr<ChatChannel> channel = Chat::Instance.Get(ChannelGuild, account_.guildUuid);
    if (channel)
    {
        channel->Talk(this, command);
    }
}

void Player::HandleChatTradeCommand(const std::string& command, Net::NetworkMessage&)
{
    std::shared_ptr<ChatChannel> channel = Chat::Instance.Get(ChannelTrade, 0);
    if (channel)
    {
        channel->Talk(this, command);
    }
}

void Player::HandleAgeCommand(const std::string&, Net::NetworkMessage&)
{
    // In seconds
    uint32_t playTime = static_cast<uint32_t>(data_.onlineTime) +
        static_cast<uint32_t>((Utils::AbTick() - loginTime_) / 1000);
    // In seconds
    uint32_t age = static_cast<uint32_t>((Utils::AbTick() - data_.creation) / 1000);

    Net::NetworkMessage nmsg;
    nmsg.AddByte(AB::GameProtocol::ServerMessage);
    nmsg.AddByte(AB::GameProtocol::ServerMessageTypeAge);
    nmsg.AddString(GetName());
    nmsg.AddString(std::to_string(age) + ":" + std::to_string(playTime));
    client_->WriteToOutput(nmsg);
}

void Player::HandleRollCommand(const std::string& command, Net::NetworkMessage& message)
{
    if (Utils::IsNumber(command))
    {
        int max = std::stoi(command);
        if (max >= ROLL_MIN && max <= ROLL_MAX)
        {
            int res = static_cast<int>(Utils::Random::Instance.GetFloat() * (float)max) + 1;
            message.AddByte(AB::GameProtocol::ServerMessage);
            message.AddByte(AB::GameProtocol::ServerMessageTypeRoll);
            message.AddString(GetName());
            message.AddString(std::to_string(res) + ":" + std::to_string(max));
        }
    }
}

void Player::HandleSitCommand(const std::string&, Net::NetworkMessage&)
{
    stateComp_.SetState(AB::GameProtocol::CreatureStateEmoteSit);
}

void Player::HandleStandCommand(const std::string&, Net::NetworkMessage&)
{
    if (stateComp_.GetState() == AB::GameProtocol::CreatureStateEmoteSit)
        stateComp_.SetState(AB::GameProtocol::CreatureStateIdle);
}

void Player::HandleCryCommand(const std::string&, Net::NetworkMessage&)
{
    stateComp_.SetState(AB::GameProtocol::CreatureStateEmoteCry);
}

void Player::HandleGeneralChatCommand(const std::string& command, Net::NetworkMessage&)
{
    std::shared_ptr<ChatChannel> channel = Chat::Instance.Get(ChannelMap, GetGame()->id_);
    if (channel)
    {
        channel->Talk(this, command);
    }
}

void Player::HandlePartyChatCommand(const std::string& command, Net::NetworkMessage&)
{
    std::shared_ptr<ChatChannel> channel = Chat::Instance.Get(ChannelParty, GetParty()->id_);
    if (channel)
    {
        channel->Talk(this, command);
    }
}

void Player::ChangeInstance(const std::string& mapUuid)
{
    // TODO: all in the party must change to the same instance
    std::shared_ptr<Game> game = GameManager::Instance.GetGame(mapUuid, true);
    if (game)
    {
        stateComp_.SetState(AB::GameProtocol::CreatureStateIdle);
        client_->ChangeInstance(mapUuid, game->instanceData_.uuid);
    }
}

void Player::RegisterLua(kaguya::State& state)
{
    state["Player"].setClass(kaguya::UserdataMetatable<Player, Actor>()
        .addFunction("ChangeInstance", &Player::ChangeInstance)
    );
}

}
