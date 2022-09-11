#ifndef CLIENSERVERECN_USER_H
#define CLIENSERVERECN_USER_H

#include <string>
#include <unordered_map>

class User {
public:
    std::string getLogin() const {
        return login;
    }

    size_t getUid() const {
        return uid;
    }

    long long getUsdBalance() const {
        return usdBalance;
    }

    long long getRubBalance() const {
        return rubBalance;
    }

    void addUsd(long long volume) {
        usdBalance += volume;
    }

    void addRub(long long volume) {
        rubBalance += volume;
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
    
    bool isPasswordCorrect(std::string pass) const {
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

    bool isUserInTable(size_t uid);

    bool isUserDataCorrect(std::string login, std::string password);

    bool addUsdToUser(size_t uid, long long volume);

    bool addRubToUser(size_t uid, long long volume);

    long long getUserUsdBalance(size_t uid);

    long long getUserRubBalance(size_t uid);

private:
    size_t maxUid = 1;
    // uid -> User
    std::unordered_map<size_t, User> users;
    // login -> uid
    std::unordered_map<std::string, size_t> logins;
};

#endif