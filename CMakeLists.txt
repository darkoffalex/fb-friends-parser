# Версия CMake
cmake_minimum_required(VERSION 3.5)

# Название проекта (решение в Visual Studio)
project(FbFriendsParser)

# Стандарт С/С++
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Устанавливаем каталоги для бинарников
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/Bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/Bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/Bin)

# Включение из рабочей CMAKE директории (для удобства включения генерируемых файлов)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Определить разрядность платформы
if("${CMAKE_SIZEOF_VOID_P}" STREQUAL "4")
    set(PLATFORM_BIT_SUFFIX "x86")
else()
    set(PLATFORM_BIT_SUFFIX "x64")
endif()

# Статическая линковка рантайма
set(STATIC_RUNTIME ON)

# Путь к QT
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set(QT_PATH "C:\\Qt\\6.1.0\\msvc2019_64")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(QT_PATH "C:\\Qt\\6.1.0\\mingw81_64")
endif()

# Путь к OpenSSL
set(OPEN_SSL_PATH "C:\\Program Files\\OpenSSL-Win64\\bin")

# Старндартные C/C++ библиотеки для MinGW/GCC (для статической линковки рантайма)
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(CMAKE_CXX_STANDARD_LIBRARIES "-static-libgcc -static-libstdc++ -lwsock32 -lws2_32 ${CMAKE_CXX_STANDARD_LIBRARIES}")
endif()

# Примеры реализаций
add_subdirectory("Sources/Parser")