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
    client_(client),
    lastPing_(0),
    mailBox_(nullptr),
    party_(nullptr),
    questComp_(std::make_unique<Components::QuestComp>(*this))
{ }

Player::~Player() = default;

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

size_t Player::GetGroupPos()
{
    return party_->GetPosition(this);
}

void Player::AddXp(int value)
{
    Actor::AddXp(value);
    data_.xp += value;
}

void Player::AddSkillPoint()
{
    Actor::AddSkillPoint();
    ++data_.skillPoints;
}

void Player::AdvanceLevel()
{
    if (data_.level < LEVEL_CAP)
        ++data_.level;
}

void Player::Initialize()
{
    Actor::Initialize();
    SetParty(GetSubsystem<PartyManager>()->GetByUuid(data_.partyUuid));
}

void Player::Logout()
{
#ifdef DEBUG_GAME
    LOG_DEBUG << "Player logging out " << GetName() << std::endl;
#endif // DEBUG_GAME

    if (auto g = GetGame())
        g->PlayerLeave(id_);
    client_->Logout();
}

void Player::Ping()
{
    lastPing_ = Utils::Tick();
    Net::NetworkMessage msg;
    msg.AddByte(AB::GameProtocol::GamePong);
    WriteToOutput(msg);
}

void Player::UpdateMailBox()
{
    if (!mailBox_ && !data_.accountUuid.empty() && !uuids::uuid(data_.accountUuid).nil())
        mailBox_ = std::make_unique<MailBox>(data_.accountUuid);
    if (mailBox_)
        mailBox_->Update();
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
    WriteToOutput(msg);
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
    WriteToOutput(nmsg);
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
        WriteToOutput(msg);
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
        WriteToOutput(msg);
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
        WriteToOutput(msg);
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
        WriteToOutput(msg);
    }
    if (mailBox_->GetTotalMailCount() >= AB::Entities::Limits::MAX_MAIL_COUNT)
    {
        // Notify player that mailbox is full.
        Net::NetworkMessage msg;
        msg.AddByte(AB::GameProtocol::ServerMessage);
        msg.AddByte(AB::GameProtocol::ServerMessageTypeMailboxFull);
        msg.AddString(GetName());
        msg.AddString(std::to_string(mailBox_->GetTotalMailCount()));
        WriteToOutput(msg);
    }
}

void Player::WriteToOutput(const Net::NetworkMessage& message)
{
    if (client_)
        client_->WriteToOutput(message);
    else
        LOG_ERROR << "client_ expired" << std::endl;
}

void Player::OnPingObject(uint32_t targetId, AB::GameProtocol::ObjectCallType type, int skillIndex)
{
    Actor::OnPingObject(targetId, type, skillIndex);

    Net::NetworkMessage msg;
    msg.AddByte(AB::GameProtocol::GameObjectPingTarget);
    msg.Add<uint32_t>(id_);
    msg.Add<uint32_t>(targetId);
    msg.Add<uint8_t>(static_cast<uint8_t>(type));
    msg.Add<int8_t>(static_cast<int8_t>(skillIndex));
    GetParty()->WriteToMembers(msg);
}

void Player::SetParty(std::shared_ptr<Party> party)
{
    if (party_)
    {
        if (party && (party_->id_ == party->id_))
            return;
        party_->Remove(this, false);
    }

    if (party)
    {
        party_ = party;
        data_.partyUuid = party->data_.uuid;
    }
    else
    {
        // Create new party
        data_.partyUuid.clear();
        party_ = GetSubsystem<PartyManager>()->GetByUuid(data_.partyUuid);
        party_->SetPartySize(GetGame()->data_.partySize);
        data_.partyUuid = party_->data_.uuid;
    }
    party_->Set(GetThis());
}

