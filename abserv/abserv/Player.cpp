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
#include "PartyManager.h"

#include "DebugNew.h"

namespace Game {

Player::Player(std::shared_ptr<Net::ProtocolGame> client) :
    Actor(),
    client_(std::move(client)),
    lastPing_(0),
    mailBox_(nullptr),
    party_(nullptr)
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
    // Changing the instance also clears any invites. The client should check that we
    // leave the instance so don't send anything to invitees.
    party_->ClearInvites();
    if (game)
    {
        if (party_->IsLeader(this))
            party_->SetPartySize(game->data_.partySize);
    }
}

uint8_t Player::GetGroupPos() const
{
    return party_->GetPosition(const_cast<const Player*>(this));
}

void Player::Initialize()
{
    party_ = GetSubsystem<PartyManager>()->GetParty(GetThis(), data_.partyUuid);
    data_.partyUuid = party_->data_.uuid;
}

void Player::Logout()
{
    if (auto g = GetGame())
        g->PlayerLeave(id_);
    party_->Remove(GetThis());
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

void Player::SetParty(std::shared_ptr<Party> party)
{
    if (party)
    {
        party_ = party;
        data_.partyUuid = party->data_.uuid;
    }
    else
    {
        // Create new party
        data_.partyUuid.clear();
        party_ = GetSubsystem<PartyManager>()->GetParty(GetThis(), data_.partyUuid);
        data_.partyUuid = party_->data_.uuid;
    }
}

void Player::PartyInvitePlayer(uint32_t playerId)
{
    // The leader invited a player
    if (GetGame()->data_.type != AB::Entities::GameTypeOutpost)
        return;
    if (id_ == playerId)
        return;
    if (!party_->IsLeader(this))
        return;
    if (party_->IsFull())
        return;
    std::shared_ptr<Player> player = GetSubsystem<PlayerManager>()->GetPlayerById(playerId);
    if (player)
    {
        if (party_->Invite(player))
        {
            Net::NetworkMessage nmsg;
            nmsg.AddByte(AB::GameProtocol::PartyPlayerInvited);
            nmsg.Add<uint32_t>(id_);
            nmsg.Add<uint32_t>(playerId);
            nmsg.Add<uint32_t>(party_->id_);
            // Send us confirmation
            party_->WriteToMembers(nmsg);
            // Send player he was invited
            player->client_->WriteToOutput(nmsg);
        }
    }
}

void Player::PartyKickPlayer(uint32_t playerId)
{
    // The leader kicks a player from the party
    if (GetGame()->data_.type != AB::Entities::GameTypeOutpost)
        return;
    if (id_ == playerId)
        // Can not kick myself
        return;
    if (!party_->IsLeader(this))
        // Only leader can kick
        return;

    std::shared_ptr<Player> player = GetSubsystem<PlayerManager>()->GetPlayerById(playerId);
    if (!player)
        return;

    Net::NetworkMessage nmsg;
    if (party_->IsMember(player))
    {
        if (!party_->Remove(player))
            return;
        nmsg.AddByte(AB::GameProtocol::PartyPlayerRemoved);
    }
    else if (party_->IsInvited(player))
    {
        if (!party_->RemoveInvite(player))
            return;
        nmsg.AddByte(AB::GameProtocol::PartyInviteRemoved);
    }
    else
        return;

    nmsg.Add<uint32_t>(id_);
    nmsg.Add<uint32_t>(playerId);
    nmsg.Add<uint32_t>(party_->id_);
    party_->WriteToMembers(nmsg);
    // Remove after
    party_->Remove(player);
    // Inform the player
    player->client_->WriteToOutput(nmsg);
}

void Player::PartyLeave()
{
    // A player leaves the party
    if (GetGame()->data_.type != AB::Entities::GameTypeOutpost)
        return;
    if (party_)
    {
        Net::NetworkMessage nmsg;
        nmsg.AddByte(AB::GameProtocol::PartyPlayerRemoved);
        nmsg.Add<uint32_t>(party_->GetLeader()->id_);
        nmsg.Add<uint32_t>(id_);
        nmsg.Add<uint32_t>(party_->id_);
        party_->WriteToMembers(nmsg);
        party_->Remove(GetThis());
    }

    // We need a new party
    SetParty(std::shared_ptr<Party>());
    Net::NetworkMessage nmsg;
    nmsg.AddByte(AB::GameProtocol::PartyPlayerAdded);
    nmsg.Add<uint32_t>(id_);                           // Acceptor
    nmsg.Add<uint32_t>(id_);                           // Leader
    nmsg.Add<uint32_t>(party_->id_);
    party_->WriteToMembers(nmsg);
}

void Player::PartyAccept(uint32_t playerId)
{
    // Sent by the acceptor to the leader of the party that a player accepted
    if (GetGame()->data_.type != AB::Entities::GameTypeOutpost)
        return;

    std::shared_ptr<Player> leader = GetSubsystem<PlayerManager>()->GetPlayerById(playerId);
    if (leader)
    {
        // Leave current party
        PartyLeave();
        if (leader->GetParty()->Add(GetThis()))
        {
            Net::NetworkMessage nmsg;
            nmsg.AddByte(AB::GameProtocol::PartyPlayerAdded);
            nmsg.Add<uint32_t>(id_);                           // Acceptor
            nmsg.Add<uint32_t>(playerId);                      // Leader
            nmsg.Add<uint32_t>(party_->id_);
            // TODO: All current members
            const size_t memberCount = party_->GetMemberCount();
            nmsg.Add<uint8_t>(static_cast<uint8_t>(memberCount));
            const auto& members = party_->GetMembers();
            for (size_t i = 0; i < memberCount; ++i)
            {
                if (auto m = members[i].lock())
                {
                    nmsg.Add<uint32_t>(m->id_);
                }
            }
            party_->WriteToMembers(nmsg);
        }
        // else party maybe full
    }
}

void Player::PartyRejectInvite(uint32_t inviterId)
{
    // We are the rejector
    if (GetGame()->data_.type != AB::Entities::GameTypeOutpost)
        return;
    std::shared_ptr<Player> leader = GetSubsystem<PlayerManager>()->GetPlayerById(inviterId);
    if (leader)
    {
        if (leader->GetParty()->RemoveInvite(GetThis()))
        {
            Net::NetworkMessage nmsg;
            nmsg.AddByte(AB::GameProtocol::PartyInviteRemoved);
            nmsg.Add<uint32_t>(inviterId);                // Leader
            nmsg.Add<uint32_t>(id_);                      // We
            nmsg.Add<uint32_t>(leader->GetParty()->id_);
            // Inform the party
            leader->GetParty()->WriteToMembers(nmsg);
            // Inform us
            client_->WriteToOutput(nmsg);
        }
    }
}

void Player::PartyGetMembers(uint32_t partyId)
{
    std::shared_ptr<Party> party = GetSubsystem<PartyManager>()->GetPartyById(partyId);
    if (party)
    {
        Net::NetworkMessage nmsg;
        nmsg.AddByte(AB::GameProtocol::PartyInfoMembers);
        nmsg.Add<uint32_t>(partyId);
        size_t count = party->GetMemberCount();
        const auto& members = party->GetMembers();
        nmsg.AddByte(static_cast<uint8_t>(count));
        for (size_t i = 0; i < count; i++)
        {
            if (auto m = members[i].lock())
                nmsg.Add<uint32_t>(m->id_);
            else
                nmsg.Add<uint32_t>(0);
        }
        client_->WriteToOutput(nmsg);
    }
}

void Player::HandleCommand(AB::GameProtocol::CommandTypes type,
    const std::string& command, Net::NetworkMessage& message)
{
    switch (type)
    {
    case AB::GameProtocol::CommandTypeChatGeneral:
        HandleGeneralChatCommand(command, message);
        break;
    case AB::GameProtocol::CommandTypeChatParty:
        HandlePartyChatCommand(command, message);
        break;
    case AB::GameProtocol::CommandTypeRoll:
        HandleRollCommand(command, message);
        break;
    case AB::GameProtocol::CommandTypeSit:
        HandleSitCommand(command, message);
        break;
    case AB::GameProtocol::CommandTypeStand:
        HandleStandCommand(command, message);
        break;
    case AB::GameProtocol::CommandTypeCry:
        HandleCryCommand(command, message);
        break;
    case AB::GameProtocol::CommandTypeTaunt:
        HandleTauntCommand(command, message);
        break;
    case AB::GameProtocol::CommandTypePonder:
        HandlePonderCommand(command, message);
        break;
    case AB::GameProtocol::CommandTypeWave:
        HandleWaveCommand(command, message);
        break;
    case AB::GameProtocol::CommandTypeLaugh:
        HandleLaughCommand(command, message);
        break;
    case AB::GameProtocol::CommandTypeAge:
        HandleAgeCommand(command, message);
        break;
    case AB::GameProtocol::CommandTypeChatWhisper:
        HandleWhisperCommand(command, message);
        break;
    case AB::GameProtocol::CommandTypeChatGuild:
        HandleChatGuildCommand(command, message);
        break;
    case AB::GameProtocol::CommandTypeChatTrade:
        HandleChatTradeCommand(command, message);
        break;
    case AB::GameProtocol::CommandTypeServerId:
        HandleServerIdCommand(command, message);
        break;
    case AB::GameProtocol::CommandTypeDie:
        HandleDieCommand(command, message);
        break;
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
    std::shared_ptr<Player> target = GetSubsystem<PlayerManager>()->GetPlayerByName(name);
    if (target)
    {
        std::shared_ptr<ChatChannel> channel = GetSubsystem<Chat>()->Get(ChatType::Whisper, target->id_);
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
    IO::DataClient* cli = GetSubsystem<IO::DataClient>();
    AB::Entities::Character character;
    character.name = name;
    if (cli->Read(character) && (character.lastLogin > character.lastLogout))
    {
        // Is online
        std::shared_ptr<ChatChannel> channel = GetSubsystem<Chat>()->Get(ChatType::Whisper, character.uuid);
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
    std::shared_ptr<ChatChannel> channel = GetSubsystem<Chat>()->Get(ChatType::Guild, account_.guildUuid);
    if (channel)
    {
        channel->Talk(this, command);
    }
}

void Player::HandleChatTradeCommand(const std::string& command, Net::NetworkMessage&)
{
    std::shared_ptr<ChatChannel> channel = GetSubsystem<Chat>()->Get(ChatType::Trade, 0);
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
            int res = static_cast<int>(GetSubsystem<Crypto::Random>()->GetFloat() * (float)max) + 1;
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

void Player::HandleTauntCommand(const std::string&, Net::NetworkMessage&)
{
    stateComp_.SetState(AB::GameProtocol::CreatureStateEmoteTaunt);
}

void Player::HandlePonderCommand(const std::string&, Net::NetworkMessage&)
{
    stateComp_.SetState(AB::GameProtocol::CreatureStateEmotePonder);
}

void Player::HandleWaveCommand(const std::string&, Net::NetworkMessage&)
{
    stateComp_.SetState(AB::GameProtocol::CreatureStateEmoteWave);
}

void Player::HandleLaughCommand(const std::string&, Net::NetworkMessage&)
{
    stateComp_.SetState(AB::GameProtocol::CreatureStateEmoteLaugh);
}

void Player::HandleDieCommand(const std::string&, Net::NetworkMessage&)
{
    if (account_.type >= AB::Entities::AccountTypeGamemaster)
    {
        Die();
    }
    else
    {
        Net::NetworkMessage nmsg;
        nmsg.AddByte(AB::GameProtocol::ServerMessage);
        nmsg.AddByte(AB::GameProtocol::ServerMessageTypeUnknownCommand);
        nmsg.AddString(GetName());
        nmsg.AddString("");
        client_->WriteToOutput(nmsg);
    }
}

void Player::HandleGeneralChatCommand(const std::string& command, Net::NetworkMessage&)
{
    std::shared_ptr<ChatChannel> channel = GetSubsystem<Chat>()->Get(ChatType::Map, GetGame()->id_);
    if (channel)
    {
        channel->Talk(this, command);
    }
}

void Player::HandlePartyChatCommand(const std::string& command, Net::NetworkMessage&)
{
    std::shared_ptr<ChatChannel> channel = GetSubsystem<Chat>()->Get(ChatType::Party, GetParty()->id_);
    if (channel)
    {
        channel->Talk(this, command);
    }
}

void Player::ChangeInstance(const std::string mapUuid)
{
    // mapUuid No reference

    // If we are the leader tell all members to change the instance.
    // If not, tell the leader to change the instance.
    auto party = GetParty();
    if (party->IsLeader(this))
        party->ChangeInstance(mapUuid);
    else if (GetGame()->data_.type == AB::Entities::GameTypeOutpost)
        // Only in outposts
        party->GetLeader()->ChangeInstance(mapUuid);
    else
    {
        // The player leaves the party and changes the instance
        PartyLeave();
        // Now we are the leader of a new party
        party_->ChangeInstance(mapUuid);
    }
}

void Player::RegisterLua(kaguya::State& state)
{
    state["Player"].setClass(kaguya::UserdataMetatable<Player, Actor>()
        .addFunction("ChangeInstance", &Player::ChangeInstance)
    );
}

}
