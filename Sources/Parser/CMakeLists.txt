# Версия CMake
cmake_minimum_required(VERSION 3.5)

# Название приложения
set(TARGET_NAME "Parser")
set(TARGET_BIN_NAME "Parser")

# Утановить путь к QT
set(CMAKE_PREFIX_PATH "${QT_PATH}")

# Поиск необходимых пакетов
find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets Network REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets Network REQUIRED)

# Включить генераторы Qt
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# Добавляем .exe (проект в Visual Studio)
add_executable(${TARGET_NAME} WIN32
        "Main.cpp"
        "MainWindow.h"
        "MainWindow.cpp"
        "MainWindow.ui"

        "../../Resources/Parser/resources.qrc"
        "../../Resources/Parser/version.rc")

# Меняем название запускаемого файла в зависимости от типа сборки
set_property(TARGET ${TARGET_NAME} PROPERTY OUTPUT_NAME "${TARGET_BIN_NAME}$<$<CONFIG:Debug>:_Debug>_${PLATFORM_BIT_SUFFIX}")

# Дополнительные настройки компиляции и линковки
if(MSVC)
    target_compile_definitions(${TARGET_NAME} PUBLIC "-DNOMINMAX")
    target_compile_options(${TARGET_NAME} PUBLIC /W3 /permissive-)
    if(STATIC_RUNTIME)
        set_property(TARGET ${TARGET_NAME} PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    endif()
else()
    target_compile_options(${TARGET_NAME} PUBLIC -Wall -Wextra -Wpedantic)
    if(STATIC_RUNTIME)
        set_property(TARGET ${TARGET_NAME} PROPERTY LINK_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-Bstatic,--whole-archive -lwinpthread -Wl,--no-whole-archive, -Wl,-Bdynamic")
    endif()
endif()

# Линковка с библиотеками QT
target_link_libraries(${TARGET_NAME} PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Network)

# Аргументы для "развертывателя" (для копирования необходимых библиотек)
set(DEPLOY_ARGS "--no-quick-import" "--no-translations" "--no-system-d3d-compiler" "--no-opengl-sw" "--verbose=0")

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(DEPLOY_ARGS ${DEPLOY_ARGS} --debug)
else()
    set(DEPLOY_ARGS ${DEPLOY_ARGS} --release)
endif()

if(STATIC_RUNTIME)
    set(DEPLOY_ARGS ${DEPLOY_ARGS} --no-compiler-runtime)
else()
    set(DEPLOY_ARGS ${DEPLOY_ARGS} --compiler-runtime)
endif()

# Копирование необходимых DLL'ок QT в папку Bin после построения
add_custom_command(TARGET ${TARGET_NAME} POST_BUILD COMMAND ${QT_PATH}\\bin\\windeployqt.exe ${DEPLOY_ARGS}
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY} COMMENT "Copying QT DLLs" VERBATIM)

# Копирование необходимых DLL'ок OpenSSL в папку Bin после построения
ADD_CUSTOM_COMMAND(TARGET ${TARGET_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy ${OPEN_SSL_PATH}\\libssl-1_1-x64.dll ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
        COMMAND ${CMAKE_COMMAND} -E copy ${OPEN_SSL_PATH}\\libcrypto-1_1-x64.dll ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
        COMMENT "Copying OpenSSL DLLs" VERBATIM)