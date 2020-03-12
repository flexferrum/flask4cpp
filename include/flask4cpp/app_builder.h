//
// Created by flexf on 13.03.2020.
//

#ifndef FLASK4CPP_APP_BUILDER_H
#define FLASK4CPP_APP_BUILDER_H

#include "app.h"

#include <nonstd/expected.hpp>
#include <nonstd/string_view.hpp>

#include <memory>

namespace flask4cpp
{
struct AppSettings;

class AppBuilder
{
public:
    AppBuilder();
    ~AppBuilder();

    enum class BuildErrors {
        NotInitialized,
        ConfigurationError,
        NoSSLSupport,
    };

    AppBuilder& ConfigureDefault();
    AppBuilder& ListenOn(const nonstd::string_view& name, const nonstd::string_view& address, uint16_t port);
    AppBuilder& ListenOnSsl(const nonstd::string_view& name,
                            const nonstd::string_view& address,
                            uint16_t port,
                            const nonstd::string_view& certPath,
                            const nonstd::string_view& keyPath);

    nonstd::expected<App, BuildErrors> CreateApp();

private:
    void EnsureSettingsCreated();

private:
    std::unique_ptr<AppSettings> m_settings;
};
} // namespace flask4cpp

#endif // FLASK4CPP_APP_BUILDER_H
