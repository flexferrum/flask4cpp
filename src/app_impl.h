//
// Created by flexf on 12.03.2020.
//

#ifndef FLASK4CPP_APP_IMPL_H
#define FLASK4CPP_APP_IMPL_H

#include "app_settings.h"

#include <memory>

namespace flask4cpp
{
class AppImpl
{
public:
    explicit AppImpl(AppSettings* setts)
        : m_settings(setts)
    {
    }

    int Run();

private:
    std::unique_ptr<AppSettings> m_settings;
};
} // namespace flask4cpp

#endif // FLASK4CPP_APP_IMPL_H
