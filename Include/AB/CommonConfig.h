#pragma once

#if defined(_WIN32)
#   define AB_WINDOWS
#elif defined(__unix__) || defined(__unix) || (defined(__APPLE__) && defined(__MACH__))
#   define AB_UNIX
#endif

#if !defined(AB_WINDOWS) && !defined(AB_UNIX)
#error Unsuppoprted platform
#endif

#if defined(_M_X64) || defined(__amd64__) || defined(__x86_64) || defined(__x86_64__)
#define AB_ARCH_64BIT
#else
#define AB_ARCH_32BIT
#endif

// Configurations shared by the server and client

#define CURRENT_YEAR 2019

// If Email is mandatory when creating an account uncomment bellow
//#define EMAIL_MANDATORY

// If defined disable nagle's algorithm, this make the game play smoother
// acceptor_.set_option(asio::ip::tcp::no_delay(true));
// But I think this makes some problems. Try again, watch out for sudden disconnects!
// Okay, let's use it, didn't see any negative effects lately.
#define TCP_OPTION_NODELAY

constexpr auto RESTRICTED_NAME_CHARS = R"(<>^!"$%&/()[]{}=?\`´,.-;:_+*~#'|)";

namespace Game {
constexpr int PLAYER_MAX_SKILLS = 8;
// Most profession have 4 attribute but Warrior and Elementarist have 5
constexpr int PLAYER_MAX_ATTRIBUTES = 10;

// For client prediction these values are also needed by the client.
constexpr float BASE_MOVE_SPEED = 150.0f;
constexpr float BASE_TURN_SPEED = 2000.0f;
}

namespace Auth {
// Auth token expires in 1 hr of inactivity
constexpr long long AUTH_TOKEN_EXPIRES_IN = 1000 * 60 * 60;
}
