//
// Created by flexf on 12.03.2020.
//

#ifndef FLASK4CPP_APP_H
#define FLASK4CPP_APP_H

#include <memory>

namespace flask4cpp
{
class AppImpl;
struct AppSettings;

class App
{
public:
    ~App();

    int Run();
    int Start(bool wait = false);
    int Shutdown();

private:
    friend class AppBuilder;

    explicit App(AppSettings* settings);
    std::shared_ptr<AppImpl> m_impl;
};
} // namespace flask4cpp

#endif // FLASK4CPP_APP_H
