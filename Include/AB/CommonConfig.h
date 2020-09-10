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

#include <sa/Compiler.h>

#if defined(SA_PLATFORM_WIN)
#   define AB_WINDOWS
#elif defined(SA_PLATFORM_LINUX) || defined(SA_PLATFORM_UNIX)
#   define AB_UNIX
#endif

#if !defined(AB_WINDOWS) && !defined(AB_UNIX)
#error Unsuppoprted platform
#endif

// Configurations shared by the server and client
#define AB_VERSION_CREATE(major, minor) (((major) << 8) | (minor))
#define AB_VERSION_GET_MAJOR(version) (((version) >> 8) & 0xFF)
#define AB_VERSION_GET_MINOR(version) ((version) & 0xFF)

#define CURRENT_YEAR 2020

// If Email is mandatory when creating an account uncomment bellow
//#define EMAIL_MANDATORY
#define CHARACTER_NAME_NIM   6
#define CHARACTER_NAME_MAX  20
#define ACCOUNT_NAME_MIN     6
#define ACCOUNT_NAME_MAX    32
#define PASSWORD_LENGTH_MIN  6
#define PASSWORD_LENGTH_MAX 61
#define EMAIL_LENGTH_MAX    60

// If defined disable nagle's algorithm, this make the game play smoother
// acceptor_.set_option(asio::ip::tcp::no_delay(true));
// But I think this makes some problems. Try again, watch out for sudden disconnects!
// Okay, let's use it, didn't see any negative effects lately.
#define TCP_OPTION_NODELAY

inline constexpr auto RESTRICTED_NAME_CHARS = R"(1234567890<>^!"$%&/()[]{}=?\`´,.-;:_+*~#'|)";

// As long as we dont have a trusted cert for the server, do not verify SSL
#define HTTP_CLIENT_VERIFY_SSL false

namespace Auth {
// Auth token expires in 1 hr of inactivity
inline constexpr long long AUTH_TOKEN_EXPIRES_IN = 1000 * 60 * 60;
// When a user deletes a character, the characters name is reserved for 1 week
inline constexpr int NAME_RESERVATION_EXPIRES_MS = 1000 * 60 * 60 * 24 * 7;
}
