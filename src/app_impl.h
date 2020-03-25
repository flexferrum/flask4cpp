//
// Created by flexf on 12.03.2020.
//

#ifndef FLASK4CPP_APP_IMPL_H
#define FLASK4CPP_APP_IMPL_H

#include "app_settings.h"

#include <boost/asio/bind_executor.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/strand.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>

#include <atomic>
#include <condition_variable>
#include <memory>
#include <thread>
#include <mutex>

namespace flask4cpp
{
class HttpListenerBase;
class HttpListener;
class HttpsListener;

class AppImpl
{
public:
    explicit AppImpl(AppSettings* setts)
        : m_settings(setts)
        , m_currentState(State::Initialized)
    {
    }

    int Run();
    int Start(bool wait);
    int Shutdown();

private:
    int WaitForStop();
    auto log() { return m_settings->logger.get(); };

private:
    friend class ListenersVisitor;

    enum class State
    {
        Initialized,
        Starting,
        Started,
        Stopping,
        Stopped
    };

    std::unique_ptr<AppSettings> m_settings;
    std::mutex m_syncMutex;
    std::condition_variable m_syncCondVar;
    std::atomic<State> m_currentState;
    std::unique_ptr<boost::asio::io_context> m_ioContext;
    std::vector<std::shared_ptr<HttpListenerBase>> m_listeners;
    std::vector<std::thread> m_ioThreads;
};
} // namespace flask4cpp

#endif // FLASK4CPP_APP_IMPL_H
