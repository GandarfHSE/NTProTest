CMAKE_MINIMUM_REQUIRED(VERSION 3.16)
PROJECT(ClienServerEcn)

FIND_PACKAGE(Boost 1.40 COMPONENTS system REQUIRED)
FIND_PACKAGE(Threads REQUIRED)
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})

SET(CMAKE_CXX_STANDARD 17)

ADD_EXECUTABLE(Server Server.cpp Common.hpp core.cpp deal.cpp json.hpp User.cpp)
TARGET_LINK_LIBRARIES(Server PRIVATE Threads::Threads ${Boost_LIBRARIES})

ADD_EXECUTABLE(Client Client.cpp Common.hpp json.hpp)
TARGET_LINK_LIBRARIES(Client PRIVATE Threads::Threads ${Boost_LIBRARIES})

ADD_EXECUTABLE(Test test.cpp Common.hpp core.cpp deal.cpp json.hpp User.cpp)
TARGET_LINK_LIBRARIES(Test PRIVATE Threads::Threads ${Boost_LIBRARIES})
