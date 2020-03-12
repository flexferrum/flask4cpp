//
// Copyright (c) 2016-2017 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/beast
//

#include <boost/asio/bind_executor.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/strand.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/config.hpp>
#include <jinja2cpp/binding/nlohmann_json.h>
#include <jinja2cpp/filesystem_handler.h>
#include <jinja2cpp/generic_list_iterator.h>
#include <jinja2cpp/template.h>
#include <jinja2cpp/template_env.h>
#include <jinja2cpp/user_callable.h>
#include <nlohmann/json.hpp>

#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>

using tcp = boost::asio::ip::tcp;    // from <boost/asio/ip/tcp.hpp>
namespace http = boost::beast::http; // from <boost/beast/http.hpp>

namespace jinja2
{
template<class Body, class Allocator>
struct TypeReflection<http::request<Body, http::basic_fields<Allocator>>> : TypeReflected<http::request<Body, http::basic_fields<Allocator>>>
{
    using FieldAccessor = typename TypeReflected<http::request<Body, http::basic_fields<Allocator>>>::FieldAccessor;
    using ReqT = http::request<Body, http::basic_fields<Allocator>>;
    static auto& GetAccessors()
    {
        static std::unordered_map<std::string, FieldAccessor> accessors = {
            { "path",
              [](const ReqT& obj) {
                  auto val = obj.target();
                  return nonstd::string_view(val.begin(), val.size());
              } },
        };

        return accessors;
    }
};
}

auto SplitFileName(boost::beast::string_view path)
{
    auto const pos = path.rfind(".");
    if (pos == boost::beast::string_view::npos)
        return std::make_pair(path, boost::beast::string_view{});
    return std::make_pair(path.substr(0, pos), path.substr(pos + 1));
}

// Return a reasonable mime type based on the extension of a file.
boost::beast::string_view GetMimeType(boost::beast::string_view ext)
{
    using boost::beast::iequals;
    if (iequals(ext, "htm"))
        return "text/html";
    if (iequals(ext, "html"))
        return "text/html";
    if (iequals(ext, "j2html"))
        return "text/html";
    if (iequals(ext, "php"))
        return "text/html";
    if (iequals(ext, "css"))
        return "text/css";
    if (iequals(ext, "txt"))
        return "text/plain";
    if (iequals(ext, "js"))
        return "application/javascript";
    if (iequals(ext, "json"))
        return "application/json";
    if (iequals(ext, "xml"))
        return "application/xml";
    if (iequals(ext, "swf"))
        return "application/x-shockwave-flash";
    if (iequals(ext, "flv"))
        return "video/x-flv";
    if (iequals(ext, "png"))
        return "image/png";
    if (iequals(ext, "jpe"))
        return "image/jpeg";
    if (iequals(ext, "jpeg"))
        return "image/jpeg";
    if (iequals(ext, "jpg"))
        return "image/jpeg";
    if (iequals(ext, "gif"))
        return "image/gif";
    if (iequals(ext, "bmp"))
        return "image/bmp";
    if (iequals(ext, "ico"))
        return "image/vnd.microsoft.icon";
    if (iequals(ext, "tiff"))
        return "image/tiff";
    if (iequals(ext, "tif"))
        return "image/tiff";
    if (iequals(ext, "svg"))
        return "image/svg+xml";
    if (iequals(ext, "svgz"))
        return "image/svg+xml";
    return "application/text";
}

// Append an HTTP rel-path to a local filesystem path.
// The returned path is normalized for the platform.
std::string PathCat(boost::beast::string_view base, boost::beast::string_view path)
{
    if (base.empty())
        return path.to_string();
    std::string result = base.to_string();
#if BOOST_MSVC
    char constexpr pathSeparator = '\\';
    if (result.back() == pathSeparator)
        result.resize(result.size() - 1);
    result.append(path.data(), path.size());
    for (auto& c : result)
        if (c == '/')
            c = pathSeparator;
#else
    char constexpr path_separator = '/';
    if (result.back() == path_separator)
        result.resize(result.size() - 1);
    result.append(path.data(), path.size());
#endif
    return result;
}

template<typename Path>
std::string AppendDefaultPage(const Path& path)
{
    std::string requestPath(path.begin(), path.end());
    if (requestPath.back() == '/')
        requestPath += "index.html";

    return requestPath;
}

