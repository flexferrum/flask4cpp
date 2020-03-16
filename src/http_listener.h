//
// Created by flexf on 16.03.2020.
//

#ifndef FLASK4CPP_HTTP_LISTENER_H
#define FLASK4CPP_HTTP_LISTENER_H

#include "http_listener_base.h"
#include "log.h"

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>

namespace flask4cpp
{
class HttpListener : public HttpListenerBase
{
public:
    using SocketType = boost::asio::ip::tcp;

    HttpListener(LoggerType* logger, boost::asio::io_context& ioc, const SocketType::endpoint& endpoint)
        : HttpListenerBase(logger)
        , m_acceptor(ioc)
        , m_socket(ioc)
    {
        boost::system::error_code ec;

        // Open the acceptor
        m_acceptor.open(endpoint.protocol(), ec);
        if (ec)
        {
            LOG_FATAL() << "Can't open protocol for endpoint '" << endpoint << "'";
            return;
        }

        // Allow address reuse
        m_acceptor.set_option(boost::asio::socket_base::reuse_address(true), ec);
        if (ec)
        {
            LOG_FATAL() << "Can't reuse address for endpoint '" << endpoint << "'";
            return;
        }

        // Bind to the server address
        m_acceptor.bind(endpoint, ec);
        if (ec)
        {
            LOG_FATAL() << "Can't bind to endpoint '" << endpoint << "'";
            return;
        }

        // Start listening for connections
        m_acceptor.listen(boost::asio::socket_base::max_listen_connections, ec);
        if (ec)
        {
            LOG_FATAL() << "Can't start listen on endpoint '" << endpoint << "'";
            return;
        }
    }

private:
    bool IsOpen() const override { return m_acceptor.is_open(); }

    void DoAccept() override {}

private:
    SocketType::acceptor m_acceptor;
    SocketType::socket m_socket;
};
} // namespace flask4cpp

#endif // FLASK4CPP_HTTP_LISTENER_H
