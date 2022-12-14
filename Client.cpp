#include <iostream>
#include <string>
#include <boost/asio.hpp>

#include "Common.hpp"
#include "json.hpp"

using boost::asio::ip::tcp;

// Отправка сообщения на сервер по шаблону.
void SendMessage(
    tcp::socket& aSocket,
    const std::string& aRequestType,
    const std::string& aMessage)
{
    nlohmann::json req;
    req["ReqType"] = aRequestType;
    req["Message"] = aMessage;

    std::string request = req.dump();
    boost::asio::write(aSocket, boost::asio::buffer(request, request.size()));
}

// Возвращает строку с ответом сервера на последний запрос.
std::string ReadMessage(tcp::socket& aSocket)
{
    boost::asio::streambuf b;
    boost::asio::read_until(aSocket, b, "\0");
    std::istream is(&b);
    std::string line(std::istreambuf_iterator<char>(is), {});
    return line;
}

std::string RegistrationProcess(tcp::socket& aSocket) {
    std::string name, pass, pass2;
    std::cout << "Enter your login: ";
    std::cin >> name;
    while (true) {
        std::cout << "Enter your password: ";
        std::cin >> pass;
        std::cout << "Enter your password again: ";
        std::cin >> pass2;

        if (pass == pass2) {
            break;
        }

        std::cout << "Sorry, passwords do not match. Try again\n";
    }

    nlohmann::json msg;
    msg["login"] = name;
    msg["password"] = pass;
    SendMessage(aSocket, Requests::Reg, msg.dump());

    auto reply = nlohmann::json::parse(ReadMessage(aSocket));
    if (reply["err"] == Errors::NoError) {
        std::cout << "Registration successfully complete, " << name << '\n';
        return reply["uid"];
    } 
    else if (reply["err"] == Errors::LoginExists) {
        std::cout << "Sorry, user with this login already exists. Log-in or register with another login\n";
    }
    else {
        std::cout << "Something strange have occured. Contact administrator. Error: REG_ERROR\n";
    }
    return Fillers::FAKE_ID;
}

std::string LoginProcess(tcp::socket& aSocket) {
    std::string name, pass;
    std::cout << "Enter your login: ";
    std::cin >> name;
    std::cout << "Enter your password: ";
    std::cin >> pass;

    nlohmann::json msg;
    msg["login"] = name;
    msg["password"] = pass;
    SendMessage(aSocket, Requests::Login, msg.dump());

    auto reply = nlohmann::json::parse(ReadMessage(aSocket));
    if (reply["err"] == Errors::NoError) {
        std::cout << "Login successfully complete, " << name << '\n';
        return reply["uid"];
    }
    else if (reply["err"] == Errors::WrongPass) {
        std::cout << "Wrong password. Register new account or try another password\n";
    }
    else if (reply["err"] == Errors::LoginDoesntExist) {
        std::cout << "Login doesn't exist. Register new account or try another login\n";
    }
    else {
        std::cout << "Something strange have occured. Contact administrator. Error: LOGIN_ERROR\n";
    }
    return Fillers::FAKE_ID;
}

void MakeDealProcess(tcp::socket& aSocket, std::string my_id, bool isBuy) {
    long long volume, cost;
    if (isBuy) {
        std::cout << "Creating buy request...\n";
    }
    else {
        std::cout << "Creating sell request...\n";
    }

    std::cout << "Enter USD volume: ";
    std::cin >> volume;
    std::cout << "Enter cost: ";
    std::cin >> cost;

    nlohmann::json msg;
    msg["uid"] = my_id;
    msg["vol"] = volume;
    msg["cost"] = cost;
    SendMessage(aSocket, (isBuy ? Requests::Buy : Requests::Sell), msg.dump());

    auto reply = ReadMessage(aSocket);
    if (reply == Errors::NoError) {
        std::cout << "Deal request successfully processed\n";
    }
    else {
        std::cout << "Something strange have occured. Contact administrator. Error: BUY_ERROR\n";
    }
}

void CheckBalance(tcp::socket& aSocket, std::string my_id) {
    nlohmann::json msg;
    msg["uid"] = my_id;
    SendMessage(aSocket, Requests::Balance, msg.dump());
    auto reply = nlohmann::json::parse(ReadMessage(aSocket));
    if (reply["err"] == Errors::NoError) {
        std::cout << "Your balance:\n" << reply["usd"] << " USD\n" << reply["rub"] << " RUB\n";
    }
    else {
        std::cout << "Something strange have occured. Contact administrator. Error: BALANCE_ERROR\n";
    }
}