jinja2::Value FindPage(const jinja2::GenericList& pages, const nonstd::string_view& current, const std::string& root, jinja2::ValuesList* breadcrumb)
{
    for (auto& pageVal : pages)
    {
        const jinja2::GenericMap* page = nonstd::get_if<jinja2::GenericMap>(&pageVal.data());
        if (!page)
            continue;
        auto fileNameVal = page->GetValueByName("name");
        if (fileNameVal.isEmpty())
            continue;
        auto fileName = jinja2::AsString(fileNameVal);
        auto path = root + fileName;
        if (path == current)
        {
            if (breadcrumb)
                breadcrumb->push_back(pageVal);
            return pageVal;
        }

        auto pagesVal = page->GetValueByName("pages");
        const jinja2::GenericList* subPages = nonstd::get_if<jinja2::GenericList>(&pagesVal.data());
        if (subPages != 0)
        {
            auto result = FindPage(*subPages, current, path + "/", breadcrumb);
            if (!result.isEmpty())
            {
                if (breadcrumb)
                    breadcrumb->insert(breadcrumb->begin(), pageVal);
                return result;
            }
        }
    }

    return jinja2::Value();
}

jinja2::Value GetPageInfo(const jinja2::GenericList& pages, const nonstd::string_view& current)
{
    jinja2::ValuesList breadcrumb;

    auto page = FindPage(pages, current, "/", &breadcrumb);

    return jinja2::ValuesMap{ { "page", std::move(page) }, { "path", std::move(breadcrumb) } };
}

// This function produces an HTTP response for the given
// request. The type of the response object depends on the
// contents of the request, so the interface requires the
// caller to pass a generic lambda for receiving the response.
template<class Body, class Allocator, class Send>
void HandleRequest(jinja2::TemplateEnv* env, const std::string& docRoot, http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send)
{
    // Returns a bad request response
    auto const badRequest = [&req](boost::beast::string_view why) {
        http::response<http::string_body> res{ http::status::bad_request, req.version() };
        res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(http::field::content_type, "text/html");
        res.keep_alive(req.keep_alive());
        res.body() = why.to_string();
        res.prepare_payload();
        return res;
    };

    // Returns a not found response
    auto const notFound = [&req](boost::beast::string_view target) {
        http::response<http::string_body> res{ http::status::not_found, req.version() };
        res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(http::field::content_type, "text/html");
        res.keep_alive(req.keep_alive());
        res.body() = "The resource '" + target.to_string() + "' was not found.";
        res.prepare_payload();
        return res;
    };

    // Returns a server error response
    auto const serverError = [&req](boost::beast::string_view what) {
        http::response<http::string_body> res{ http::status::internal_server_error, req.version() };
        res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(http::field::content_type, "text/html");
        res.keep_alive(req.keep_alive());
        res.body() = "An error occurred: '" + what.to_string() + "'";
        res.prepare_payload();
        return res;
    };

    auto const sendTemplate = [&req, &serverError](jinja2::Template& tpl, auto& ext, auto& send) {
        jinja2::ValuesMap params{ { "request", jinja2::Reflect(req) } };

        auto renderResult = tpl.RenderAsString(params);
        if (!renderResult)
        {
            std::ostringstream os;
            os << renderResult.error();
            return send(serverError(os.str()));
        }

        auto& body = renderResult.value();
        auto size = body.size();

        if (req.method() == http::verb::head)
        {
            http::response<http::empty_body> res{ http::status::ok, req.version() };
            res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
            res.set(http::field::content_type, GetMimeType(ext));
            res.content_length(size);
            res.keep_alive(req.keep_alive());
            return send(std::move(res));
        }

        // Respond to GET request
        http::response<http::string_body> res{ std::piecewise_construct, std::make_tuple(std::move(body)), std::make_tuple(http::status::ok, req.version()) };
        res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(http::field::content_type, GetMimeType(ext));
        res.content_length(size);
        res.keep_alive(req.keep_alive());
        return send(std::move(res));
    };

    // Make sure we can handle the method
    if (req.method() != http::verb::get && req.method() != http::verb::head)
        return send(badRequest("Unknown HTTP-method"));

    // Request path must be absolute and not contain "..".
    if (req.target().empty() || req.target()[0] != '/' || req.target().find("..") != boost::beast::string_view::npos)
        return send(badRequest("Illegal request-target"));

    // Build the path to the requested file
    auto requestPath = AppendDefaultPage(req.target());

    std::cout << "---- Requested resource: " << requestPath << std::endl;

    auto fileInfo = SplitFileName(requestPath);
    auto& fileName = fileInfo.first;
    auto& fileExt = fileInfo.second;

    std::cout << "---- Requested resource/file name: " << fileName << std::endl;
    std::cout << "---- Requested resource/file ext: " << fileExt << std::endl;

    std::string templateName(fileName.begin(), fileName.end());
    templateName += ".j2";
    templateName.append(fileExt.begin(), fileExt.end());

    auto loadResult = env->LoadTemplate(templateName);

    if (loadResult)
    {
        return sendTemplate(loadResult.value(), fileExt, send);
    }
    else
    {
        auto& error = loadResult.error();
        if (error.GetCode() != jinja2::ErrorCode::FileNotFound)
        {
            std::ostringstream os;
            os << loadResult.error();
            return send(serverError(os.str()));
        }
    }

    std::string path = PathCat(docRoot, requestPath);

    // Attempt to open the file
    boost::beast::error_code ec;
    http::file_body::value_type body;
    body.open(path.c_str(), boost::beast::file_mode::scan, ec);

    // Handle the case where the file doesn't exist
    if (ec == boost::system::errc::no_such_file_or_directory)
        return send(notFound(req.target()));

    // Handle an unknown error
    if (ec)
        return send(serverError(ec.message()));

    // Cache the size since we need it after the move
    auto const size = body.size();

    // Respond to HEAD request
    if (req.method() == http::verb::head)
    {
        http::response<http::empty_body> res{ http::status::ok, req.version() };
        res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(http::field::content_type, GetMimeType(fileExt));
        res.content_length(size);
        res.keep_alive(req.keep_alive());
        return send(std::move(res));
    }

    // Respond to GET request
    http::response<http::file_body> res{ std::piecewise_construct, std::make_tuple(std::move(body)), std::make_tuple(http::status::ok, req.version()) };
    res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(http::field::content_type, GetMimeType(fileExt));
    res.content_length(size);
    res.keep_alive(req.keep_alive());
    return send(std::move(res));
}

