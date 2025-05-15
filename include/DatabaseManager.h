#pragma once
#include "common.h"
#include "TransactionData.h"
#include "User.h"
#include "Account.h"
#include "Deposit.h"
#include "Wallet.h"

class DatabaseManager {
private:
	connection* conn;
	string connectionString;

public:
	DatabaseManager(const string& connString);

	void updateWalletBalance(int walletId, double amount, bool isAdd);

	void loadUserProducts(User* user);

	void fundWallet(int walletId, double amount);

	void closeAccount(int accountId);

	void closeDeposit(int depositId);

	void transferFunds(int senderWalletId, int receiverWalletId, double amount, const string& description);

	void loadUserProductsInline(transaction_base& txn, User* user);

	void getSystemStatistics(int& totalUsers, int& totalProducts, double& totalVolume);

	int createUser(const string& login, const string& password, const string& fullName);

	int createWallet(int userId, const string& cardNumber);

	int createAccount(int userId, const string& accountType, double initialAmount, double interestRate, int durationMonths);

	int createDeposit(int userId, double amount, double interestRate, int durationMonths);

	int createTransaction(int senderWalletId, int receiverWalletId, int senderAccountId, int receiverAccountId, int senderDepositId, int receiverDepositId, const string& type, double amount, const string& description);

	bool isConnected() const { return conn && conn->is_open(); }

	bool isAdminUser(int userId);

	bool userExists(const string& login);

	User* authenticateUser(const string& login, const string& hashedPassword);

	vector<TransactionData> getUserTransactions(int userId);

	string getCurrentTimestamp() const;

	int getTotalUsers();

	int getTotalActiveProducts();
	
	shared_ptr<Wallet> getWalletForUser(int userId);

	User* getUserById(int userId);
	
	double getTotalUserBalance(int userId);

	vector<User*> getAllUsers();

	vector<TransactionData> getAllTransactions();
	
	double getTotalTransactionVolume();

	~DatabaseManager();
};