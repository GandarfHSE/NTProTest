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

std::string Core::RegisterNewUser(const std::string& login, const std::string& pass) {
    nlohmann::json msg;
    if (userTable.isUserInTable(login)) {
        msg["err"] = Errors::LoginExists;
    } else {
        msg["err"] = Errors::NoError;
        msg["uid"] = std::to_string(userTable.addUser(login, pass));
    }
    return msg.dump();
}

std::string Core::TryLogin(const std::string& login, const std::string& pass) {
    nlohmann::json msg;
    if (!userTable.isUserInTable(login)) {
        msg["err"] = Errors::LoginDoesntExist;
    } else if (!userTable.isUserDataCorrect(login, pass)) {
        msg["err"] = Errors::WrongPass;
    } else {
        msg["err"] = Errors::NoError;
        msg["uid"] = std::to_string(userTable.getUid(login));
    }
    return msg.dump();
}

std::string Core::TryBuy(size_t uid, long long volume, long long cost) {
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

        closedDeals[deals.first.getUid()].emplace_back(new BuyDeal(deals.first));
        closedDeals[deals.second.getUid()].emplace_back(new SellDeal(deals.second));

        if (!sell.getIsClosed()) {
            activeSells.insert(sell);
        }
    }

    if (!myDeal.getIsClosed()) {
        activeBuys.insert(myDeal);
    }

    return Errors::NoError;
}

std::string Core::TrySell(size_t uid, long long volume, long long cost) {
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

        closedDeals[deals.first.getUid()].emplace_back(new BuyDeal(deals.first));
        closedDeals[deals.second.getUid()].emplace_back(new SellDeal(deals.second));

        if (!buy.getIsClosed()) {
            activeBuys.insert(buy);
        }
    }

    if (!myDeal.getIsClosed()) {
        activeSells.insert(myDeal);
    }

    return Errors::NoError;
}

std::string Core::GetUserBalance(size_t uid) {
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

std::string Core::GetUserActiveDeals(size_t uid) {
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
    } else {
        msg["err"] = Errors::UserDoesntExist;
    }
    return msg.dump();
}

std::string Core::GetUserClosedDeals(size_t uid) {
    nlohmann::json msg;
    if (userTable.isUserInTable(uid)) {
        msg["err"] = Errors::NoError;

        for (auto& dealPtr : closedDeals[uid]) {
            msg["deals"].push_back(dealPtr->getInfo());
        }
    } else {
        msg["err"] = Errors::UserDoesntExist;
    }
    return msg.dump();
}

std::string Core::GetBestPrices() {
    nlohmann::json msg;
    if (!activeBuys.empty()) {
        msg["buy"] = std::to_string(activeBuys.begin()->getPrice());
    } else {
        msg["buy"] = Fillers::FAKE_NUMBER;
    }

    if (!activeSells.empty()) {
        msg["sell"] = std::to_string(activeSells.begin()->getPrice());
    } else {
        msg["sell"] = Fillers::FAKE_NUMBER;
    }

    return msg.dump();
}

Core& GetCore() {
    static Core core;
    return core;
}