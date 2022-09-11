#pragma once
#ifndef CLIENSERVERECN_DEAL_H
#define CLIENSERVERECN_DEAL_H

#include <chrono>
#include "Common.hpp"
#include "json.hpp"

typedef std::chrono::time_point<std::chrono::system_clock> Time;

class Deal {
public:
    Deal(size_t uid, long long volume, long long price, Time time) :
        uid(uid),
        volume(volume),
        price(price),
        dealTime(time) {
        isClosed = false;
    }

    Deal(size_t uid, long long volume, long long price) :
        uid(uid),
        volume(volume),
        price(price) {
        dealTime = std::chrono::system_clock::now();
        isClosed = false;
    }

    long long getPrice() const {
        return price;
    }

    long long getVolume() const {
        return volume;
    }

    void subVolume(long long volumeToSub) {
        volume -= volumeToSub;
    }

    size_t getUid() const {
        return uid;
    }

    Time getTime() const {
        return dealTime;
    }

    void close() {
        isClosed = true;
    }

    bool getIsClosed() const {
        return isClosed;
    }

    virtual std::string getInfo() const = 0;

protected:
    size_t uid;
    long long volume;
    long long price;
    Time dealTime;
    bool isClosed;
};

class SellDeal;

class BuyDeal : public Deal {
public:
    BuyDeal(size_t uid, long long volume, long long price) : Deal(uid, volume, price) {

    }

    BuyDeal(size_t uid, long long volume, long long price, Time time) : Deal(uid, volume, price, time) {

    }

    BuyDeal(const BuyDeal& other, long long volume) : Deal(other.getUid(),
                                                           volume,
                                                           other.getPrice(),
                                                           other.getTime()) {

    }

    BuyDeal(const BuyDeal& other) : Deal(other.getUid(),
                                         other.getVolume(),
                                         other.getPrice(),
                                         other.getTime()) {

    }

    bool operator < (const BuyDeal& rhs) const;

    bool isCompatible(const SellDeal deal) const;

    virtual std::string getInfo() const;
};

class SellDeal : public Deal {
public:
    SellDeal(size_t uid, long long volume, long long price) : Deal(uid, volume, price) {

    }

    SellDeal(size_t uid, long long volume, long long price, Time time) : Deal(uid, volume, price, time) {

    }

    SellDeal(const SellDeal& other, long long volume) : Deal(other.getUid(),
                                                             volume,
                                                             other.getPrice(),
                                                             other.getTime()) {

    }

    SellDeal(const SellDeal& other) : Deal(other.getUid(),
                                           other.getVolume(),
                                           other.getPrice(),
                                           other.getTime()) {

    }

    bool operator < (const SellDeal& rhs) const;

    bool isCompatible(const BuyDeal deal) const;

    virtual std::string getInfo() const;
};

typedef std::pair<BuyDeal, SellDeal> DealPair;

// Take two active deals and complete them
// Return two closed deals
DealPair completeDeals(BuyDeal& buy, SellDeal& sell);

long long getDealPrice(BuyDeal& buy, SellDeal& sell);

long long getDealVolume(BuyDeal& buy, SellDeal& sell);

#endif