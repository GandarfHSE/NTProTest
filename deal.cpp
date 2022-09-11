#include "deal.h"
#include <chrono>
#include <exception>
#include <stdexcept>

// BuyDeal

bool BuyDeal::operator< (const BuyDeal& rhs) const {
    if (price != rhs.price) {
        return price > rhs.price;
    } else if (dealTime != rhs.dealTime) {
        return dealTime < rhs.dealTime;
    } else {
        return uid < rhs.uid;
    }
}

bool BuyDeal::isCompatible(const SellDeal deal) const {
    return price <= deal.getPrice();
}

std::string BuyDeal::getInfo() const {
    nlohmann::json info;
    info["t"] = "b";
    info["v"] = getVolume();
    info["p"] = getPrice();
    return info.dump();
}

// SellDeal

bool SellDeal::operator< (const SellDeal& rhs) const {
    if (price != rhs.price) {
        return price < rhs.price;
    } else if (dealTime != rhs.dealTime) {
        return dealTime < rhs.dealTime;
    } else {
        return uid < rhs.uid;
    }
}

bool SellDeal::isCompatible(const BuyDeal deal) const {
    return price >= deal.getPrice();
}

std::string SellDeal::getInfo() const {
    nlohmann::json info;
    info["t"] = "s";
    info["v"] = getVolume();
    info["p"] = getPrice();
    return info.dump();
}

// Take two active deals and complete them
// Return two closed deals
DealPair completeDeals(BuyDeal& buy, SellDeal& sell) {
    long long minVolume = getDealVolume(buy, sell);
    DealPair res(buy, sell);

    if (buy.getVolume() == minVolume) {
        buy.close();
        res.first = buy;
    } else {
        BuyDeal closedDeal(buy, minVolume);
        buy.subVolume(minVolume);
        closedDeal.close();
        res.first = closedDeal;
    }

    if (sell.getVolume() == minVolume) {
        sell.close();
        res.second = sell;
    } else {
        SellDeal closedDeal(sell, minVolume);
        sell.subVolume(minVolume);
        closedDeal.close();
        res.second = closedDeal;
    }

    return res;
}

long long getDealPrice(BuyDeal& buy, SellDeal& sell) {
    return (buy.getTime() < sell.getTime() ? buy.getPrice() : sell.getPrice());
}

long long getDealVolume(BuyDeal& buy, SellDeal& sell) {
    return std::min(buy.getVolume(), sell.getVolume());
}

