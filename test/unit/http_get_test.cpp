//
// Created by Flex Ferrum on 25.03.2020.
//

#include <flask4cpp/app.h>
#include <flask4cpp/app_builder.h>
#include <gtest/gtest.h>

#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>

namespace beast = boost::beast; // from <boost/beast.hpp>
namespace http = beast::http;   // from <boost/beast/http.hpp>
namespace net = boost::asio;    // from <boost/asio.hpp>
using tcp = net::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

class HttpRequestFixture : public testing::Test
{
public:
protected:
    nonstd::expected<std::string, beast::system_error> GetPage(const std::string& host, const std::string& port, const std::string& target, int version)
    {
        try
        {
            std::cout << ">>>>> 1" << std::endl;
            // The io_context is required for all I/O
            // Подключение не установлено, т.к. конечный компьютер отверг запрос на подключение
            net::io_context ioc;

            // These objects perform our I/O
            std::cout << ">>>>> 2" << std::endl;
            tcp::resolver resolver(ioc);
            std::cout << ">>>>> 3" << std::endl;
            beast::tcp_stream stream(ioc);

            // Look up the domain name
            std::cout << ">>>>> 4" << std::endl;
            auto const results = resolver.resolve(host, port);

            // Make the connection on the IP address we get from a lookup
            std::cout << ">>>>> 5" << std::endl;
            stream.connect(results);

            // Set up an HTTP GET request message
            std::cout << ">>>>> 6" << std::endl;
            http::request<http::string_body> req{ http::verb::get, target, 11 };
            req.set(http::field::host, host);
            req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

            // Send the HTTP request to the remote host
            std::cout << ">>>>> 7" << std::endl;
            http::write(stream, req);

            // This buffer is used for reading and must be persisted
            beast::flat_buffer buffer;

            // Declare a container to hold the response
            http::response<http::dynamic_body> res;

            // Receive the HTTP response
            std::cout << ">>>>> 8" << std::endl;
            http::read(stream, buffer, res);

            // Write the message to standard out
            std::cout << ">>>>> 9" << std::endl;
            std::cout << res << std::endl;

            std::string result;

            // Gracefully close the socket
            beast::error_code ec;
            std::cout << ">>>>> 10" << std::endl;
            stream.socket().shutdown(tcp::socket::shutdown_both, ec);

            if (!ec)
            {
                nonstd::make_unexpected(beast::system_error{ ec });
            }
            else
            {
                std::ostringstream os;
                os << res;
                result = os.str();
            }

            return result;
        }
        catch (beast::system_error const& e)
        {
            return nonstd::make_unexpected(e);
        }
        catch (std::exception const& e)
        {
            std::cerr << "Error: " << e.what() << std::endl;
            throw;
        }
    }
};

TEST_F(HttpRequestFixture, Http11GetDefault_Successfull)
{
    flask4cpp::AppBuilder builder;

    builder.ConfigureDefault();
    // Create HTTP server on 0.0.0.0:80
    auto app = builder.CreateApp();
    EXPECT_TRUE(app.has_value());

    try
    {
        std::cout << "Before HTTP server start" << std::endl;
        app->Start(true);
        std::cout << "After HTTP server start" << std::endl;

        auto result = GetPage("127.0.0.1", "80", "/", 11);

        EXPECT_FALSE(!result);

        if (!result)
        {
            auto& err = result.error();
            std::cerr << "Page get error. Code: " << err.code() << ", message: " << result.error().what() << std::endl;
        }
        else
        {
            std::cout << result.value() << std::endl;
        }

        std::cout << "Before HTTP server shutdown" << std::endl;
        app->Shutdown();
        std::cout << "After HTTP server shutdown" << std::endl;
    }
    catch (std::exception const& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        EXPECT_TRUE(false);
    }
}
