# 🏦 BankSystem

`BankSystem` — это консольное приложение для управления банковскими счетами с подключением к базе данных PostgreSQL. Написано на C++ с использованием `libpqxx`, `OpenSSL`, `CMake` и `vcpkg`.

---

## 📋 Возможности

- Хранение информации о пользователях и транзакциях
- Безопасное хеширование данных с использованием OpenSSL
- Простая архитектура и настройка
- Расширяемый C++-код

---

## 📂 Структура проекта

BankSystem/

  ── include/ # Заголовочные файлы
  
  ── src/ # Исходный код (.cpp)
  
  ── scripts/ # SQL-скрипты для создания БД
  
  ── CMakeLists.txt # CMake файл сборки
  
  ── .gitignore # Исключённые файлы
  
  ── README.md # Этот файл

## ⚙️ Требования

CMake ≥ 3.20

Компилятор C++ с поддержкой C++17 (например, MSVC или GCC)

vcpkg (пакетный менеджер)

PostgreSQL-сервер с доступом

## 💡 Важная информация

В файле подключения к БД используется строка подключения:

BankSystem.cpp
std::string connStr = "dbname=DBNAME user=USERNAME password=PASSWORD host=localhost port=PORT";

Обязательно замените DBNAME, USERNAME, PASSWORD, PORT на ваши реальные данные от PostgreSQL.

📄 SQL-скрипт для создания нужной базы данных находится в папке scripts/. Выполните его в вашей PostgreSQL-среде перед запуском приложения.

## 🛠️ Сборка проекта и установка всех необходимых библиотек
Установите CMake (https://cmake.org/download/) и vcpkg.

1. Установите vcpkg
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg
./bootstrap-vcpkg.sh        # Linux/macOS
.\bootstrap-vcpkg.bat       # Windows

2. Установите зависимости через vcpkg
vcpkg integrate install

vcpkg install libpqxx openssl    

3. Сборка через CMake
Для windows:
cmake -B build -DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%/scripts/buildsystems/vcpkg.cmake -A x64
cmake --build build --config Release

Для Linux/macOS
cmake -B build -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake
cmake --build build

## Запуск

После успешной сборки:

Убедитесь, что PostgreSQL запущен.

Замените строку подключения connStr на свои данные.

Выполните SQL-скрипт из scripts/ в своей базе данных.

Запустите программу:

./build/Release/BankSystem.exe  # Linux/macOS

cd build/Release/
BankSystem.exe  # Windows(cmd)

