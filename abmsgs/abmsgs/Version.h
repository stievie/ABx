#pragma once

#define SERVER_VERSION_MAJOR 0
#define SERVER_VERSION_MINOR 1
#define SERVER_YEAR 2018
#define SERVER_PRODUCT_NAME "Forgotten Wars Message Server"

#define AB_VERSION_CREATE(major, minor) (((major) << 8) | (minor))
#define AB_VERSION_GET_MAJOR(version) (((version) >> 8) & 0xFF)
#define AB_VERSION_GET_MINOR(version) ((version) & 0xFF)
#define AB_SERVER_VERSION AB_VERSION_CREATE(SERVER_VERSION_MAJOR, SERVER_VERSION_MINOR)

