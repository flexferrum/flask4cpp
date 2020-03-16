//
// Created by flexf on 16.03.2020.
//

#ifndef FLASK4CPP_HTTP_LISTENER_BASE_H
#define FLASK4CPP_HTTP_LISTENER_BASE_H

#include "log.h"

namespace flask4cpp
{
class HttpListenerBase
{
public:
    HttpListenerBase(LoggerType* logger)
        : m_logger(logger)
    {
    }
    virtual ~HttpListenerBase() {}

    virtual void run()
    {
        // Start accepting incoming connections
        if (!IsOpen())
            return;
        DoAccept();
    }

    virtual bool IsOpen() const = 0;

protected:
    auto log() { return m_logger; };

    virtual void DoAccept() = 0;

private:
    LoggerType* m_logger = nullptr;
};
} // namespace flask4cpp

#endif // FLASK4CPP_HTTP_LISTENER_BASE_H
