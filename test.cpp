#include <cassert>
#include <chrono>
#include <iostream>

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "json.hpp"

#include "Common.hpp"
#include "core.h"
#include "deal.h"
#include "User.h"

TEST_CASE("Test User methods") {
    size_t uid = 42;
    std::string login = "my_login";
    std::string password = "qwerty", another_password = "123456";
    User user(uid, login, password);

    SUBCASE("Test user getters") {
        CHECK(user.getUid() == uid);
        CHECK(user.getLogin() == login);
        CHECK(user.getRubBalance() == 0);
        CHECK(user.getUsdBalance() == 0);
    }

    SUBCASE("Test user password") {
        CHECK(user.isPasswordCorrect(password));
        CHECK_FALSE(user.isPasswordCorrect(another_password));
    }

    SUBCASE("Test money addition") {
        CHECK(user.getRubBalance() == 0);
        CHECK(user.getUsdBalance() == 0);

        user.addRub(10);
        user.addUsd(-37);

        CHECK(user.getRubBalance() == 10);
        CHECK(user.getUsdBalance() == -37);
    }

    SUBCASE("Test default constructor") {
        User default_user;
        CHECK(default_user.getUid() == 0);
        CHECK(default_user.getLogin() == "");
        CHECK(default_user.isPasswordCorrect(""));
        CHECK(default_user.getRubBalance() == 0);
        CHECK(default_user.getUsdBalance() == 0);
    }
}

TEST_CASE("Test UserTable methods") {
    std::string login = "my_login", another_login = "kek";
    std::string password = "qwerty", another_password = "123456";
    UserTable userTable;

    SUBCASE("Test user addition") {
        CHECK_FALSE(userTable.isUserInTable(1));
        CHECK_FALSE(userTable.isUserInTable(login));

        CHECK(userTable.addUser(login, password) == 1);
        CHECK(userTable.addUser(login, password) == 0);
        CHECK(userTable.addUser(login, another_password) == 0);

        CHECK(userTable.isUserInTable(1));
        CHECK(userTable.isUserInTable(login));

        CHECK(userTable.addUser(another_login, another_password) == 2);
    }

    SUBCASE("Test user data checking") {
        REQUIRE(userTable.addUser(login, password) == 1);

        CHECK(userTable.isUserDataCorrect(login, password));
        CHECK_FALSE(userTable.isUserDataCorrect(another_login, password));
        CHECK_FALSE(userTable.isUserDataCorrect(login, another_password));
        CHECK_FALSE(userTable.isUserDataCorrect(another_login, another_password));

        REQUIRE(userTable.addUser(another_login, another_password) == 2);
        CHECK(userTable.isUserDataCorrect(another_login, another_password));
    }

    SUBCASE("Test getUid") {
        REQUIRE(userTable.addUser(login, password) == 1);
        CHECK(userTable.getUid(login) == 1);
        CHECK(userTable.getUid(another_login) == 0);

        REQUIRE(userTable.addUser(another_login, another_password) == 2);
        CHECK(userTable.getUid(login) == 1);
        CHECK(userTable.getUid(another_login) == 2);
    }

    SUBCASE("Test money addition") {
        REQUIRE(userTable.addUser(login, password) == 1);
        CHECK(userTable.getUserRubBalance(1) == 0);
        CHECK(userTable.getUserUsdBalance(1) == 0);

        CHECK(userTable.addRubToUser(1, 42));
        CHECK(userTable.addUsdToUser(1, -37));
        CHECK(userTable.getUserRubBalance(1) == 42);
        CHECK(userTable.getUserUsdBalance(1) == -37);

        CHECK_FALSE(userTable.addRubToUser(2, 1));
        CHECK_FALSE(userTable.addUsdToUser(2, 1));
        CHECK_THROWS(userTable.getUserRubBalance(2));
    }
}

TEST_CASE("Test Deal methods") {
    size_t uid = 42;
    long long price = 37, volume = 13;
    Time time = std::chrono::system_clock::now();
    Deal deal(uid, volume, price, time);

    SUBCASE("Test Deal getters") {
        CHECK(deal.getUid() == uid);
        CHECK(deal.getPrice() == price);
        CHECK(deal.getVolume() == volume);
        CHECK(deal.getTime() == time);
        CHECK(deal.getInfo() == Fillers::FAKE_INFO);
        CHECK_FALSE(deal.getIsClosed());
    }

    SUBCASE("Test Deal close") {
        CHECK_FALSE(deal.getIsClosed());
        deal.close();
        CHECK(deal.getIsClosed());
    }

    SUBCASE("Test Deal subVolume") {
        CHECK(deal.getVolume() == volume);
        deal.subVolume(2);
        CHECK(deal.getVolume() == volume - 2);
    }
}

