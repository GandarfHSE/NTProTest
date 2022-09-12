#pragma once
#ifndef CLIENSERVERECN_CORE_H
#define CLIENSERVERECN_CORE_H

#include <memory>
#include <set>
#include <string>

#include "deal.h"
#include "User.h"

class Core
{
public:
    std::string RegisterNewUser(const std::string& login, const std::string& pass);

    std::string TryLogin(const std::string& login, const std::string& pass);

    std::string TryBuy(size_t uid, long long volume, long long cost);

    std::string TrySell(size_t uid, long long volume, long long cost);

    std::string GetUserBalance(size_t uid);

    std::string GetUserActiveDeals(size_t uid);

    std::string GetUserClosedDeals(size_t uid);

    std::string GetBestPrices();

private:
    UserTable userTable;
    std::multiset<BuyDeal> activeBuys;
    std::multiset<SellDeal> activeSells;
    std::unordered_map<size_t, std::vector<std::unique_ptr<Deal>>> closedDeals;
};

Core& GetCore();

#endif