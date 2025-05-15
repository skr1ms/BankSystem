#include "DatabaseManager.h"

DatabaseManager::DatabaseManager(const string& connString) : connectionString(connString), conn(nullptr) {
	try {
		conn = new connection(connectionString);
		if (!conn->is_open()) {
			throw runtime_error("Failed to open database connection");
		}
	}
	catch (const exception& e) {
		cerr << "Error connecting to database: " << e.what() << endl;
		if (conn) {
			delete conn;
			conn = nullptr;
		}
		throw;
	}
}

string DatabaseManager::getCurrentTimestamp() const {
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

bool DatabaseManager::userExists(const string& login) {
	try {
		work txn(*conn);
		result r = txn.exec("SELECT COUNT(*) FROM users WHERE login = " + txn.quote(login));
		txn.commit();
		return r[0][0].as<int>() > 0;
	}
	catch (const exception& e) {
		cerr << "Database error checking user existence: " << e.what() << endl;
		throw;
	}
}

int DatabaseManager::createUser(const string& login, const string& password, const string& fullName) {
	try {
		work txn(*conn);
		result r = txn.exec("INSERT INTO users (login, password, full_name) VALUES (" + txn.quote(login) + ", " + txn.quote(password) + ", " + txn.quote(fullName) + ") RETURNING id");
		int userId = r[0][0].as<int>();
		txn.commit();
		return userId;
	}
	catch (const pqxx::sql_error& e) {
		if (string(e.what()).find("duplicate key") != string::npos) {
			throw runtime_error("A user with this login already exists");
		}
		cerr << "Database error creating user: " << e.what() << endl;
		throw;
	}
	catch (const exception& e) {
		cerr << "Error creating user: " << e.what() << endl;
		throw;
	}
}

int DatabaseManager::createWallet(int userId, const string& cardNumber) {
	try {
		work txn(*conn);
		result r = txn.exec("INSERT INTO wallets (user_id, card_number) VALUES (" + to_string(userId) + ", " + txn.quote(cardNumber) + ") RETURNING id");
		int walletId = r[0][0].as<int>();
		txn.commit();
		return walletId;
	}
	catch (const exception& e) {
		cerr << "Error creating wallet: " << e.what() << endl;
		throw;
	}
}

User* DatabaseManager::authenticateUser(const string& login, const string& hashedPassword) {
	try {
		work txn(*conn);
		result r = txn.exec("SELECT id, login, password, full_name FROM users WHERE login = " + txn.quote(login) + " AND password = " + txn.quote(hashedPassword));
		txn.commit();

		if (r.size() == 0) {
			return nullptr;
		}

		int id = r[0][0].as<int>();
		string dbLogin = r[0][1].as<string>();
		string dbPassword = r[0][2].as<string>();
		string dbFullName = r[0][3].as<string>();

		User* user = new User(id, dbLogin, dbPassword, dbFullName);
		return user;
	}
	catch (const exception& e) {
		cerr << "Error authenticating user: " << e.what() << endl;
		throw;
	}
}

void DatabaseManager::loadUserProducts(User* user) {
	if (!user) return;

	try {
		work txnWallet(*conn);
		result rWallet = txnWallet.exec("SELECT id, balance, card_number, is_active, updated_at FROM wallets WHERE user_id = " + to_string(user->getId()));
		txnWallet.commit();

		for (size_t i = 0; i < rWallet.size(); i++) {
			int id = rWallet[i][0].as<int>();
			double balance = rWallet[i][1].as<double>();
			string cardNumber = rWallet[i][2].as<string>();
			bool isActive = rWallet[i][3].as<bool>();
			string updatedAt = rWallet[i][4].as<string>();

			shared_ptr<Wallet> wallet = make_shared<Wallet>(
				id, user->getId(), balance, isActive, updatedAt, cardNumber
			);
			user->addProduct(wallet);
		}

		work txnAccount(*conn);
		result rAccount = txnAccount.exec("SELECT id, account_type, balance, interest_rate, duration_months, is_active, created_at FROM accounts WHERE user_id = " + to_string(user->getId()));
		txnAccount.commit();

		for (size_t i = 0; i < rAccount.size(); i++) {
			int id = rAccount[i][0].as<int>();
			string accountType = rAccount[i][1].as<string>();
			double balance = rAccount[i][2].as<double>();
			double interestRate = rAccount[i][3].as<double>();
			int durationMonths = rAccount[i][4].as<int>();
			bool isActive = rAccount[i][5].as<bool>();
			string createdAt = rAccount[i][6].as<string>();

			shared_ptr<Account> account = make_shared<Account>(
				id, user->getId(), balance, isActive, createdAt,
				accountType, interestRate, durationMonths
			);
			user->addProduct(account);
		}

		work txnDeposit(*conn);
		result rDeposit = txnDeposit.exec("SELECT id, amount, interest_rate, duration_months, is_active, opened_at, closed_at FROM deposits WHERE user_id = " + to_string(user->getId()));
		txnDeposit.commit();

		for (size_t i = 0; i < rDeposit.size(); i++) {
			int id = rDeposit[i][0].as<int>();
			double amount = rDeposit[i][1].as<double>();
			double interestRate = rDeposit[i][2].as<double>();
			int durationMonths = rDeposit[i][3].as<int>();
			bool isActive = rDeposit[i][4].as<bool>();
			string openedAt = rDeposit[i][5].as<string>();
			string closedAt = rDeposit[i][6].is_null() ? "" : rDeposit[i][6].as<string>();

			shared_ptr<Deposit> deposit = make_shared<Deposit>(
				id, user->getId(), amount, isActive, openedAt,
				interestRate, durationMonths, closedAt
			);
			user->addProduct(deposit);
		}
	}
	catch (const exception& e) {
		cerr << "Error loading user products: " << e.what() << endl;
		throw;
	}
}

void DatabaseManager::fundWallet(int walletId, double amount) {
	try {
		work txn(*conn);
		txn.exec("UPDATE wallets SET balance = balance + " + to_string(amount) + ", updated_at = NOW() WHERE id = " + to_string(walletId));
		txn.commit();
	}
	catch (const exception& e) {
		cerr << "Error funding wallet: " << e.what() << endl;
		throw;
	}
}

shared_ptr<Wallet> DatabaseManager::getWalletForUser(int userId) {
	try {
		work txn(*conn);
		result r = txn.exec("SELECT id, balance, card_number, is_active, updated_at FROM wallets WHERE user_id = " + to_string(userId));
		txn.commit();

		if (r.size() == 0) {
			return nullptr;
		}

		int id = r[0][0].as<int>();
		double balance = r[0][1].as<double>();
		string cardNumber = r[0][2].as<string>();
		bool isActive = r[0][3].as<bool>();
		string updatedAt = r[0][4].as<string>();

		return make_shared<Wallet>(id, userId, balance, isActive, updatedAt, cardNumber);
	}
	catch (const exception& e) {
		cerr << "Error getting wallet: " << e.what() << endl;
		throw;
	}
}

void DatabaseManager::updateWalletBalance(int walletId, double amount, bool isAdd) {
	try {
		work txn(*conn);
		string op = isAdd ? "+" : "-";
		txn.exec("UPDATE wallets SET balance = balance " + op + " " + to_string(amount) + ", updated_at = NOW() WHERE id = " + to_string(walletId));
		txn.commit();
	}
	catch (const exception& e) {
		cerr << "Error updating wallet balance: " << e.what() << endl;
		throw;
	}
}

int DatabaseManager::createAccount(int userId, const string& accountType, double initialAmount, double interestRate, int durationMonths) {
	try {
		work txn(*conn);
		result r = txn.exec("INSERT INTO accounts (user_id, account_type, balance, interest_rate, duration_months) VALUES (" + to_string(userId) + ", " + txn.quote(accountType) + ", " + to_string(initialAmount) + ", " + to_string(interestRate) + ", " + to_string(durationMonths) + ") RETURNING id");
		int accountId = r[0][0].as<int>();
		txn.commit();
		return accountId;
	}
	catch (const exception& e) {
		cerr << "Error creating account: " << e.what() << endl;
		throw;
	}
}

int DatabaseManager::createDeposit(int userId, double amount, double interestRate, int durationMonths) {
	try {
		work txn(*conn);
		result r = txn.exec("INSERT INTO deposits (user_id, amount, interest_rate, duration_months) VALUES (" + to_string(userId) + ", " + to_string(amount) + ", " + to_string(interestRate) + ", " + to_string(durationMonths) + ") RETURNING id");
		int depositId = r[0][0].as<int>();
		txn.commit();
		return depositId;
	}
	catch (const exception& e) {
		cerr << "Error creating deposit: " << e.what() << endl;
		throw;
	}
}

int DatabaseManager::createTransaction(int senderWalletId, int receiverWalletId, int senderAccountId, int receiverAccountId, int senderDepositId, int receiverDepositId, const string& type, double amount, const string& description) {
	try {
		work txn(*conn);

		string query = "INSERT INTO transactions (";
		string values = "VALUES (";

		if (senderWalletId > 0) {
			query += "sender_wallet_id, ";
			values += to_string(senderWalletId) + ", ";
		}

		if (receiverWalletId > 0) {
			query += "receiver_wallet_id, ";
			values += to_string(receiverWalletId) + ", ";
		}

		if (senderAccountId > 0) {
			query += "sender_account_id, ";
			values += to_string(senderAccountId) + ", ";
		}

		if (receiverAccountId > 0) {
			query += "receiver_account_id, ";
			values += to_string(receiverAccountId) + ", ";
		}

		if (senderDepositId > 0) {
			query += "sender_deposit_id, ";
			values += to_string(senderDepositId) + ", ";
		}

		if (receiverDepositId > 0) {
			query += "receiver_deposit_id, ";
			values += to_string(receiverDepositId) + ", ";
		}

		query += "type, amount, description) ";
		values += txn.quote(type) + ", " + to_string(amount) + ", " + txn.quote(description) + ") RETURNING id";

		result r = txn.exec(query + values);
		int transactionId = r[0][0].as<int>();
		txn.commit();
		return transactionId;
	}
	catch (const exception& e) {
		cerr << "Error creating transaction: " << e.what() << endl;
		throw;
	}
}

void DatabaseManager::closeAccount(int accountId) {
	try {
		work txn(*conn);

		result accountCheck = txn.exec("SELECT balance FROM accounts WHERE id = " + to_string(accountId) + " AND is_active = TRUE");

		if (accountCheck.size() == 0) {
			txn.abort();
			throw runtime_error("Account not found or is already closed");
		}

		double balance = accountCheck[0][0].as<double>();

		result userIdQuery = txn.exec("SELECT user_id FROM accounts WHERE id = " + to_string(accountId));

		if (userIdQuery.size() == 0) {
			txn.abort();
			throw runtime_error("Account not found");
		}

		int userId = userIdQuery[0][0].as<int>();

		result walletQuery = txn.exec("SELECT id FROM wallets WHERE user_id = " + to_string(userId) + " AND is_active = TRUE LIMIT 1");

		if (walletQuery.size() == 0) {
			txn.abort();
			throw runtime_error("No active wallet found for this user");
		}

		int walletId = walletQuery[0][0].as<int>();

		txn.exec("UPDATE accounts SET is_active = FALSE, closed_at = NOW() WHERE id = " + to_string(accountId));

		txn.exec("UPDATE wallets SET balance = balance + " + to_string(balance) + ", updated_at = NOW() WHERE id = " + to_string(walletId));

		result accountInfo = txn.exec("SELECT account_type FROM accounts WHERE id = " + to_string(accountId));

		string accountType = accountInfo[0][0].as<string>();
		string description = "Closing account: " + accountType;

		txn.exec("INSERT INTO transactions (sender_account_id, receiver_wallet_id, type, amount, description) VALUES (" + to_string(accountId) + ", " + to_string(walletId) + ", " + txn.quote("account_closing") + ", " + to_string(balance) + ", " + txn.quote(description) + ")");

		txn.commit();
	}
	catch (const exception& e) {
		cerr << "Error closing account: " << e.what() << endl;
		throw;
	}
}

void DatabaseManager::closeDeposit(int depositId) {
	try {
		work txn(*conn);

		result depositCheck = txn.exec("SELECT amount, interest_rate, duration_months, opened_at FROM deposits WHERE id = " + to_string(depositId) + " AND is_active = TRUE");

		if (depositCheck.size() == 0) {
			txn.abort();
			throw runtime_error("Deposit not found or is already closed");
		}

		double amount = depositCheck[0][0].as<double>();
		double interestRate = depositCheck[0][1].as<double>();
		int durationMonths = depositCheck[0][2].as<int>();
		string openedAt = depositCheck[0][3].as<string>();

		double profit = amount * (interestRate / 100.0) * (static_cast<double>(durationMonths) / 12.0);
		double totalAmount = amount + profit;

		result userIdQuery = txn.exec("SELECT user_id FROM deposits WHERE id = " + to_string(depositId));

		if (userIdQuery.size() == 0) {
			txn.abort();
			throw runtime_error("Deposit not found");
		}

		int userId = userIdQuery[0][0].as<int>();

		result walletQuery = txn.exec("SELECT id FROM wallets WHERE user_id = " + to_string(userId) + " AND is_active = TRUE LIMIT 1");

		if (walletQuery.size() == 0) {
			txn.abort();
			throw runtime_error("No active wallet found for this user");
		}

		int walletId = walletQuery[0][0].as<int>();

		txn.exec("UPDATE deposits SET is_active = FALSE, closed_at = NOW() WHERE id = " + to_string(depositId));

		txn.exec("UPDATE wallets SET balance = balance + " + to_string(totalAmount) + ", updated_at = NOW() WHERE id = " + to_string(walletId));

		string description = "Closing deposit ID: " + to_string(depositId) + " (Principal: " + to_string(amount) + ", Interest: " + to_string(profit) + ")";

		txn.exec("INSERT INTO transactions (sender_deposit_id, receiver_wallet_id, type, amount, description) VALUES (" + to_string(depositId) + ", " + to_string(walletId) + ", " + txn.quote("deposit_closing") + ", " + to_string(totalAmount) + ", " + txn.quote(description) + ")");

		txn.commit();
	}
	catch (const exception& e) {
		cerr << "Error closing deposit: " << e.what() << endl;
		throw;
	}
}

User* DatabaseManager::getUserById(int userId) {
	try {
		work txn(*conn);
		result r = txn.exec("SELECT id, login, password, full_name FROM users WHERE id = " + to_string(userId));
		txn.commit();

		if (r.size() == 0) {
			return nullptr;
		}

		int id = r[0][0].as<int>();
		string login = r[0][1].as<string>();
		string password = r[0][2].as<string>();
		string fullName = r[0][3].as<string>();

		User* user = new User(id, login, password, fullName);
		loadUserProducts(user);
		return user;
	}
	catch (const exception& e) {
		cerr << "Error getting user by ID: " << e.what() << endl;
		throw;
	}
}

vector<TransactionData> DatabaseManager::getUserTransactions(int userId) {
	vector<TransactionData> transactions;
	try {
		work txn(*conn);

		result walletResult = txn.exec("SELECT id FROM wallets WHERE user_id = " + to_string(userId));

		result accountResult = txn.exec("SELECT id FROM accounts WHERE user_id = " + to_string(userId));

		result depositResult = txn.exec("SELECT id FROM deposits WHERE user_id = " + to_string(userId));

		string walletIds = "";
		for (size_t i = 0; i < walletResult.size(); i++) {
			if (i > 0) walletIds += ", ";
			walletIds += to_string(walletResult[i][0].as<int>());
		}

		string accountIds = "";
		for (size_t i = 0; i < accountResult.size(); i++) {
			if (i > 0) accountIds += ", ";
			accountIds += to_string(accountResult[i][0].as<int>());
		}

		string depositIds = "";
		for (size_t i = 0; i < depositResult.size(); i++) {
			if (i > 0) depositIds += ", ";
			depositIds += to_string(depositResult[i][0].as<int>());
		}

		string query = "SELECT id, type, amount, description, created_at FROM transactions WHERE 1=0 ";

		if (!walletIds.empty()) {
			query += "OR sender_wallet_id IN (" + walletIds + ") ";
			query += "OR receiver_wallet_id IN (" + walletIds + ") ";
		}

		if (!accountIds.empty()) {
			query += "OR sender_account_id IN (" + accountIds + ") ";
			query += "OR receiver_account_id IN (" + accountIds + ") ";
		}

		if (!depositIds.empty()) {
			query += "OR sender_deposit_id IN (" + depositIds + ") ";
			query += "OR receiver_deposit_id IN (" + depositIds + ") ";
		}

		query += "ORDER BY created_at DESC";

		if (walletIds.empty() && accountIds.empty() && depositIds.empty()) {
			return transactions;
		}

		result r = txn.exec(query);
		txn.commit();

		for (size_t i = 0; i < r.size(); i++) {
			int id = r[i][0].as<int>();
			string type = r[i][1].as<string>();
			double amount = r[i][2].as<double>();
			string description = r[i][3].as<string>();
			string createdAt = r[i][4].as<string>();

			transactions.push_back(TransactionData(id, type, amount, description, createdAt));
		}
	}
	catch (const exception& e) {
		cerr << "Error getting user transactions: " << e.what() << endl;
		throw;
	}

	return transactions;
}

void DatabaseManager::transferFunds(int senderWalletId, int receiverWalletId, double amount, const string& description) {
	try {
		if (amount <= 0) {
			throw invalid_argument("Amount must be positive");
		}

		work txn(*conn);

		result senderCheck = txn.exec("SELECT balance FROM wallets WHERE id = " + to_string(senderWalletId) + " AND is_active = TRUE");

		if (senderCheck.size() == 0) {
			txn.abort();
			throw runtime_error("Sender wallet not found or is inactive");
		}

		double senderBalance = senderCheck[0][0].as<double>();

		if (senderBalance < amount) {
			txn.abort();
			throw runtime_error("Insufficient funds in sender wallet");
		}

		result receiverCheck = txn.exec("SELECT id FROM wallets WHERE id = " + to_string(receiverWalletId) + " AND is_active = TRUE");

		if (receiverCheck.size() == 0) {
			txn.abort();
			throw runtime_error("Receiver wallet not found or is inactive");
		}

		txn.exec("UPDATE wallets SET balance = balance - " + to_string(amount) + ", updated_at = NOW() WHERE id = " + to_string(senderWalletId));

		txn.exec("UPDATE wallets SET balance = balance + " + to_string(amount) + ", updated_at = NOW() WHERE id = " + to_string(receiverWalletId));

		txn.exec("INSERT INTO transactions (sender_wallet_id, receiver_wallet_id, type, amount, description) VALUES (" + to_string(senderWalletId) + ", " + to_string(receiverWalletId) + ", " + txn.quote("transfer") + ", " + to_string(amount) + ", " + txn.quote(description) + ")");

		txn.commit();
	}
	catch (const exception& e) {
		cerr << "Error transferring funds: " << e.what() << endl;
		throw;
	}
}

double DatabaseManager::getTotalUserBalance(int userId) {
	try {
		work txn(*conn);

		result walletResult = txn.exec("SELECT COALESCE(SUM(balance), 0) FROM wallets WHERE user_id = " + to_string(userId) + " AND is_active = TRUE");

		double walletBalance = walletResult[0][0].as<double>();

		result accountResult = txn.exec("SELECT COALESCE(SUM(balance), 0) FROM accounts WHERE user_id = " + to_string(userId) + " AND is_active = TRUE");

		double accountBalance = accountResult[0][0].as<double>();

		result depositResult = txn.exec("SELECT COALESCE(SUM(amount), 0) FROM deposits WHERE user_id = " + to_string(userId) + " AND is_active = TRUE");

		double depositAmount = depositResult[0][0].as<double>();

		txn.commit();

		return walletBalance + accountBalance + depositAmount;
	}
	catch (const exception& e) {
		cerr << "Error getting total user balance: " << e.what() << endl;
		throw;
	}
}

bool DatabaseManager::isAdminUser(int userId) {
	try {
		work txn(*conn);
		result r = txn.exec("SELECT is_admin FROM users WHERE id = " + to_string(userId));
		txn.commit();

		if (r.size() == 0) {
			return false;
		}

		return r[0][0].as<bool>();
	}
	catch (const exception& e) {
		cerr << "Error checking admin status: " << e.what() << endl;
		throw;
	}
}

vector<User*> DatabaseManager::getAllUsers() {
	vector<User*> users;
	try {
		work txn(*conn);

		result r_users = txn.exec("SELECT id, login, password, full_name, is_admin FROM users ORDER BY id");

		for (const auto& row : r_users) {
			int id = row["id"].as<int>();
			string login = row["login"].as<string>();
			string password = row["password"].as<string>();
			string fullName = row["full_name"].as<string>();
			bool isAdmin = row["is_admin"].as<bool>();

			User* user = new User(id, login, password, fullName);

			loadUserProductsInline(txn, user);

			users.push_back(user);
		}

		txn.commit();

		return users;
	}
	catch (const exception& e) {
		cerr << "Error getting all users: " << e.what() << endl;
		throw;
	}
}

void DatabaseManager::loadUserProductsInline(transaction_base& txn, User* user) {
	if (!user) return;

	result rWallet = txn.exec("SELECT id, balance, card_number, is_active, updated_at FROM wallets WHERE user_id = " + to_string(user->getId()));
	for (size_t i = 0; i < rWallet.size(); i++) {
		int id = rWallet[i][0].as<int>();
		double balance = rWallet[i][1].as<double>();
		string cardNumber = rWallet[i][2].as<string>();
		bool isActive = rWallet[i][3].as<bool>();
		string updatedAt = rWallet[i][4].as<string>();
		shared_ptr<Wallet> wallet = make_shared<Wallet>(
			id, user->getId(), balance, isActive, updatedAt, cardNumber
		);
		user->addProduct(wallet);
	}

	result rAccount = txn.exec("SELECT id, account_type, balance, interest_rate, duration_months, is_active, created_at FROM accounts WHERE user_id = " + to_string(user->getId()));
	for (size_t i = 0; i < rAccount.size(); i++) {
		int id = rAccount[i][0].as<int>();
		string accountType = rAccount[i][1].as<string>();
		double balance = rAccount[i][2].as<double>();
		double interestRate = rAccount[i][3].as<double>();
		int durationMonths = rAccount[i][4].as<int>();
		bool isActive = rAccount[i][5].as<bool>();
		string createdAt = rAccount[i][6].as<string>();
		shared_ptr<Account> account = make_shared<Account>(
			id, user->getId(), balance, isActive, createdAt,
			accountType, interestRate, durationMonths
		);
		user->addProduct(account);
	}

	result rDeposit = txn.exec("SELECT id, amount, interest_rate, duration_months, is_active, opened_at, closed_at FROM deposits WHERE user_id = " + to_string(user->getId()));
	for (size_t i = 0; i < rDeposit.size(); i++) {
		int id = rDeposit[i][0].as<int>();
		double amount = rDeposit[i][1].as<double>();
		double interestRate = rDeposit[i][2].as<double>();
		int durationMonths = rDeposit[i][3].as<int>();
		bool isActive = rDeposit[i][4].as<bool>();
		string openedAt = rDeposit[i][5].as<string>();
		string closedAt = rDeposit[i][6].is_null() ? "" : rDeposit[i][6].as<string>();
		shared_ptr<Deposit> deposit = make_shared<Deposit>(
			id, user->getId(), amount, isActive, openedAt,
			interestRate, durationMonths, closedAt
		);
		user->addProduct(deposit);
	}
}

vector<TransactionData> DatabaseManager::getAllTransactions() {
	vector<TransactionData> transactions;
	try {
		work txn(*conn);
		result r = txn.exec("SELECT id, type, amount, description, created_at FROM transactions ORDER BY created_at DESC");
		for (size_t i = 0; i < r.size(); ++i) {
			int id = r[i][0].as<int>();
			string type = r[i][1].as<string>();
			double amount = r[i][2].as<double>();
			string description = r[i][3].as<string>();
			string createdAt = r[i][4].as<string>();
			transactions.emplace_back(id, type, amount, description, createdAt);
		}
		return transactions;
	}
	catch (const exception& e) {
		cerr << "Error getting all transactions: " << e.what() << endl;
		throw;
	}
}

int DatabaseManager::getTotalUsers() {
	try {
		work txn(*conn);
		result r = txn.exec("SELECT COUNT(*) FROM users");
		txn.commit();
		return r[0][0].as<int>();
	}
	catch (const exception& e) {
		cerr << "Error getting total users: " << e.what() << endl;
		throw;
	}
}

int DatabaseManager::getTotalActiveProducts() {
	try {
		work txn(*conn);

		result walletResult = txn.exec("SELECT COUNT(*) FROM wallets WHERE is_active = TRUE");

		int walletCount = walletResult[0][0].as<int>();

		result accountResult = txn.exec("SELECT COUNT(*) FROM accounts WHERE is_active = TRUE");

		int accountCount = accountResult[0][0].as<int>();

		result depositResult = txn.exec("SELECT COUNT(*) FROM deposits WHERE is_active = TRUE");

		int depositCount = depositResult[0][0].as<int>();

		txn.commit();

		return walletCount + accountCount + depositCount;
	}
	catch (const exception& e) {
		cerr << "Error getting total active products: " << e.what() << endl;
		throw;
	}
}

double DatabaseManager::getTotalTransactionVolume() {
	try {
		work txn(*conn);
		result r = txn.exec("SELECT COALESCE(SUM(amount), 0) FROM transactions");
		txn.commit();
		return r[0][0].as<double>();
	}
	catch (const exception& e) {
		cerr << "Error getting total transaction volume: " << e.what() << endl;
		throw;
	}
}

void DatabaseManager::getSystemStatistics(int& totalUsers, int& totalProducts, double& totalVolume) {
	totalUsers = getTotalUsers();
	totalProducts = getTotalActiveProducts();
	totalVolume = getTotalTransactionVolume();
}

DatabaseManager::~DatabaseManager() {
	if (conn) {
		if (conn->is_open()) {
			conn->close();
		}
		delete conn;
	}
}