#include "BankSystem.h"

BankSystem::BankSystem() {
	try {
		std::string connStr = "dbname=DBNAME user=USERNAME password=PASSWORD host=localhost port=PORT";
		dbManager = new DatabaseManager(connStr);
		currentDate = getCurrentDate();
	}
	catch (const exception& e) {
		cerr << "Failed to initialize bank system: " << e.what() << endl;
		throw;
	}
}

int BankSystem::registerUser(const string& login, const string& password, const string& fullName) {
	if (dbManager->userExists(login)) {
		throw runtime_error("A user with this login already exists");
	}

	if (!User::validatePassword(password)) {
		throw invalid_argument("Password must be at least 8 characters long and contain at least one special character");
	}

	string hashedPassword = User::hashPassword(password);

	int userId = dbManager->createUser(login, hashedPassword, fullName);

	Wallet wallet;
	wallet.setUserId(userId);
	wallet.generateCardNumber("CARD");
	dbManager->createWallet(userId, wallet.getCardNumber());

	return userId;
}

User* BankSystem::loginUser(const string& login, const string& password) {
	string hashedPassword = User::hashPassword(password);

	User* user = dbManager->authenticateUser(login, hashedPassword);
	if (user) {
		dbManager->loadUserProducts(user);
	}
	return user;
}

void BankSystem::fundWallet(int userId, double amount) {
	if (amount <= 0) {
		throw invalid_argument("Amount must be positive");
	}

	shared_ptr<Wallet> wallet = dbManager->getWalletForUser(userId);
	if (!wallet) {
		throw runtime_error("Wallet not found for this user");
	}

	dbManager->fundWallet(wallet->getId(), amount);

	dbManager->createTransaction(0, wallet->getId(), 0, 0, 0, 0, "deposit", amount, "Wallet funding");
}

int BankSystem::createAccount(int userId, const string& accountType, double initialAmount, double interestRate, int durationMonths) {
	if (initialAmount <= 0) {
		throw invalid_argument("Initial amount must be positive");
	}
	if (interestRate < 0) {
		throw invalid_argument("Interest rate cannot be negative");
	}
	if (durationMonths <= 0) {
		throw invalid_argument("Duration must be positive");
	}

	shared_ptr<Wallet> wallet = dbManager->getWalletForUser(userId);
	if (!wallet) {
		throw runtime_error("Wallet not found for this user");
	}

	if (wallet->getBalance() < initialAmount) {
		throw runtime_error("Insufficient funds in wallet");
	}

	dbManager->updateWalletBalance(wallet->getId(), initialAmount, false);

	int accountId = dbManager->createAccount(userId, accountType, initialAmount, interestRate, durationMonths);

	string description = "Opening " + accountType + " account";
	dbManager->createTransaction(wallet->getId(), 0, 0, accountId, 0, 0, "account_opening", initialAmount, description);

	refreshUserProducts(dbManager->getUserById(userId));

	return accountId;
}

int BankSystem::createDeposit(int userId, double amount, double interestRate, int durationMonths) {
	if (amount <= 0) {
		throw invalid_argument("Amount must be positive");
	}
	if (interestRate < 0) {
		throw invalid_argument("Interest rate cannot be negative");
	}
	if (durationMonths <= 0) {
		throw invalid_argument("Duration must be positive");
	}

	shared_ptr<Wallet> wallet = dbManager->getWalletForUser(userId);
	if (!wallet) {
		throw runtime_error("Wallet not found for this user");
	}

	if (wallet->getBalance() < amount) {
		throw runtime_error("Insufficient funds in wallet");
	}

	dbManager->updateWalletBalance(wallet->getId(), amount, false);

	int depositId = dbManager->createDeposit(userId, amount, interestRate, durationMonths);

	string description = "Creating deposit with " + to_string(interestRate) + "% interest rate for " +
		to_string(durationMonths) + " months";
	dbManager->createTransaction(wallet->getId(), 0, 0, 0, 0, depositId, "deposit_creation", amount, description);

	refreshUserProducts(dbManager->getUserById(userId));

	return depositId;
}

void BankSystem::refreshUserProducts(User* user) {
	if (!user) return;

	user->getProducts().clear();

	dbManager->loadUserProducts(user);
}

void BankSystem::closeAccount(int userId, int accountId) {
	User* user = dbManager->getUserById(userId);
	if (!user) {
		delete user;
		throw runtime_error("User not found");
	}
	delete user;

	dbManager->closeAccount(accountId);
}

void BankSystem::closeDeposit(int userId, int depositId) {
	User* user = dbManager->getUserById(userId);
	if (!user) {
		delete user;
		throw runtime_error("User not found");
	}
	delete user;

	dbManager->closeDeposit(depositId);
}

void BankSystem::transferFunds(int senderUserId, int receiverUserId, double amount, const string& description) {
	if (amount <= 0) {
		throw invalid_argument("Amount must be positive");
	}

	shared_ptr<Wallet> senderWallet = dbManager->getWalletForUser(senderUserId);
	shared_ptr<Wallet> receiverWallet = dbManager->getWalletForUser(receiverUserId);

	if (!senderWallet) {
		throw runtime_error("Sender wallet not found");
	}

	if (!receiverWallet) {
		throw runtime_error("Receiver wallet not found");
	}

	if (senderWallet->getBalance() < amount) {
		throw runtime_error("Insufficient funds in sender wallet");
	}

	dbManager->transferFunds(senderWallet->getId(), receiverWallet->getId(), amount, description);
}

void BankSystem::displayLoginMenu() const {
	cout << "\n==== BANK SYSTEM ====" << endl;
	cout << "1. Register" << endl;
	cout << "2. Login" << endl;
	cout << "3. Admin Login" << endl;
	cout << "4. Exit" << endl;
	cout << "====================" << endl;
	cout << "Enter your choice: ";
}

void BankSystem::displayUserMenu() const {
	cout << "\n==== USER MENU ====" << endl;
	cout << "1. Display My Products" << endl;
	cout << "2. Fund Wallet" << endl;
	cout << "3. Create Account" << endl;
	cout << "4. Create Deposit" << endl;
	cout << "5. Close Account" << endl;
	cout << "6. Close Deposit" << endl;
	cout << "7. View Transactions" << endl;
	cout << "8. Transfer Funds" << endl;
	cout << "9. View Total Balance" << endl;
	cout << "10. Log Out" << endl;
	cout << "==================" << endl;
	cout << "Enter your choice: ";
}

void BankSystem::displayAdminMenu() const {
	cout << "\n==== ADMIN MENU ====" << endl;
	cout << "1. Display All Users" << endl;
	cout << "2. Display All Transactions" << endl;
	cout << "3. View System Statistics" << endl;
	cout << "4. Log Out" << endl;
	cout << "===================" << endl;
	cout << "Enter your choice: ";
}