//------------------------------------------------------------------------------

// Report a failure
void Fail(boost::system::error_code ec, char const* what)
{
    std::cerr << what << ": " << ec.message() << "\n";
}

// Handles an HTTP server connection
class HttpSession : public std::enable_shared_from_this<HttpSession>
{

    tcp::socket m_socket;
    boost::asio::strand<boost::asio::executor> m_strand;
    boost::beast::flat_buffer m_buffer;
    std::shared_ptr<std::string const> doc_root_;
    http::request<http::string_body> m_req;
    std::shared_ptr<void> m_res;
    jinja2::TemplateEnv* m_env;
    const std::string* m_docRoot;
    // send_lambda lambda_;

public:
    // Take ownership of the socket
    HttpSession(tcp::socket socket, jinja2::TemplateEnv* env, const std::string* docRoot)
        : m_socket(std::move(socket))
        , m_strand(m_socket.get_executor())
        , m_env(env)
        , m_docRoot(docRoot)
    {
    }

    // Start the asynchronous operation
    void run() { DoRead(); }

    void DoRead()
    {
        // Make the request empty before reading,
        // otherwise the operation behavior is undefined.
        m_req = {};

        // Read a request
        http::async_read(m_socket, m_buffer,m_req,
          boost::asio::bind_executor(m_strand, std::bind(&HttpSession::OnRead, shared_from_this(), std::placeholders::_1, std::placeholders::_2)));
    }

    void OnRead(boost::system::error_code ec, std::size_t /*bytes_transferred*/)
    {
        // This means they closed the connection
        if (ec == http::error::end_of_stream)
            return DoClose();

        if (ec)
            return Fail(ec, "read");

        // Send the response
        HandleRequest(m_env, *m_docRoot, std::move(m_req), [this](auto&& msg)
        {
            // The lifetime of the message has to extend
            // for the duration of the async operation so
            // we use a shared_ptr to manage it.
            auto sp = std::make_shared<std::decay_t<decltype(msg)>>(std::move(msg));

            // Store a type-erased version of the shared
            // pointer in the class to keep it alive.
            m_res = sp;

            // Write the response
            http::async_write(m_socket, *sp,
              boost::asio::bind_executor(
                m_strand,
                std::bind(&HttpSession::OnWrite,
                  shared_from_this(),
                  std::placeholders::_1,
                  std::placeholders::_2,
                  sp->need_eof())));
        });
    }

    void OnWrite(boost::system::error_code ec, std::size_t bytes_transferred, bool close)
    {
        boost::ignore_unused(bytes_transferred);

        if (ec)
            return Fail(ec, "write");

        if (close)
        {
            // This means we should close the connection, usually because
            // the response indicated the "Connection: close" semantic.
            return DoClose();
        }

        // We're done with the response so delete it
        m_res = nullptr;

        // Read another request
        DoRead();
    }

