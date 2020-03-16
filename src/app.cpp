//
// Created by flexf on 12.03.2020.
//

#include "app_impl.h"

#include <flask4cpp/app.h>

namespace flask4cpp
{

App::App(AppSettings* settings)
    : m_impl(new AppImpl(settings))
{
}
App::~App() = default;

int App::Run()
{
    return m_impl->Run();
}

int App::Start(bool wait)
{
    return m_impl->Start(wait);
}
int App::Shutdown()
{
    return m_impl->Shutdown();
}

} // flask4cpp