TEST_CASE("Test BuyDeal and SellDeal methods") {
    size_t uid = 42;
    long long price = 37, volume = 13;
    Time time = std::chrono::system_clock::now();

    BuyDeal buy(uid, volume, price, time), buyH(uid, volume, price + 1, time), buyL(uid, volume, price - 1, time);
    SellDeal sell(uid, volume, price, time), sellH(uid, volume, price + 1, time), sellL(uid, volume, price - 1, time);

    SUBCASE("Test BuyDeal getters") {
        CHECK(buy.getUid() == uid);
        CHECK(buy.getPrice() == price);
        CHECK(buy.getVolume() == volume);
        CHECK(buy.getTime() == time);
        CHECK_FALSE(buy.getIsClosed());

        nlohmann::json info;
        info[DealInfo::KeyType] = DealInfo::TypeBuy;
        info[DealInfo::KeyVolume] = volume;
        info[DealInfo::KeyPrice] = price;
        CHECK(buy.getInfo() == info.dump());
    }

    SUBCASE("Test SellDeal getters") {
        CHECK(sell.getUid() == uid);
        CHECK(sell.getPrice() == price);
        CHECK(sell.getVolume() == volume);
        CHECK(sell.getTime() == time);
        CHECK_FALSE(sell.getIsClosed());

        nlohmann::json info;
        info[DealInfo::KeyType] = DealInfo::TypeSell;
        info[DealInfo::KeyVolume] = volume;
        info[DealInfo::KeyPrice] = price;
        CHECK(sell.getInfo() == info.dump());
    }

    SUBCASE("Test compatibility") {
        CHECK(buy.isCompatible(sell));
        CHECK(sell.isCompatible(buy));

        CHECK(buy.isCompatible(sellL));
        CHECK_FALSE(buy.isCompatible(sellH));

        CHECK(sell.isCompatible(buyH));
        CHECK_FALSE(sell.isCompatible(buyL));
    }

    SUBCASE("Test operator <") {
        CHECK(buyH < buy);
        CHECK(buy < buyL);

        CHECK(sell < sellH);
        CHECK(sellL < sell);
    }
}

TEST_CASE("Test Deal functions") {
    size_t uid = 42;
    long long price = 37, volume = 13;
    Time time = std::chrono::system_clock::now();

    BuyDeal buy(uid, volume, price, time);
    SellDeal sell(uid, volume, price, time);

    SUBCASE("Test getDealVolume") {
        CHECK(getDealVolume(buy, sell) == volume);

        BuyDeal another_buy(uid, volume - 1, price, time);
        SellDeal another_sell(uid, volume - 1, price, time);

        CHECK(getDealVolume(buy, another_sell) == volume - 1);
        CHECK(getDealVolume(another_buy, sell) == volume - 1);
    }

    SUBCASE("Test getDealPrice") {
        CHECK(getDealPrice(buy, sell) == price);

        time -= std::chrono::system_clock::duration(1);
        BuyDeal another_buy(uid, volume, price - 5, time);
        SellDeal another_sell(uid, volume, price + 5, time);

        CHECK(getDealPrice(another_buy, sell) == price - 5);
        CHECK(getDealPrice(buy, another_sell) == price + 5);
    }

    SUBCASE("Test completeDeals") {
        SUBCASE("Test full completion") {
            long long dealVolume = getDealVolume(buy, sell);
            DealPair deals = completeDeals(buy, sell);

            CHECK(buy.getIsClosed());
            CHECK(buy.getVolume() == dealVolume);
            CHECK(deals.first == buy);

            CHECK(sell.getIsClosed());
            CHECK(sell.getVolume() == dealVolume);
            CHECK(deals.second == sell);
        }

        SUBCASE("Test buy completion") {
            SellDeal another_sell(uid, volume + 10, price, time);
            long long dealVolume = getDealVolume(buy, another_sell);
            DealPair deals = completeDeals(buy, another_sell);

            CHECK(buy.getIsClosed());
            CHECK(buy.getVolume() == dealVolume);
            CHECK(deals.first == buy);

            CHECK_FALSE(deals.second == another_sell);
            CHECK(deals.second.getIsClosed());
            CHECK_FALSE(another_sell.getIsClosed());
            CHECK(deals.second.getVolume() == dealVolume);
            CHECK(another_sell.getVolume() == 10);
        }

        SUBCASE("Test sell completion") {
            BuyDeal another_buy(uid, volume + 10, price, time);
            long long dealVolume = getDealVolume(another_buy, sell);
            DealPair deals = completeDeals(another_buy, sell);

            CHECK_FALSE(deals.first == another_buy);
            CHECK(deals.first.getIsClosed());
            CHECK_FALSE(another_buy.getIsClosed());
            CHECK(deals.first.getVolume() == dealVolume);
            CHECK(another_buy.getVolume() == 10);

            CHECK(sell.getIsClosed());
            CHECK(sell.getVolume() == dealVolume);
            CHECK(deals.second == sell);
        }
    }
}
