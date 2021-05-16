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
add_executable(${TARGET_NAME}
        "Main.cpp"
        "MainWindow.h"
        "MainWindow.cpp"
        "MainWindow.ui")

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

# Копирование необходимых DLL в папку bin после построения
set(DEPLOY_ARGS "--no-quick-import" "--no-translations" "--no-system-d3d-compiler" "--verbose=0")
if(STATIC_RUNTIME)
    set(DEPLOY_ARGS ${DEPLOY_ARGS} --no-compiler-runtime)
else()
    set(DEPLOY_ARGS ${DEPLOY_ARGS} --compiler-runtime)
endif()

add_custom_command(TARGET ${TARGET_NAME} POST_BUILD COMMAND ${QT_PATH}\\bin\\windeployqt.exe ${DEPLOY_ARGS}
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY} COMMENT "Copying DLLs" VERBATIM)