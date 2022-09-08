#include <string>
#include "User.h"

bool UserTable::isUserInTable(std::string login) {
    return logins.find(login) != logins.end();
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
