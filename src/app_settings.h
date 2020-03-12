//
// Created by flexf on 13.03.2020.
//

#ifndef FLASK4CPP_APP_SETTINGS_H
#define FLASK4CPP_APP_SETTINGS_H

#include <nonstd/variant.hpp>

#include <string>
#include <vector>

namespace flask4cpp
{

struct ListenerParams
{
    std::string name;
    std::string address;
    uint16_t port;
};

struct SSLListenerParams
{
    std::string name;
    std::string address;
    uint16_t port;
    std::string certPath;
    std::string keyPath;
};

using ListenerInfo = nonstd::variant<ListenerParams, SSLListenerParams>;

struct AppSettings
{
    std::vector<ListenerInfo> listeners;
};

} // namespace flask4cpp

#endif // FLASK4CPP_APP_SETTINGS_H
