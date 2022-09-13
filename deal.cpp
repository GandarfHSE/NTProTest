#include <chrono>
#include <exception>
#include <stdexcept>

#include "Common.hpp"
#include "deal.h"

std::string Deal::getInfo() const {
    return Fillers::FAKE_INFO;
}

// BuyDeal

bool BuyDeal::operator == (const BuyDeal& rhs) const {
    return uid      == rhs.uid      &&
           price    == rhs.price    &&
           volume   == rhs.volume   &&
           dealTime == rhs.dealTime &&
           isClosed == rhs.isClosed;
}

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
    return price >= deal.getPrice();
}

std::string BuyDeal::getInfo() const {
    nlohmann::json info;
    info[DealInfo::KeyType] = DealInfo::TypeBuy;
    info[DealInfo::KeyVolume] = getVolume();
    info[DealInfo::KeyPrice] = getPrice();
    return info.dump();
}

// SellDeal

bool SellDeal::operator == (const SellDeal& rhs) const {
    return uid      == rhs.uid      &&
           price    == rhs.price    &&
           volume   == rhs.volume   &&
           dealTime == rhs.dealTime &&
           isClosed == rhs.isClosed;
}

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
    return price <= deal.getPrice();
}

std::string SellDeal::getInfo() const {
    nlohmann::json info;
    info[DealInfo::KeyType] = DealInfo::TypeSell;
    info[DealInfo::KeyVolume] = getVolume();
    info[DealInfo::KeyPrice] = getPrice();
    return info.dump();
}

// Take two active deals and complete them
// Return two closed deals
DealPair completeDeals(BuyDeal& buy, SellDeal& sell) {
    long long minVolume = getDealVolume(buy, sell);
    long long dealPrice = getDealPrice(buy, sell);
    DealPair res(buy, sell);

    if (buy.getVolume() == minVolume) {
        buy.close();
    } else {
        buy.subVolume(minVolume);
    }

    res.first = BuyDeal(buy.getUid(), minVolume, dealPrice, buy.getTime());
    res.first.close();

    if (sell.getVolume() == minVolume) {
        sell.close();
    } else {
        sell.subVolume(minVolume);
    }

    res.second = SellDeal(sell.getUid(), minVolume, dealPrice, sell.getTime());
    res.second.close();

    return res;
}

long long getDealPrice(BuyDeal& buy, SellDeal& sell) {
    return (buy.getTime() < sell.getTime() ? buy.getPrice() : sell.getPrice());
}

long long getDealVolume(BuyDeal& buy, SellDeal& sell) {
    return std::min(buy.getVolume(), sell.getVolume());
}