    void DoClose()
    {
        // Send a TCP shutdown
        boost::system::error_code ec;
        m_socket.shutdown(tcp::socket::shutdown_send, ec);

        // At this point the connection is closed gracefully
    }
};

//------------------------------------------------------------------------------

// Accepts incoming connections and launches the sessions
class HttpListener : public std::enable_shared_from_this<HttpListener>
{
    tcp::acceptor m_acceptor;
    tcp::socket m_socket;
    jinja2::TemplateEnv* m_env;
    const std::string* m_docRoot;

public:
    HttpListener(boost::asio::io_context& ioc, const tcp::endpoint& endpoint, jinja2::TemplateEnv& env, const std::string& docRoot)
        : m_acceptor(ioc)
        , m_socket(ioc)
        , m_env(&env)
        , m_docRoot(&docRoot)
    {
        boost::system::error_code ec;

        // Open the acceptor
        m_acceptor.open(endpoint.protocol(), ec);
        if (ec)
        {
            Fail(ec, "open");
            return;
        }

        // Allow address reuse
        m_acceptor.set_option(boost::asio::socket_base::reuse_address(true), ec);
        if (ec)
        {
            Fail(ec, "set_option");
            return;
        }

        // Bind to the server address
        m_acceptor.bind(endpoint, ec);
        if (ec)
        {
            Fail(ec, "bind");
            return;
        }

        // Start listening for connections
        m_acceptor.listen(boost::asio::socket_base::max_listen_connections, ec);
        if (ec)
        {
            Fail(ec, "listen");
            return;
        }
    }

    // Start accepting incoming connections
    void run()
    {
        if (!m_acceptor.is_open())
            return;
        DoAccept();
    }

    void DoAccept() { m_acceptor.async_accept(m_socket, std::bind(&HttpListener::OnAccept, shared_from_this(), std::placeholders::_1)); }

    void OnAccept(boost::system::error_code ec)
    {
        if (ec)
        {
            Fail(ec, "accept");
        }
        else
        {
            // Create the session and run it
            std::make_shared<HttpSession>(std::move(m_socket), m_env, m_docRoot)->run();
        }

        // Accept another connection
        DoAccept();
    }
};

//------------------------------------------------------------------------------

int main(int argc, char* argv[])
{
    // Check command line arguments.
    if (argc != 5)
    {
        std::cerr << "Usage: beast-jinja2cpp <address> <port> <doc_root> <threads>\n"
                  << "Example:\n"
                  << "    beast-jinja2cpp 0.0.0.0 8080 . 1\n";
        return EXIT_FAILURE;
    }
    auto const address = boost::asio::ip::make_address(argv[1]);
    auto const port = static_cast<unsigned short>(std::atoi(argv[2]));
    auto const docRoot = std::string(argv[3]);
    auto const threads = std::max<int>(1, std::atoi(argv[4]));

    jinja2::RealFileSystem fs(docRoot);
    jinja2::TemplateEnv env;

    auto& setts = env.GetSettings();
    setts.extensions.Do = true;
    setts.lstripBlocks = true;
    setts.trimBlocks = true;

    auto jsonPath = PathCat(docRoot, "/.site_info.json");
    std::cout << "---- Read site info from " << jsonPath << std::endl;

    nlohmann::json siteInfo;
    std::ifstream jsonStream(jsonPath);

    jsonStream >> siteInfo;
    env.AddGlobal("site", jinja2::Reflect(std::move(siteInfo)));
    env.AddGlobal("__GetPageInfo", jinja2::MakeCallable(GetPageInfo, jinja2::ArgInfo{ "pages" }, jinja2::ArgInfo{ "path" }));
    if (port != 80 && port != 443)
        env.AddGlobal("__site_port", static_cast<int64_t>(port));

    env.AddFilesystemHandler(std::string(), fs);

    // The io_context is required for all I/O
    boost::asio::io_context ioc{ threads };

    // Create and launch a listening port
    std::make_shared<HttpListener>(ioc, tcp::endpoint{ address, port }, env, docRoot)->run();

    // Run the I/O service on the requested number of threads
    std::vector<std::thread> v;
    v.reserve(threads - 1);
    for (auto i = threads - 1; i > 0; --i)
        v.emplace_back([&ioc] { ioc.run(); });
    ioc.run();

    return EXIT_SUCCESS;
}
