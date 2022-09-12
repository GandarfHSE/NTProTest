#include <cstdlib>
#include <iostream>
#include <memory>
#include <set>
#include <string>

#include <boost/bind/bind.hpp>
#include <boost/asio.hpp>

#include "json.hpp"

#include "Common.hpp"
#include "core.h"
#include "deal.h"
#include "User.h"

using boost::asio::ip::tcp;

class session
{
public:
    session(boost::asio::io_service& io_service)
        : socket_(io_service)
    {
    }

    tcp::socket& socket()
    {
        return socket_;
    }

    void start()
    {
        socket_.async_read_some(boost::asio::buffer(data_, max_length),
            boost::bind(&session::handle_read, this,
                boost::asio::placeholders::error,
                boost::asio::placeholders::bytes_transferred));
    }

    // Обработка полученного сообщения.
    void handle_read(const boost::system::error_code& error,
        size_t bytes_transferred)
    {
        if (!error) {
            data_[bytes_transferred] = '\0';

            // Парсим json, который пришёл нам в сообщении.
            auto j = nlohmann::json::parse(data_);
            auto reqType = j["ReqType"];

            std::string reply = "Error! Unknown request type";
            auto msg = nlohmann::json::parse(static_cast<std::string>(j["Message"]));
            if (reqType == Requests::Reg) {
                reply = GetCore().RegisterNewUser(msg["login"], msg["password"]);
            }
            else if (reqType == Requests::Login) {
                reply = GetCore().TryLogin(msg["login"], msg["password"]);
            }
            else if (reqType == Requests::Buy) {
                std::string uid = msg["uid"];
                reply = GetCore().TryBuy(std::stoi(uid), msg["vol"], msg["cost"]);
            }
            else if (reqType == Requests::Sell) {
                std::string uid = msg["uid"];
                reply = GetCore().TrySell(std::stoi(uid), msg["vol"], msg["cost"]);
            }
            else if (reqType == Requests::Balance) {
                std::string uid = msg["uid"];
                reply = GetCore().GetUserBalance(std::stoi(uid));
            }
            else if (reqType == Requests::ActiveDeals) {
                std::string uid = msg["uid"];
                reply = GetCore().GetUserActiveDeals(std::stoi(uid));
            }
            else if (reqType == Requests::ClosedDeals) {
                std::string uid = msg["uid"];
                reply = GetCore().GetUserClosedDeals(std::stoi(uid));
            }
            else if (reqType == Requests::BestPrices) {
                reply = GetCore().GetBestPrices();
            }

            boost::asio::async_write(socket_,
                boost::asio::buffer(reply, reply.size()),
                boost::bind(&session::handle_write, this,
                    boost::asio::placeholders::error));
        }
        else {
            delete this;
        }
    }

    void handle_write(const boost::system::error_code& error)
    {
        if (!error)
        {
            socket_.async_read_some(boost::asio::buffer(data_, max_length),
                boost::bind(&session::handle_read, this,
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred));
        }
        else
        {
            delete this;
        }
    }

private:
    tcp::socket socket_;
    enum { max_length = 1024 };
    char data_[max_length];
};

class server
{
public:
    server(boost::asio::io_service& io_service)
        : io_service_(io_service),
        acceptor_(io_service, tcp::endpoint(tcp::v4(), port))
    {
        std::cout << "Server started! Listen " << port << " port" << std::endl;

        session* new_session = new session(io_service_);
        acceptor_.async_accept(new_session->socket(),
            boost::bind(&server::handle_accept, this, new_session,
                boost::asio::placeholders::error));
    }

    void handle_accept(session* new_session,
        const boost::system::error_code& error)
    {
        if (!error)
        {
            new_session->start();
            new_session = new session(io_service_);
            acceptor_.async_accept(new_session->socket(),
                boost::bind(&server::handle_accept, this, new_session,
                    boost::asio::placeholders::error));
        }
        else
        {
            delete new_session;
        }
    }

private:
    boost::asio::io_service& io_service_;
    tcp::acceptor acceptor_;
};

int main()
{
    try
    {
        boost::asio::io_service io_service;
        static Core core;

        server s(io_service);

        io_service.run();
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}