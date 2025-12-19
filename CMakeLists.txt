cmake_minimum_required(VERSION 3.10)
project(RPG_Dungeon_Simulator VERSION 1.0.0)

# Настройки C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Настройки компилятора
if(MSVC)
    add_compile_options(/W4 /WX /EHsc)
else()
    add_compile_options(-Wall -Wextra -Werror -pthread)
    add_link_options(-pthread)
endif()

# Включаем поддержку многопоточности
find_package(Threads REQUIRED)

# Файлы исходного кода
set(SOURCES
    main.cpp
    src/npc.cpp
    src/observer.cpp
    src/visitor.cpp
    src/factory.cpp
    src/dungeon.cpp
)

# Заголовочные файлы
set(HEADERS
    include/npc.h
    include/observer.h
    include/visitor.h
    include/factory.h
    include/dungeon.h
)

# Создаем исполняемый файл
add_executable(dungeon_simulator ${SOURCES} ${HEADERS})

# Подключаем библиотеку потоков
target_link_libraries(dungeon_simulator Threads::Threads)

# Для Windows добавляем дополнительную линковку
if(WIN32)
    target_link_libraries(dungeon_simulator ws2_32)
endif()

# Настройки для релизной сборки
set_target_properties(dungeon_simulator PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
)

# Копируем примеры и документацию при сборке
configure_file(README.md ${CMAKE_BINARY_DIR}/README.md COPYONLY)

# Опции для сборки
option(BUILD_TESTS "Build tests" OFF)

if(BUILD_TESTS)
    enable_testing()
    # Здесь можно добавить тесты
    # add_subdirectory(tests)
endif()

# Инструкция по установке
install(TARGETS dungeon_simulator
    RUNTIME DESTINATION bin
)

install(FILES ${HEADERS}
    DESTINATION include/rpg_dungeon
)

# Сообщение о успешной конфигурации
message(STATUS "===========================================")
message(STATUS "  RPG Dungeon Simulator - Конфигурация")
message(STATUS "===========================================")
message(STATUS "Проект: ${PROJECT_NAME}")
message(STATUS "Версия: ${PROJECT_VERSION}")
message(STATUS "C++ стандарт: ${CMAKE_CXX_STANDARD}")
message(STATUS "Компилятор: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "Сборка тестов: ${BUILD_TESTS}")
message(STATUS "===========================================")