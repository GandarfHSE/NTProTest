#ifndef CLIENSERVERECN_COMMON_HPP
#define CLIENSERVERECN_COMMON_HPP

#include <string>

static short port = 5555;

namespace Requests
{
    static std::string Registration = "Reg";
    static std::string Hello = "Hel";
    static std::string Reg = "reg";
    static std::string Login = "log";
}

namespace Errors {
    static std::string NoError = "ok";
    static std::string LoginExists = "loginexists";
    static std::string LoginDoesntExist = "trevor";
    static std::string WrongPass = "wrongpass";
}

#endif //CLIENSERVERECN_COMMON_HPP
