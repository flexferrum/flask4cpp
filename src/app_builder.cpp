//
// Created by flexf on 13.03.2020.
//

#include "app_settings.h"
#include "log.h"

#include <flask4cpp/app_builder.h>
#include <fmt/format.h>

#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/filter_parser.hpp>

namespace flask4cpp
{
AppBuilder::AppBuilder() = default;
AppBuilder::~AppBuilder() = default;

AppBuilder& AppBuilder::ConfigureDefault()
{
    EnsureSettingsCreated();

    ListenerParams params = {};
    params.name = "default";
    params.address = "0.0.0.0";
    params.port = 80;

    m_settings->listeners.emplace_back(std::move(params));

    return *this;
}

AppBuilder& AppBuilder::ListenOn(const nonstd::string_view& name, const nonstd::string_view& address, uint16_t port)
{
    EnsureSettingsCreated();

    ListenerParams params = {};
    params.name.assign(name.begin(), name.end());
    params.address.assign(address.begin(), address.end());
    params.port = port;

    m_settings->listeners.emplace_back(std::move(params));

    return *this;
}

AppBuilder& AppBuilder::ListenOnSsl(const nonstd::string_view& name,
                                    const nonstd::string_view& address,
                                    uint16_t port,
                                    const nonstd::string_view& certPath,
                                    const nonstd::string_view& keyPath)
{
    EnsureSettingsCreated();

    SSLListenerParams params = {};
    params.name.assign(name.begin(), name.end());
    params.address.assign(address.begin(), address.end());
    params.port = port;
    params.certPath.assign(certPath.begin(), certPath.end());
    params.keyPath.assign(keyPath.begin(), keyPath.end());

    m_settings->listeners.emplace_back(std::move(params));

    return *this;
}

AppBuilder& AppBuilder::SetLogLevel(AppBuilder::LogLevel level)
{
    EnsureSettingsCreated();
    auto& logSetts = m_settings->logSettings;
    using namespace boost::log;

    switch (level)
    {
        case LogLevel::Trace:
            logSetts.severity = trivial::trace;
            break;
        case LogLevel::Debug:
            logSetts.severity = trivial::debug;
            break;
        case LogLevel::Info:
            logSetts.severity = trivial::info;
            break;
        case LogLevel::Warning:
            logSetts.severity = trivial::warning;
            break;
        case LogLevel::Error:
            logSetts.severity = trivial::error;
            break;
        case LogLevel::Fatal:
            logSetts.severity = trivial::fatal;
            break;
    }
    return *this;
}

namespace
{
auto InitializeLogger(const LogSettings& setts)
{
    auto log = std::make_unique<AppSettings::LoggerType>();
    if (!setts.enableConsole)
    {
        auto console = boost::log::add_console_log();
        console->set_filter(boost::log::expressions::attr<int>("Severity") >= 10);
    }

    return log;
}

struct EndpointAddressBuilder
{
    std::string operator()(const ListenerParams& l) const { return fmt::format("http://{}:{}", l.address, l.port); }
    std::string operator()(const SSLListenerParams& l) const { return fmt::format("https://{}:{}", l.address, l.port); }
};
} // namespace

nonstd::expected<App, AppBuilder::BuildErrors> AppBuilder::CreateApp()
{
    if (!m_settings)
        return nonstd::make_unexpected(BuildErrors::NotInitialized);

    m_settings->logger = std::move(InitializeLogger(m_settings->logSettings));

    auto log = [this] { return m_settings->logger.get(); };
    LOG_DEBUG() << "Start verifying configuration";
    for (auto& l : m_settings->listeners)
    {
        auto endpoint = nonstd::visit(EndpointAddressBuilder(), l);
        LOG_DEBUG() << "Found endpoint: " << endpoint;
    }
    return App(m_settings.release());
}

void AppBuilder::EnsureSettingsCreated()
{
    if (!m_settings)
    {
        m_settings = std::make_unique<AppSettings>();
#ifndef NDEBUG
        m_settings->logSettings.severity = boost::log::trivial::debug;
        m_settings->logSettings.enableConsole = true;
#endif
    }
}

} // namespace flask4cpp