void Player::Update(uint32_t timeElapsed, Net::NetworkMessage& message)
{
    Actor::Update(timeElapsed, message);
    questComp_->Update(timeElapsed);
    questComp_->Write(message);
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
            nmsg.Add<uint32_t>(id_);                             // Leader
            nmsg.Add<uint32_t>(playerId);                        // Invitee
            nmsg.Add<uint32_t>(party_->id_);
            // Send us confirmation
            party_->WriteToMembers(nmsg);
            // Send player he was invited
            player->WriteToOutput(nmsg);
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

    bool removedMember = false;
    {
        Net::NetworkMessage nmsg;
        if (party_->IsMember(player.get()))
        {
            if (!party_->Remove(player.get()))
                return;
            nmsg.AddByte(AB::GameProtocol::PartyPlayerRemoved);
            removedMember = true;
        }
        else if (party_->IsInvited(player.get()))
        {
            if (!party_->RemoveInvite(player))
                return;
            nmsg.AddByte(AB::GameProtocol::PartyInviteRemoved);
        }
        else
            return;

        nmsg.Add<uint32_t>(id_);                 // Leader
        nmsg.Add<uint32_t>(playerId);            // Member
        nmsg.Add<uint32_t>(party_->id_);
        party_->WriteToMembers(nmsg);

        // Also send to player which is removed already
        player->WriteToOutput(nmsg);
    }

    if (removedMember)
    {
        // The kicked player needs a new party
        player->SetParty(std::shared_ptr<Party>());
        Net::NetworkMessage nmsg;
        nmsg.AddByte(AB::GameProtocol::PartyPlayerAdded);
        nmsg.Add<uint32_t>(player->id_);                           // Acceptor
        nmsg.Add<uint32_t>(player->id_);                           // Leader
        nmsg.Add<uint32_t>(player->GetParty()->id_);
        player->GetParty()->WriteToMembers(nmsg);
    }
}

void Player::PartyLeave()
{
    if (party_->IsLeader(this) && party_->GetMemberCount() == 1)
        // Just we
        return;

    {
        Net::NetworkMessage nmsg;
        nmsg.AddByte(AB::GameProtocol::PartyPlayerRemoved);
        nmsg.Add<uint32_t>(party_->GetLeader()->id_);
        nmsg.Add<uint32_t>(id_);
        nmsg.Add<uint32_t>(party_->id_);
        party_->WriteToMembers(nmsg);
        party_->Remove(this);
    }

    {
        // We need a new party
        SetParty(std::shared_ptr<Party>());
        Net::NetworkMessage nmsg;
        nmsg.AddByte(AB::GameProtocol::PartyPlayerAdded);
        nmsg.Add<uint32_t>(id_);                           // Acceptor
        nmsg.Add<uint32_t>(id_);                           // Leader
        nmsg.Add<uint32_t>(party_->id_);
        party_->WriteToMembers(nmsg);
    }
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
            party_->WriteToMembers(nmsg);
#ifdef DEBUG_GAME
            LOG_DEBUG << "Acceptor: " << id_ << ", Leader: " << playerId << ", Party: " << party_->id_ << std::endl;
#endif
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
            WriteToOutput(nmsg);
        }
    }
}

void Player::PartyGetMembers(uint32_t partyId)
{
    std::shared_ptr<Party> party = GetSubsystem<PartyManager>()->Get(partyId);
    if (party)
    {
        Net::NetworkMessage nmsg;
        nmsg.AddByte(AB::GameProtocol::PartyInfoMembers);
        nmsg.Add<uint32_t>(partyId);
        size_t count = party->GetMemberCount();
        const auto& members = party->GetMembers();
        nmsg.AddByte(static_cast<uint8_t>(count));
        for (auto& m : members)
        {
            if (auto sm = m.lock())
                nmsg.Add<uint32_t>(sm->id_);
            else
                nmsg.Add<uint32_t>(0);
        }
        WriteToOutput(nmsg);
#ifdef DEBUG_GAME
        LOG_DEBUG << "Player: " << id_ << ", Party: " << partyId << ", Count: " << static_cast<int>(count) << std::endl;
#endif
    }
#ifdef DEBUG_GAME
    else
        LOG_DEBUG << "Party not found: " << partyId << std::endl;
#endif
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
    case AB::GameProtocol::CommandTypeHealth:
        HandleHpCommand(command, message);
        break;
    case AB::GameProtocol::CommandTypePos:
        HandlePosCommand(command, message);
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
    WriteToOutput(nmsg);
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
                WriteToOutput(nmsg);
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
            WriteToOutput(nmsg);
            return;
        }
    }

    // Send not online message
    Net::NetworkMessage nmsg;
    nmsg.AddByte(AB::GameProtocol::ServerMessage);
    nmsg.AddByte(AB::GameProtocol::ServerMessageTypePlayerNotOnline);
    nmsg.AddString(GetName());
    nmsg.AddString(name);
    WriteToOutput(nmsg);
}

