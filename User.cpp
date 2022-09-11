#include <string>
#include "User.h"

bool UserTable::isUserInTable(std::string login) {
    return logins.find(login) != logins.end();
}

bool UserTable::isUserInTable(size_t uid) {
    return users.find(uid) != users.end();
}

size_t UserTable::getUid(std::string login) {
    return (isUserInTable(login) ? logins.at(login) : 0);
}

bool UserTable::isUserDataCorrect(std::string login, std::string password) {
    return isUserInTable(login) && users[logins.at(login)].isPasswordCorrect(password);
}

size_t UserTable::addUser(std::string login, std::string password) {
    if (!isUserInTable(login)) {
        size_t myUid = maxUid++;
        User newUser(myUid, login, password);
        logins[login] = myUid;
        users[myUid] = newUser;
        return myUid;
    }
    return 0;
}

bool UserTable::addUsdToUser(size_t uid, long long volume) {
    if (!isUserInTable(uid)) {
        return false;
    }

    users[uid].addUsd(volume);
    return true;
}

bool UserTable::addRubToUser(size_t uid, long long volume) {
    if (!isUserInTable(uid)) {
        return false;
    }

    users[uid].addRub(volume);
    return true;
}

long long UserTable::getUserUsdBalance(size_t uid) {
    return users.at(uid).getUsdBalance();
}

long long UserTable::getUserRubBalance(size_t uid) {
    return users.at(uid).getRubBalance();
}