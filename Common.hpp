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
}

namespace Errors {
    static std::string NoError = "ok";
    static std::string LoginExists = "loginexists";
    static std::string LoginDoesntExist = "trevor";
    static std::string UserDoesntExist = "404";
    static std::string WrongPass = "wrongpass";
}

#endif //CLIENSERVERECN_COMMON_HPP
