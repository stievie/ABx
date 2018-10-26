#pragma once

class Cookie
{
public:
    Cookie();
    Cookie(const std::string& content);
    ~Cookie() = default;

    std::string domain_;
    std::string path_;
    time_t expires_;
    bool httpOnly_;
    std::string content_;
};

class Cookies
{
private:
    std::map<std::string, Cookie> cookies_;
public:
    Cookies() = default;
    explicit Cookies(const HttpsServer::Request& request);
    void Write(SimpleWeb::CaseInsensitiveMultimap& header);
    void Add(const std::string& name, const Cookie& cookie);
    Cookie* Get(const std::string& name);
};
