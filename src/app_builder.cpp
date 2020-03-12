//
// Created by flexf on 13.03.2020.
//

#include "app_settings.h"

#include <flask4cpp/app_builder.h>

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

nonstd::expected<App, AppBuilder::BuildErrors> AppBuilder::CreateApp()
{
    if (!m_settings)
        return nonstd::make_unexpected(BuildErrors::NotInitialized);

    return App(m_settings.release());
}

void AppBuilder::EnsureSettingsCreated()
{
    if (!m_settings)
        m_settings = std::make_unique<AppSettings>();
}

} // namespace flask4cpp
