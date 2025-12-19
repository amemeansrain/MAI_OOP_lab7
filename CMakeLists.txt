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
    # Убрали -Werror чтобы предупреждения не останавливали компиляцию
    add_compile_options(-Wall -Wextra -Wno-unused-parameter -pthread)
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

# Указываем include директории
target_include_directories(dungeon_simulator PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

# Подключаем библиотеку потоков
target_link_libraries(dungeon_simulator Threads::Threads)

# Для Windows добавляем дополнительную линковку
if(WIN32)
    target_link_libraries(dungeon_simulator ws2_32)
endif()

# Настройки для релизной сборки
set_target_properties(dungeon_simulator PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
)

# Сообщение о успешной конфигурации
message(STATUS "===========================================")
message(STATUS "  RPG Dungeon Simulator - Конфигурация")
message(STATUS "===========================================")
message(STATUS "Проект: ${PROJECT_NAME}")
message(STATUS "Версия: ${PROJECT_VERSION}")
message(STATUS "C++ стандарт: ${CMAKE_CXX_STANDARD}")
message(STATUS "Компилятор: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "Исходные файлы: ${SOURCES}")
message(STATUS "Заголовочные файлы: ${HEADERS}")
message(STATUS "Выходной файл: ${CMAKE_BINARY_DIR}/dungeon_simulator")
message(STATUS "===========================================")

# Копируем README при сборке (если есть)
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
    configure_file(README.md ${CMAKE_BINARY_DIR}/README.md COPYONLY)
endif()

# Добавляем опцию для тестов
option(BUILD_TESTS "Build tests" OFF)

if(BUILD_TESTS)
    enable_testing()
    # Здесь можно добавить тесты
    # add_subdirectory(tests)
endif()

# Опция для отладочной сборки
option(DEBUG_BUILD "Build with debug symbols" OFF)
if(DEBUG_BUILD)
    add_compile_definitions(DEBUG)
    if(MSVC)
        add_compile_options(/Od /Zi)
    else()
        add_compile_options(-O0 -g)
    endif()
else()
    if(MSVC)
        add_compile_options(/O2)
    else()
        add_compile_options(-O2)
    endif()
endif()

# Дополнительная опция для verbose вывода
option(VERBOSE_OUTPUT "Enable verbose output" OFF)
if(VERBOSE_OUTPUT)
    add_compile_definitions(VERBOSE_OUTPUT)
endif()

# Создаем цель для запуска
add_custom_target(run
    COMMAND ${CMAKE_BINARY_DIR}/dungeon_simulator
    DEPENDS dungeon_simulator
    COMMENT "Запуск RPG Dungeon Simulator"
)

# Создаем цель для очистки
add_custom_target(clean_all
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}
    COMMENT "Очистка всей директории сборки"
)

# Добавляем инструкции по установке
install(TARGETS dungeon_simulator
    RUNTIME DESTINATION bin
    BUNDLE DESTINATION bin
)

install(FILES ${HEADERS}
    DESTINATION include/rpg_dungeon
)