void Player::HandleChatGuildCommand(const std::string& command, Net::NetworkMessage&)
{
    std::shared_ptr<ChatChannel> channel = GetSubsystem<Chat>()->Get(ChatType::Guild, account_.guildUuid);
    if (channel)
        channel->Talk(this, command);
}

void Player::HandleChatTradeCommand(const std::string& command, Net::NetworkMessage&)
{
    std::shared_ptr<ChatChannel> channel = GetSubsystem<Chat>()->Get(ChatType::Trade, 0);
    if (channel)
        channel->Talk(this, command);
}

void Player::HandleAgeCommand(const std::string&, Net::NetworkMessage&)
{
    // In seconds
    uint32_t playTime = static_cast<uint32_t>(data_.onlineTime) +
        static_cast<uint32_t>((Utils::Tick() - loginTime_) / 1000);
    // In seconds
    uint32_t age = static_cast<uint32_t>((Utils::Tick() - data_.creation) / 1000);

    Net::NetworkMessage nmsg;
    nmsg.AddByte(AB::GameProtocol::ServerMessage);
    nmsg.AddByte(AB::GameProtocol::ServerMessageTypeAge);
    nmsg.AddString(GetName());
    nmsg.AddString(std::to_string(age) + ":" + std::to_string(playTime));
    WriteToOutput(nmsg);
}

void Player::HandleHpCommand(const std::string&, Net::NetworkMessage&)
{
    Net::NetworkMessage nmsg;
    int maxHp = resourceComp_.GetMaxHealth();
    int hp = resourceComp_.GetHealth();
    int maxE = resourceComp_.GetMaxEnergy();
    int e = resourceComp_.GetEnergy();
    nmsg.AddByte(AB::GameProtocol::ServerMessage);
    nmsg.AddByte(AB::GameProtocol::ServerMessageTypeHp);
    nmsg.AddString(GetName());
    nmsg.AddString(std::to_string(hp) + ":" + std::to_string(maxHp) + "|" + std::to_string(e) + ":" + std::to_string(maxE));
    WriteToOutput(nmsg);
}

void Player::HandlePosCommand(const std::string&, Net::NetworkMessage&)
{
    if (account_.type < AB::Entities::AccountTypeGamemaster)
    {
        Net::NetworkMessage nmsg;
        nmsg.AddByte(AB::GameProtocol::ServerMessage);
        nmsg.AddByte(AB::GameProtocol::ServerMessageTypeUnknownCommand);
        nmsg.AddString(GetName());
        nmsg.AddString("");
        WriteToOutput(nmsg);
        return;
    }

    Net::NetworkMessage nmsg;
    std::stringstream ss;
    ss << transformation_.position_.x_ << "," <<
        transformation_.position_.y_ << "," <<
        transformation_.position_.z_;
    ss << " " << transformation_.GetYRotation();
    nmsg.AddByte(AB::GameProtocol::ServerMessage);
    nmsg.AddByte(AB::GameProtocol::ServerMessageTypePos);
    nmsg.AddString(GetName());
    nmsg.AddString(ss.str());
    WriteToOutput(nmsg);
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
    if (attackComp_.IsAttackState())
        attackComp_.Cancel();
    stateComp_.SetState(AB::GameProtocol::CreatureStateEmoteSit);
}

