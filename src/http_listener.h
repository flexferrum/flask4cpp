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
        , m_strand(m_socket.get_executor())
    {
        boost::system::error_code ec;

        // Open the acceptor
        m_acceptor.open(endpoint.protocol(), ec);
        if (ec)
        {
            LOG_FATAL() << "Can't open protocol for endpoint '" << endpoint << "'";
            return;
        }
        LOG_DEBUG() << "Acceptor for endpoint '" << endpoint << "' successfully opened";

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
        LOG_DEBUG() << "Endpoint '" << endpoint << "' successfully bound";

        // Start listening for connections
        m_acceptor.listen(boost::asio::socket_base::max_listen_connections, ec);
        if (ec)
        {
            LOG_FATAL() << "Can't start listen on endpoint '" << endpoint << "'";
            return;
        }
        LOG_DEBUG() << "Start HTTP listening on endpoint '" << endpoint << "'";
    }

private:
    bool IsOpen() const override { return m_acceptor.is_open(); }

    void DoAccept() override
    {
        LOG_DEBUG() << "Invoking async_accept";
        m_acceptor.async_accept(m_socket, [object = shared_from_this(), this](boost::system::error_code ec) {
            LOG_DEBUG() << "New connection accepted with error code: " << ec;
            HandleRequest();
            DoAccept();
        });
    }

    void HandleRequest()
    {
        // std::bind(&HttpSession::OnRead, shared_from_this(), std::placeholders::_1, std::placeholders::_2)
        boost::beast::http::async_read(
          m_socket, m_buffer, m_req, boost::asio::bind_executor(m_strand, [this, object = shared_from_this()](boost::system::error_code ec, std::size_t size) {
              ProcessRequest(ec, size);
          }));
    }

    void ProcessRequest(boost::system::error_code ec, std::size_t size)
    {
        namespace http = boost::beast::http;
        http::response<http::string_body> res{ http::status::bad_request, m_req.version() };
        res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(http::field::content_type, "text/html");
        res.keep_alive(false);
        res.body() = "Hello World!";
        res.prepare_payload();

        auto sp = std::make_shared<std::decay_t<decltype(res)>>(std::move(res));

        http::async_write(
          m_socket,
          *sp,
          boost::asio::bind_executor(m_strand, std::bind(&HttpListener::OnWrite, this, std::placeholders::_1, std::placeholders::_2, sp->need_eof())));
    }

    void OnWrite(boost::system::error_code, std::size_t, bool) {}

    //    void OnAccept(boost::system::error_code ec)
    //    {
    //        LOG_DEBUG() << "New connection accepted with error code: " << ec;
    //    }

private:
    SocketType::acceptor m_acceptor;
    SocketType::socket m_socket;
    boost::asio::strand<boost::asio::executor> m_strand;
    boost::beast::flat_buffer m_buffer;
    boost::beast::http::request<boost::beast::http::string_body> m_req;
};
} // namespace flask4cpp

#endif // FLASK4CPP_HTTP_LISTENER_H
