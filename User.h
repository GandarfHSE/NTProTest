#ifndef CLIENSERVERECN_USER_H
#define CLIENSERVERECN_USER_H

#include <string>
#include <unordered_map>

class User {
public:
    std::string getLogin() {
        return login;
    }

    size_t getUid() {
        return uid;
    }

    long long getUsdBalance() {
        return usdBalance;
    }

    long long getRubBalance() {
        return rubBalance;
    }

    void setUsdBalance(long long newBalance) {
        usdBalance = newBalance;
    }

    void setRubBalance(long long newBalance) {
        rubBalance = newBalance;
    }

    User(size_t uid, std::string login, std::string password):
        uid(uid),
        login(std::move(login)),
        password(std::move(password)),
        usdBalance(0),
        rubBalance(0) {
    }

    User(): uid(0), login(""), password(""), usdBalance(0), rubBalance(0) {

    }
    
    bool isPasswordCorrect(std::string pass) {
        return password == pass;
    }

private:
    size_t uid;
    std::string login;
    std::string password;
    long long usdBalance, rubBalance;
};


class UserTable {
public:
    size_t addUser(std::string login, std::string password);

    size_t getUid(std::string login);

    bool isUserInTable(std::string login);

    bool isUserDataCorrect(std::string login, std::string password);

private:
    size_t maxUid = 1;
    // uid -> User
    std::unordered_map<size_t, User> users;
    // login -> uid
    std::unordered_map<std::string, size_t> logins;
};

#endif