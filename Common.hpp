#ifndef CLIENSERVERECN_COMMON_HPP
#define CLIENSERVERECN_COMMON_HPP

#include <string>

static short port = 5555;

namespace Requests
{
    static std::string Reg = "reg";
    static std::string Login = "log";
    static std::string Buy = "buy";
    static std::string Sell = "sell";
    static std::string Balance = "bal";
    static std::string ActiveDeals = "ad";
    static std::string ClosedDeals = "cd";
    static std::string BestPrices = "bp";
}

namespace Errors {
    static std::string NoError = "ok";
    static std::string LoginExists = "loginexists";
    static std::string LoginDoesntExist = "trevor";
    static std::string UserDoesntExist = "404";
    static std::string WrongPass = "wrongpass";
}

namespace DealInfo {
    static std::string KeyType = "t";
    static std::string KeyVolume = "v";
    static std::string KeyPrice = "p";
    static std::string TypeBuy = "b";
    static std::string TypeSell = "s";
}

namespace Fillers {
    static std::string FAKE_ID = "fake_id";
    static std::string FAKE_INFO = "fake_info";
    static std::string FAKE_NUMBER = "fake_number";
}

#endif //CLIENSERVERECN_COMMON_HPP
