//
// Created by flexf on 12.03.2020.
//

#include "app_impl.h"

#include "http_listener.h"
#include "log.h"

namespace flask4cpp
{

namespace
{
class ListenerCreator
{
    LoggerType* m_logger;
    boost::asio::io_context* m_ioc;

public:
    ListenerCreator(LoggerType* logger, boost::asio::io_context* ioc)
        : m_logger(logger)
        , m_ioc(ioc)
    {
    }

    std::shared_ptr<HttpListenerBase> operator()(const ListenerParams& l) const
    {
        auto const address = boost::asio::ip::make_address(l.address);

        HttpListener::SocketType::endpoint endpoint{ address, l.port };
        return std::make_shared<HttpListener>(m_logger, *m_ioc, endpoint);
    }
    std::shared_ptr<HttpListenerBase> operator()(const SSLListenerParams& l) const { return std::shared_ptr<HttpListenerBase>{}; }
};

} // namespace

int AppImpl::Run()
{
    Start(false);

    return WaitForStop();
}
int AppImpl::WaitForStop()
{
    {
        std::unique_lock<std::mutex> l(m_syncMutex);
        m_syncCondVar.wait(l, [this] { return m_currentState == State::Stopped; });
    }
    return 0;
}

int AppImpl::Start(bool wait)
{
    m_currentState = State::Starting;
    m_ioContext = std::make_unique<boost::asio::io_context>(m_settings->threadNum);

    LOG_DEBUG() << "Start creating listeners";
    for (auto& l : m_settings->listeners)
    {
        LOG_DEBUG() << ">>>>>>>>> 1";
        auto listener = nonstd::visit(ListenerCreator(log(), m_ioContext.get()), l);
        LOG_DEBUG() << ">>>>>>>>> 2";
        if (!listener)
            continue;

        LOG_DEBUG() << ">>>>>>>>> 3";
        if (!listener->IsOpen())
            return -1;

        LOG_DEBUG() << ">>>>>>>>> 4";
        m_listeners.push_back(listener);
        LOG_DEBUG() << ">>>>>>>>> 5";
        listener->run();
        LOG_DEBUG() << ">>>>>>>>> 6";
    }
    LOG_DEBUG() << "End creating listeners. " << m_listeners.size() << " created";

    auto threads = m_settings->threadNum;
    m_ioThreads.reserve(threads);
    LOG_DEBUG() << "Start creating IO thread pool threads";

    std::atomic_int started_threads{ 0 };

    for (auto i = threads - 1; i >= 0; --i)
        m_ioThreads.emplace_back([this, &started_threads] {
            LOG_DEBUG() << "IO thread started";
            started_threads++;
            m_ioContext->run();
        });

    LOG_DEBUG() << "End creating IO thread pool threads. " << m_ioThreads.size() << " threads created";

    if (wait)
    {
        LOG_DEBUG() << "Wait for IO threads started";
        while (started_threads != threads)
            ;
        LOG_DEBUG() << "IO threads actually started";
    }

    m_currentState = State::Started;

    return 0;
}

int AppImpl::Shutdown()
{
    return 0;
}

} // flask4cpp
