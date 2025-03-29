cmake_minimum_required(VERSION 3.5)
project(LuntikFarm)
include(FetchContent)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake_modules")

set(CMAKE_CXX_FLAGS -DVERBOSE)

set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
set(BUILD_SHARED_LIBS OFF)
set(CMAKE_EXE_LINKER_FLAGS "-static")

add_executable(LuntikFarm src/main.cpp
        src/Networking/Common.cpp
        src/Networking/Common.h
        src/Networking/Overloads.cpp
        src/Networking/SocketServer.h
        src/Networking/SocketServer.cpp
        src/Networking/SocketClient.h
        src/Networking/SocketClient.cpp
        src/Utils/Utils.h
        libs/logy/logy.h
        src/Server/Server.cpp
        src/Server/Server.h
        src/Utils/Timers.h
        src/Client/Client.cpp
        src/Client/Client.h
        src/Packets.h
        src/Client/ClientGameState.h
        src/Client/Renderer/Renderer.cpp
        src/Client/Renderer/Renderer.h
        src/Server/ServerGameState.h
        src/Server/ServerPlayerInfo.h
        src/Client/Map.cpp
        src/Client/Map.h
        src/Server/MapInfo.h
        src/Server/Structure.h
        src/NetworkEntityMap.h
        src/Server/Farm.h
        src/Client/InputManager.cpp
        src/Client/InputManager.h
        src/Server/Soldier.h
        src/Client/Renderer/YSort.h
        src/Server/Position.h
        src/Client/InterpolatedPosition.h
        src/Server/Hitbox.h
        src/opts.h
)

target_include_directories(LuntikFarm PRIVATE src)
file(COPY assets DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

# SFML
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

FetchContent_Declare(SFML
        GIT_REPOSITORY https://github.com/SFML/SFML.git
        GIT_TAG 3.0.0)
FetchContent_MakeAvailable(SFML)

target_link_libraries(LuntikFarm PRIVATE sfml-graphics sfml-network sfml-graphics sfml-system)

install(TARGETS LuntikFarm)

# ENTT
target_include_directories(LuntikFarm PRIVATE libs/entt)

# LOGY
target_include_directories(LuntikFarm PRIVATE libs/logy)
