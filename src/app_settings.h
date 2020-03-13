//
// Created by flexf on 13.03.2020.
//

#ifndef FLASK4CPP_APP_SETTINGS_H
#define FLASK4CPP_APP_SETTINGS_H

#include <nonstd/variant.hpp>

#if defined(BOOST_LOG_NO_THREADS)
#undef BOOST_LOG_NO_THREADS
#endif

#include <boost/log/common.hpp>
#include <boost/log/trivial.hpp>
// #include <boost/log/sources/severity_logger.hpp>
#include <boost/log/sources/logger.hpp>

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

struct LogSettings
{
    boost::log::trivial::severity_level severity = boost::log::trivial::info;
    bool enableConsole = false;
};

struct AppSettings
{
    using LoggerType = boost::log::sources::severity_logger_mt<boost::log::trivial::severity_level>;

    std::vector<ListenerInfo> listeners;
    LogSettings logSettings;
    std::unique_ptr<LoggerType> logger;
};

} // namespace flask4cpp

#endif // FLASK4CPP_APP_SETTINGS_H
