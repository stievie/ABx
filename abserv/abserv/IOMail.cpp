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

#include "stdafx.h"
#include "IOMail.h"
#include "IOPlayer.h"

namespace IO {
namespace IOMail {

bool LoadMailList(AB::Entities::MailList& ml, const std::string& accountUuid)
{
    IO::DataClient* client = GetSubsystem<IO::DataClient>();
    ml.uuid = accountUuid;
    return client->Read(ml);
}

bool SendMailToPlayer(const std::string& playerName, const std::string& fromAcc,
    const std::string& fromName,
    const std::string& subject, const std::string& message)
{
    AB::Entities::Character ch;
    ch.name = playerName;

    // Get recipient account
    bool ret = IOPlayer::LoadCharacter(ch);
    if (!ret)
        return false;

    return SendMailToAccount(ch.accountUuid, fromAcc, fromName, playerName, subject, message);
}

bool SendMailToAccount(const std::string& accountUuid, const std::string& fromAcc,
    const std::string& fromName, const std::string& toName,
    const std::string& subject, const std::string& message)
{
    // Can not send mail to players that ignore me
    if (IOPlayer::IsIgnoringMe(fromAcc, accountUuid))
        return false;

    IO::DataClient* client = GetSubsystem<IO::DataClient>();
    AB::Entities::MailList ml;
    ml.uuid = accountUuid;
    // Check quota
    if (!client->Read(ml))
        return false;
    if (ml.mails.size() >= AB::Entities::Limits::MAX_MAIL_COUNT)
        return false;

    AB::Entities::Mail m;
    m.uuid = Utils::Uuid::New();
    m.fromAccountUuid = fromAcc;
    m.toAccountUuid = accountUuid;
    m.fromName = fromName;
    m.toName = toName;
    m.subject = subject;
    m.message = message;
    m.created = Utils::Tick();

    bool ret = client->Create(m);
    if (ret)
    {
        // Add to recipients mail list
        ml.mails.push_back({
            m.uuid,
            fromName,
            subject,
            m.created,
            false
            });
        client->Update(ml);

        // Notify receiver
        Net::MessageClient* msgCli = GetSubsystem<Net::MessageClient>();
        Net::MessageMsg msg;
        msg.type_ = Net::MessageType::NewMail;
        IO::PropWriteStream stream;
        stream.WriteString(accountUuid);
        msg.SetPropStream(stream);
        msgCli->Write(msg);
    }
    return ret;
}

bool ReadMail(AB::Entities::Mail& mail)
{
    IO::DataClient* client = GetSubsystem<IO::DataClient>();
    bool ret = client->Read(mail);
    if (ret)
    {
        mail.isRead = true;
        client->Update(mail);
        // Update read flag in mail list
        AB::Entities::MailList ml;
        ml.uuid = mail.toAccountUuid;
        if (client->Read(ml))
        {
            auto it = std::find_if(ml.mails.begin(), ml.mails.end(), [&mail](const AB::Entities::MailHeader& current)
            {
                return Utils::Uuid::IsEqual(current.uuid, mail.uuid);
            });
            if (it != ml.mails.end())
            {
                (*it).isRead = true;
                client->Update(ml);
            }
        }
    }
    return ret;
}

bool DeleteMail(AB::Entities::Mail& mail)
{
    IO::DataClient* client = GetSubsystem<IO::DataClient>();
    if (!client->Read(mail))
        return false;

    bool ret = client->Delete(mail);
    if (ret)
    {
        // Delete mail from mail list
        AB::Entities::MailList ml;
        ml.uuid = mail.toAccountUuid;
        if (client->Read(ml))
        {
            auto it = std::find_if(ml.mails.begin(), ml.mails.end(), [&mail](const AB::Entities::MailHeader& current)
            {
                return Utils::Uuid::IsEqual(current.uuid, mail.uuid);
            });
            if (it != ml.mails.end())
            {
                ml.mails.erase(it);
                client->Update(ml);
            }
        }
    }
    return ret;
}

}
}
