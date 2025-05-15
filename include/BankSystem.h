#pragma once
#include "common.h"
#include "DatabaseManager.h"
#include "Wallet.h"
#include "User.h"

class BankSystem {
private:
	DatabaseManager* dbManager;
	string currentDate;

	string getCurrentDate() const {
		time_t now = time(0);
		tm* ltm = localtime(&now);
		stringstream ss;
		ss << 1900 + ltm->tm_year << "-"
			<< setw(2) << setfill('0') << 1 + ltm->tm_mon << "-"
			<< setw(2) << setfill('0') << ltm->tm_mday << " "
			<< setw(2) << setfill('0') << ltm->tm_hour << ":"
			<< setw(2) << setfill('0') << ltm->tm_min << ":"
			<< setw(2) << setfill('0') << ltm->tm_sec;
		return ss.str();
	}

public:
	BankSystem();

	User* loginUser(const string& login, const string& password);

	int registerUser(const string& login, const string& password, const string& fullName);

	int createAccount(int userId, const string& accountType, double initialAmount, double interestRate, int durationMonths);

	int createDeposit(int userId, double amount, double interestRate, int durationMonths);

	void fundWallet(int userId, double amount);

	void refreshUserProducts(User* user);

	void closeAccount(int userId, int accountId);

	void closeDeposit(int userId, int depositId);

	void transferFunds(int senderUserId, int receiverUserId, double amount, const string& description);

	void displayLoginMenu() const;

	void displayUserMenu() const;

	void displayAdminMenu() const;

	bool isAdmin(int userId) { return dbManager->isAdminUser(userId); }

	double getTotalUserBalance(int userId) { return dbManager->getTotalUserBalance(userId); }

	void getSystemStatistics(int& totalUsers, int& totalProducts, double& totalVolume) { dbManager->getSystemStatistics(totalUsers, totalProducts, totalVolume); }

	vector<TransactionData> getUserTransactions(int userId) { return dbManager->getUserTransactions(userId); }

	vector<User*> getAllUsers() { return dbManager->getAllUsers(); }

	vector<TransactionData> getAllTransactions() { return dbManager->getAllTransactions(); }
};
