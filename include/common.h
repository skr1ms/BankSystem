#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <iomanip>
#include <memory>
#include <algorithm>
#include <regex>
#include <openssl/sha.h>
#include <pqxx/pqxx>

#pragma warning(disable : 4996)

using namespace std;
using namespace pqxx;

struct TransactionData;
class FinancialProduct;
class Wallet;
class Account;
class Deposit;
class User;
class Transaction;
class DatabaseManager;
