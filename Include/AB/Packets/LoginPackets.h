#pragma once

#include <string>
#include <vector>

namespace AB {
namespace Packets {

namespace Client {
namespace Login {

struct Login
{
    std::string accountName;
    std::string password;
    template<typename _Ar>
    void Serialize(_Ar& ar)
    {
        ar.value_enc(accountName);
        ar.value_enc(password);
    }
};

struct CreateAccount
{
    std::string accountName;
    std::string password;
    std::string email;
    std::string accountKey;
    template<typename _Ar>
    void Serialize(_Ar& ar)
    {
        ar.value_enc(accountName);
        ar.value_enc(password);
        ar.value_enc(email);
        ar.value_enc(accountKey);
    }
};

struct AddAccountKey
{
    std::string accountUuid;
    std::string authToken;
    std::string accountKey;
    template<typename _Ar>
    void Serialize(_Ar& ar)
    {
        ar.value_enc(accountUuid);
        ar.value_enc(authToken);
        ar.value_enc(accountKey);
    }
};

struct CreatePlayer
{
    std::string accountUuid;
    std::string authToken;
    std::string charName;
    uint32_t itemIndex;
    uint8_t sex;
    std::string profUuid;
    bool pvp;
    template<typename _Ar>
    void Serialize(_Ar& ar)
    {
        ar.value_enc(accountUuid);
        ar.value_enc(authToken);
        ar.value_enc(charName);
        ar.value(itemIndex);
        ar.value(sex);
        ar.value(profUuid);
        ar.value(pvp);
    }
};

struct DeleteCharacter
{
    std::string accountUuid;
    std::string authToken;
    std::string charUuid;
    template<typename _Ar>
    void Serialize(_Ar& ar)
    {
        ar.value_enc(accountUuid);
        ar.value_enc(authToken);
        ar.value_enc(charUuid);
    }
};

struct GetOutposts
{
    std::string accountUuid;
    std::string authToken;
    template<typename _Ar>
    void Serialize(_Ar& ar)
    {
        ar.value_enc(accountUuid);
        ar.value_enc(authToken);
    }
};

struct GetServers
{
    std::string accountUuid;
    std::string authToken;
    template<typename _Ar>
    void Serialize(_Ar& ar)
    {
        ar.value_enc(accountUuid);
        ar.value_enc(authToken);
    }
};

}  // namespace Login
}  // namerspace Client

namespace Server {
namespace Login {

struct Error
{
    uint8_t code;
    template<typename _Ar>
    void Serialize(_Ar& ar)
    {
        ar.value(code);
    }
};

struct CreateCharacterSuccess
{
    // UUID of newly created character
    std::string uuid;
    std::string mapUuid;
    template<typename _Ar>
    void Serialize(_Ar& ar)
    {
        ar.value_enc(uuid);
        ar.value_enc(mapUuid);
    }
};

struct CharacterList
{
    struct Character
    {
        std::string uuid;
        uint8_t level;
        std::string name;
        std::string profession;
        std::string profession2;
        uint8_t sex;
        uint32_t modelIndex;
        std::string outpostUuid;
    };

    std::string accountUuid;
    std::string authToken;
    std::string serverHost;
    uint16_t serverPort;
    std::string fileHost;
    uint16_t filePort;
    uint16_t charSlots;
    uint16_t charCount;
    std::vector<Character> characters;
    template<typename _Ar>
    void Serialize(_Ar& ar)
    {
        ar.value_enc(accountUuid);
        ar.value_enc(authToken);
        ar.value_enc(serverHost);
        ar.value(serverPort);
        ar.value_enc(fileHost);
        ar.value(filePort);
        ar.value(charSlots);
        ar.value(charCount);
        characters.resize(charCount);
        for (uint16_t i = 0; i < charCount; ++i)
        {
            auto& character = characters[i];
            ar.value_enc(character.uuid);
            ar.value(character.level);
            ar.value_enc(character.name);
            ar.value_enc(character.profession);
            ar.value_enc(character.profession2);
            ar.value(character.sex);
            ar.value(character.modelIndex);
            ar.value_enc(character.outpostUuid);
        }
    }
};

struct OutpostList
{
    struct Outpost
    {
        std::string uuid;
        std::string name;
        uint8_t type;
        uint8_t partySize;
        int32_t coordX;
        int32_t coordY;
    };

    uint16_t count;
    std::vector<Outpost> outposts;
    template<typename _Ar>
    void Serialize(_Ar& ar)
    {
        ar.value_enc(count);
        outposts.resize(count);
        for (uint16_t i = 0; i < count; ++i)
        {
            auto& outpost = outposts[i];
            ar.value_enc(outpost.uuid);
            ar.value_enc(outpost.name);
            ar.value(outpost.type);
            ar.value(outpost.partySize);
            ar.value(outpost.coordX);
            ar.value(outpost.coordY);
        }
    }
};

struct ServerList
{
    struct Server
    {
        uint8_t type;
        std::string uuid;
        std::string host;
        uint16_t port;
        std::string location;
        std::string name;
    };

    uint16_t count;
    std::vector<Server> servers;
    template<typename _Ar>
    void Serialize(_Ar& ar)
    {
        ar.value_enc(count);
        servers.resize(count);
        for (uint16_t i = 0; i < count; ++i)
        {
            auto& server = servers[i];
            ar.value(server.type);
            ar.value_enc(server.uuid);
            ar.value_enc(server.host);
            ar.value(server.port);
            ar.value_enc(server.location);
            ar.value_enc(server.name);
        }
    }
};

}  // namespace Login
}  // namespace Server

}  // namespace Packets
}
