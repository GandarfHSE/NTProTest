#include <cstdlib>
#include <iostream>
#include <memory>
#include <set>
#include <string>

#include <boost/bind/bind.hpp>
#include <boost/asio.hpp>

#include "json.hpp"
#include "Common.hpp"
#include "deal.h"
#include "User.h"

using boost::asio::ip::tcp;

class Core
{
public:
    std::string RegisterNewUser(const std::string& login, const std::string& pass) {
        nlohmann::json msg;
        if (userTable.isUserInTable(login)) {
            msg["err"] = Errors::LoginExists;
        } else {
            msg["err"] = Errors::NoError;
            msg["uid"] = std::to_string(userTable.addUser(login, pass));
        }
        return msg.dump();
    }

    std::string TryLogin(const std::string& login, const std::string& pass) {
        nlohmann::json msg;
        if (!userTable.isUserInTable(login)) {
            msg["err"] = Errors::LoginDoesntExist;
        }
        else if (!userTable.isUserDataCorrect(login, pass)) {
            msg["err"] = Errors::WrongPass;
        }
        else {
            msg["err"] = Errors::NoError;
            msg["uid"] = std::to_string(userTable.getUid(login));
        }
        return msg.dump();
    }

    std::string TryBuy(size_t uid, long long volume, long long cost) {
        BuyDeal myDeal(uid, volume, cost);
        while (!myDeal.getIsClosed() && !activeSells.empty() && myDeal.isCompatible(*activeSells.begin())) {
            SellDeal sell = *activeSells.begin();
            activeSells.erase(activeSells.begin());
            long long dealPrice = getDealPrice(myDeal, sell);
            long long dealVolume = getDealVolume(myDeal, sell);
            DealPair deals = completeDeals(myDeal, sell);

            if (!userTable.addUsdToUser(myDeal.getUid(), dealVolume) ||
                !userTable.addRubToUser(myDeal.getUid(), -dealPrice * dealVolume) ||
                !userTable.addUsdToUser(sell.getUid(), -dealVolume) ||
                !userTable.addRubToUser(sell.getUid(), dealPrice * dealVolume)) {
                return Errors::UserDoesntExist;
            }

            closedDeals[deals.first.getUid()].emplace_back(&deals.first);
            closedDeals[deals.second.getUid()].emplace_back(&deals.second);

            if (!sell.getIsClosed()) {
                activeSells.insert(sell);
            }
        }

        if (!myDeal.getIsClosed()) {
            activeBuys.insert(myDeal);
        }

        return Errors::NoError;
    }

    std::string TrySell(size_t uid, long long volume, long long cost) {
        SellDeal myDeal(uid, volume, cost);
        while (!myDeal.getIsClosed() && !activeBuys.empty() && myDeal.isCompatible(*activeBuys.begin())) {
            BuyDeal buy = *activeBuys.begin();
            activeBuys.erase(activeBuys.begin());
            long long dealPrice = getDealPrice(buy, myDeal);
            long long dealVolume = getDealVolume(buy, myDeal);
            DealPair deals = completeDeals(buy, myDeal);

            if (!userTable.addUsdToUser(myDeal.getUid(), -dealVolume) ||
                !userTable.addRubToUser(myDeal.getUid(), dealPrice * dealVolume) ||
                !userTable.addUsdToUser(buy.getUid(), dealVolume) ||
                !userTable.addRubToUser(buy.getUid(), -dealPrice * dealVolume)) {
                return Errors::UserDoesntExist;
            }

            closedDeals[deals.first.getUid()].emplace_back(&deals.first);
            closedDeals[deals.second.getUid()].emplace_back(&deals.second);

            if (!buy.getIsClosed()) {
                activeBuys.insert(buy);
            }
        }

        if (!myDeal.getIsClosed()) {
            activeSells.insert(myDeal);
        }

        return Errors::NoError;
    }

    std::string GetUserBalance(size_t uid) {
        nlohmann::json msg;
        if (userTable.isUserInTable(uid)) {
            msg["err"] = Errors::NoError;
            msg["rub"] = userTable.getUserRubBalance(uid);
            msg["usd"] = userTable.getUserUsdBalance(uid);
        } else {
            msg["err"] = Errors::UserDoesntExist;
        }
        return msg.dump();
    }

    std::string GetUserActiveDeals(size_t uid) {
        nlohmann::json msg;
        if (userTable.isUserInTable(uid)) {
            msg["err"] = Errors::NoError;

            for (auto& deal : activeBuys) {
                if (deal.getUid() == uid) {
                    msg["deals"].push_back(deal.getInfo());
                }
            }
            for (auto& deal : activeSells) {
                if (deal.getUid() == uid) {
                    msg["deals"].push_back(deal.getInfo());
                }
            }
        }
        else {
            msg["err"] = Errors::UserDoesntExist;
        }
        return msg.dump();
    }

    std::string GetUserClosedDeals(size_t uid) {
        nlohmann::json msg;
        if (userTable.isUserInTable(uid)) {
            msg["err"] = Errors::NoError;

            for (auto& dealPtr : closedDeals[uid]) {
                msg["deals"].push_back(dealPtr->getInfo());
            }
        }
        else {
            msg["err"] = Errors::UserDoesntExist;
        }
        return msg.dump();
    }

private:
    UserTable userTable;
    std::multiset<BuyDeal> activeBuys;
    std::multiset<SellDeal> activeSells;
    std::unordered_map<size_t, std::vector<std::unique_ptr<Deal>>> closedDeals;
};

Core& GetCore()
{
    static Core core;
    return core;
}

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