void Player::HandleStandCommand(const std::string&, Net::NetworkMessage&)
{
    if (stateComp_.GetState() == AB::GameProtocol::CreatureStateEmoteSit)
        stateComp_.SetState(AB::GameProtocol::CreatureStateIdle);
}

void Player::HandleCryCommand(const std::string&, Net::NetworkMessage&)
{
    if (attackComp_.IsAttackState())
        attackComp_.Cancel();
    stateComp_.SetState(AB::GameProtocol::CreatureStateEmoteCry);
}

void Player::HandleTauntCommand(const std::string&, Net::NetworkMessage&)
{
    if (attackComp_.IsAttackState())
        attackComp_.Cancel();
    stateComp_.SetState(AB::GameProtocol::CreatureStateEmoteTaunt);
}

void Player::HandlePonderCommand(const std::string&, Net::NetworkMessage&)
{
    if (attackComp_.IsAttackState())
        attackComp_.Cancel();
    stateComp_.SetState(AB::GameProtocol::CreatureStateEmotePonder);
}

void Player::HandleWaveCommand(const std::string&, Net::NetworkMessage&)
{
    if (attackComp_.IsAttackState())
        attackComp_.Cancel();
    stateComp_.SetState(AB::GameProtocol::CreatureStateEmoteWave);
}

void Player::HandleLaughCommand(const std::string&, Net::NetworkMessage&)
{
    if (attackComp_.IsAttackState())
        attackComp_.Cancel();
    stateComp_.SetState(AB::GameProtocol::CreatureStateEmoteLaugh);
}

void Player::HandleDieCommand(const std::string&, Net::NetworkMessage&)
{
    if (account_.type >= AB::Entities::AccountTypeGamemaster)
        Die();
    else
    {
        Net::NetworkMessage nmsg;
        nmsg.AddByte(AB::GameProtocol::ServerMessage);
        nmsg.AddByte(AB::GameProtocol::ServerMessageTypeUnknownCommand);
        nmsg.AddString(GetName());
        nmsg.AddString("");
        WriteToOutput(nmsg);
    }
}

void Player::HandleGeneralChatCommand(const std::string& command, Net::NetworkMessage&)
{
    std::shared_ptr<ChatChannel> channel = GetSubsystem<Chat>()->Get(ChatType::Map, GetGame()->id_);
    if (channel)
        channel->Talk(this, command);
}

void Player::HandlePartyChatCommand(const std::string& command, Net::NetworkMessage&)
{
    std::shared_ptr<ChatChannel> channel = GetSubsystem<Chat>()->Get(ChatType::Party, GetParty()->id_);
    if (channel)
        channel->Talk(this, command);
}

void Player::ChangeMap(const std::string mapUuid)
{
    // mapUuid No reference

    // If we are the leader tell all members to change the instance.
    // If not, tell the leader to change the instance.
    auto party = GetParty();
    if (!party)
        return;

    auto game = GetGame();
    if (game && game->data_.type != AB::Entities::GameTypeOutpost)
    {
        // The player leaves the party and changes the instance
        PartyLeave();
        party = GetParty();
    }
    if (party->IsLeader(this))
        // If we are the leader tell all members to change the instance.
        party->ChangeInstance(mapUuid);
}

void Player::ChangeInstance(const std::string& mapUuid, const std::string& instanceUuid)
{
    if (client_)
        client_->ChangeInstance(mapUuid, instanceUuid);
    else
        LOG_ERROR << "client_ = null" << std::endl;
}

void Player::RegisterLua(kaguya::State& state)
{
    state["Player"].setClass(kaguya::UserdataMetatable<Player, Actor>()
        .addFunction("ChangeMap", &Player::ChangeMap)
    );
}

}