void PrintDeal(std::string jsonStr) {       // jsonStr - dumped json from deal.getInfo()
    auto info = nlohmann::json::parse(jsonStr);
    if (info[DealInfo::KeyType] == DealInfo::TypeBuy) {
        std::cout << "Buy ";
    } else {
        std::cout << "Sell ";
    }
    std::cout << info[DealInfo::KeyVolume] << " USD for " << info[DealInfo::KeyPrice] << " RUB each\n";
}

void PrintDeals(tcp::socket& aSocket, std::string my_id, bool is_active) {
    nlohmann::json msg;
    msg["uid"] = my_id;
    SendMessage(aSocket, (is_active ? Requests::ActiveDeals : Requests::ClosedDeals), msg.dump());
    auto reply = nlohmann::json::parse(ReadMessage(aSocket));
    if (reply["err"] == Errors::NoError) {
        if (reply["deals"].empty()) {
            std::cout << "You have no " << (is_active ? "active" : "closed") << " deals now\n";
        }
        else {
            std::cout << (is_active ? "Your active deals:\n" : "Your closed deals:\n");
            int i = 1;
            for (auto& str : reply["deals"]) {
                std::cout << i++ << ") ";
                PrintDeal(str);
            }
        }
    } else {
        std::cout << "Something strange have occured. Contact administrator. Error: PRINTDEALS_ERROR\n";
    }
}

void GetBestPrices(tcp::socket& aSocket) {
    SendMessage(aSocket, Requests::BestPrices, "{}");
    auto reply = nlohmann::json::parse(ReadMessage(aSocket));
    std::cout << "Best current prices:\n";

    if (reply["buy"] == Fillers::FAKE_NUMBER) {
        std::cout << "There's no active buy request\n";
    } else {
        std::cout << "Best buy price is " << std::stoll(static_cast<std::string>(reply["buy"])) << " RUB for 1 USD\n";
    }

    if (reply["sell"] == Fillers::FAKE_NUMBER) {
        std::cout << "There's no active sell request\n";
    } else {
        std::cout << "Best sell price is " << std::stoll(static_cast<std::string>(reply["sell"])) << " RUB for 1 USD\n";
    }
}

int main()
{
    try
    {
        boost::asio::io_service io_service;

        tcp::resolver resolver(io_service);
        tcp::resolver::query query(tcp::v4(), "127.0.0.1", std::to_string(port));
        tcp::resolver::iterator iterator = resolver.resolve(query);

        tcp::socket s(io_service);
        s.connect(*iterator);

        std::string my_id = "fake_id";

        while (my_id == Fillers::FAKE_ID) {
            std::cout << "Greetings! Please, register or login\n"
                         "1) Register new account\n"
                         "2) Login to existing account\n"
                      << std::endl;

            short menu_option;
            std::cin >> menu_option;
            switch (menu_option) {
                case 1:
                {
                    my_id = RegistrationProcess(s);
                    break;
                }
                case 2:
                {
                    my_id = LoginProcess(s);
                    break;
                }
                default:
                {
                    std::cout << "Unknown menu option\n";
                }
            }
            std::cout << "\n----------------------------------------------------\n";
        }

        while (true)
        {
            std::cout << "Please, choose menu option:\n"
                         "1) Buy USD\n"
                         "2) Sell USD\n"
                         "3) Check balance\n"
                         "4) Check my active deals\n"
                         "5) Check my closed deals\n"
                         "6) Check best prices\n"
                         "0) Exit\n"
                      << std::endl;

            short menu_option;
            std::cin >> menu_option;

            if (menu_option == 1) {                         // Buy
                MakeDealProcess(s, my_id, /*isBuy = */ true);
            }
            else if (menu_option == 2) {                    // Sell
                MakeDealProcess(s, my_id, /*isBuy = */ false);
            }
            else if (menu_option == 3) {                    // Check balance
                CheckBalance(s, my_id);
            }
            else if (menu_option == 4) {                    // Check active deals
                PrintDeals(s, my_id, /*isActive = */ true);
            }
            else if (menu_option == 5) {                    // Check closed deals
                PrintDeals(s, my_id, /*isActive = */ false);
            }
            else if (menu_option == 6) {                    // Check best prices
                GetBestPrices(s);
            }
            else if (menu_option == 0) {                    // Exit
                std::cout << "Exit successfully\n";
                exit(0);
            }
            else {
                std::cout << "Unknown menu option\n";
            }

            std::cout << "\n----------------------------------------------------\n";
        }
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}