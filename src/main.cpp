#include "BankSystem.h"

int main() {
	srand(time(NULL));

	try {
		BankSystem bankSystem;

		int choice;
		bool running = true;

		while (running) {
			bankSystem.displayLoginMenu();
			cin >> choice;
			cin.ignore(numeric_limits<streamsize>::max(), '\n');

			switch (choice) {
			case 1: { // Register
				string login, password, fullName;

				cout << "Enter login: ";
				std::getline(cin, login);

				cout << "Enter password: ";
				std::getline(cin, password);

				cout << "Enter full name: ";
				std::getline(cin, fullName);

				try {
					int userId = bankSystem.registerUser(login, password, fullName);
					cout << "Registration successful! Your user ID is: " << userId << endl;
				}
				catch (const exception& e) {
					cout << "Registration failed: " << e.what() << endl;
				}
				break;
			}
			case 2: { // Login
				string login, password;

				cout << "Enter login: ";
				std::getline(cin, login);

				cout << "Enter password: ";
				std::getline(cin, password);

				User* user = bankSystem.loginUser(login, password);
				if (user) {
					cout << "Login successful! Welcome, " << user->getFullName() << endl;

					bool userLoggedIn = true;
					while (userLoggedIn) {
						bankSystem.displayUserMenu();
						int userChoice;
						cin >> userChoice;
						cin.ignore(numeric_limits<streamsize>::max(), '\n');

						switch (userChoice) {
						case 1: { // Display products
							user->displayAllProducts();
							break;
						}
						case 2: { // Fund wallet
							double amount;
							cout << "Enter amount to add to your wallet: ";
							cin >> amount;
							cin.ignore(numeric_limits<streamsize>::max(), '\n');

							try {
								bankSystem.fundWallet(user->getId(), amount);
								cout << "Wallet funded successfully!" << endl;
							}
							catch (const exception& e) {
								cout << "Failed to fund wallet: " << e.what() << endl;
							}
							break;
						}
						case 3: { // Create account
							string accountType;
							double initialAmount, interestRate;
							int durationMonths;
							cout << "Enter account type (e.g., Savings, Fixed): ";
							std::getline(cin, accountType);
							cout << "Enter initial amount: ";
							cin >> initialAmount;
							cout << "Enter interest rate (%): ";
							cin >> interestRate;
							cout << "Enter duration (months): ";
							cin >> durationMonths;
							cin.ignore(numeric_limits<streamsize>::max(), '\n');
							try {
								int accountId = bankSystem.createAccount(
									user->getId(), accountType, initialAmount, interestRate, durationMonths
								);
								cout << "Account created successfully! Account ID: " << accountId << endl;

								bankSystem.refreshUserProducts(user);

							}
							catch (const exception& e) {
								cout << "Failed to create account: " << e.what() << endl;
							}
							break;
						}
						case 4: { // Create deposit
							double amount, interestRate;
							int durationMonths;
							cout << "Enter deposit amount: ";
							cin >> amount;
							cout << "Enter interest rate (%): ";
							cin >> interestRate;
							cout << "Enter duration (months): ";
							cin >> durationMonths;
							cin.ignore(numeric_limits<streamsize>::max(), '\n');
							try {
								int depositId = bankSystem.createDeposit(
									user->getId(), amount, interestRate, durationMonths
								);
								cout << "Deposit created successfully! Deposit ID: " << depositId << endl;

								bankSystem.refreshUserProducts(user);

							}
							catch (const exception& e) {
								cout << "Failed to create deposit: " << e.what() << endl;
							}
							break;
						}
						case 5: { // Close account
							int accountId;
							cout << "Enter account ID to close: ";
							cin >> accountId;
							cin.ignore(numeric_limits<streamsize>::max(), '\n');
							try {
								bankSystem.closeAccount(user->getId(), accountId);
								cout << "Account closed successfully!" << endl;

								bankSystem.refreshUserProducts(user);

							}
							catch (const exception& e) {
								cout << "Failed to close account: " << e.what() << endl;
							}
							break;
						}
						case 6: { // Close deposit
							int depositId;
							cout << "Enter deposit ID to close: ";
							cin >> depositId;
							cin.ignore(numeric_limits<streamsize>::max(), '\n');
							try {
								bankSystem.closeDeposit(user->getId(), depositId);
								cout << "Deposit closed successfully!" << endl;

								bankSystem.refreshUserProducts(user);

							}
							catch (const exception& e) {
								cout << "Failed to close deposit: " << e.what() << endl;
							}
							break;
						}
						case 7: { // View transactions
							vector<TransactionData> transactions = bankSystem.getUserTransactions(user->getId());

							if (transactions.empty()) {
								cout << "No transactions found." << endl;
							}
							else {
								cout << "\n==== TRANSACTIONS ====" << endl;
								cout << setw(5) << "ID" << " | "
									<< setw(15) << "Type" << " | "
									<< setw(10) << "Amount" << " | "
									<< setw(50) << "Description" << " | "
									<< "Date" << endl;
								cout << string(100, '-') << endl;

								for (const auto& txn : transactions) {
									cout << setw(5) << txn.id << " | "
										<< setw(15) << txn.type << " | "
										<< setw(10) << fixed << setprecision(2) << txn.amount << " | "
										<< setw(50) << txn.description << " | "
										<< txn.createdAt << endl;
								}
							}
							break;
						}
						case 8: { // Transfer funds
							int receiverUserId;
							double amount;
							string description;

							cout << "Enter receiver user ID: ";
							cin >> receiverUserId;
							cin.ignore(numeric_limits<streamsize>::max(), '\n');

							cout << "Enter amount to transfer: ";
							cin >> amount;
							cin.ignore(numeric_limits<streamsize>::max(), '\n');

							cout << "Enter description: ";
							std::getline(cin, description);

							try {
								bankSystem.transferFunds(user->getId(), receiverUserId, amount, description);
								cout << "Funds transferred successfully!" << endl;
							}
							catch (const exception& e) {
								cout << "Failed to transfer funds: " << e.what() << endl;
							}
							break;
						}
						case 9: { // View total balance
							try {
								double totalBalance = bankSystem.getTotalUserBalance(user->getId());
								cout << "Your total balance across all products: " << fixed << setprecision(2) << totalBalance << endl;
							}
							catch (const exception& e) {
								cout << "Failed to get total balance: " << e.what() << endl;
							}
							break;
						}
						case 10: { // Log out
							cout << "Logging out..." << endl;
							userLoggedIn = false;
							break;
						}
						default:
							cout << "Invalid choice. Please try again." << endl;
						}
					}

					delete user;
				}
				else {
					cout << "Login failed. Invalid credentials." << endl;
				}
				break;
			}
			case 3: { // Admin login
				string login, password;
				cout << "Enter admin login: ";
				std::getline(cin, login);
				cout << "Enter admin password: ";
				std::getline(cin, password);

				User* adminUser = bankSystem.loginUser(login, password);
				if (adminUser && bankSystem.isAdmin(adminUser->getId())) {
					cout << "Admin login successful! Welcome, " << adminUser->getFullName() << endl;
					bool adminLoggedIn = true;

					while (adminLoggedIn) {
						bankSystem.displayAdminMenu();
						int adminChoice;
						cin >> adminChoice;
						cin.ignore(numeric_limits<streamsize>::max(), '\n');

						switch (adminChoice) {
						case 1: { // Display all users
							try {
								vector<User*> allUsers = bankSystem.getAllUsers();
								if (allUsers.empty()) {
									cout << "No users found in the system." << endl;
									break;
								}

								cout << "\n==== ALL USERS ====" << endl;
								cout << setw(5) << "ID" << " | "
									<< setw(20) << "Login" << " | "
									<< setw(30) << "Full Name" << " | "
									<< setw(10) << "Is Admin" << endl;
								cout << string(80, '-') << endl;

								for (User* user : allUsers) {
									cout << setw(5) << user->getId() << " | "
										<< setw(20) << user->getLogin() << " | "
										<< setw(30) << user->getFullName() << " | "
										<< setw(10) << (bankSystem.isAdmin(user->getId()) ? "Yes" : "No") << endl;
								}
								cout << "===================" << endl;

								// Освобождаем память
								for (auto user : allUsers) delete user;
							}
							catch (const exception& e) {
								cout << "Error fetching users: " << e.what() << endl;
							}
							break;
						}
						case 2: { // Display all transactions
							try {
								vector<TransactionData> allTransactions = bankSystem.getAllTransactions();
								if (allTransactions.empty()) {
									cout << "No transactions found in the system." << endl;
									break;
								}

								cout << "\n==== ALL TRANSACTIONS ====" << endl;
								cout << setw(5) << "ID" << " | "
									<< setw(15) << "Type" << " | "
									<< setw(10) << "Amount" << " | "
									<< setw(50) << "Description" << " | "
									<< "Date" << endl;
								cout << string(100, '-') << endl;

								for (const auto& txn : allTransactions) {
									cout << setw(5) << txn.id << " | "
										<< setw(15) << txn.type << " | "
										<< setw(10) << fixed << setprecision(2) << txn.amount << " | "
										<< setw(50) << txn.description << " | "
										<< txn.createdAt << endl;
								}
								cout << "==========================" << endl;
							}
							catch (const exception& e) {
								cout << "Error fetching transactions: " << e.what() << endl;
							}
							break;
						}
						case 3: { // View system statistics
							int totalUsers, totalProducts;
							double totalVolume;
							try {
								bankSystem.getSystemStatistics(totalUsers, totalProducts, totalVolume);
								cout << "\n==== SYSTEM STATISTICS ====" << endl;
								cout << "Total Users: " << totalUsers << endl;
								cout << "Total Active Products: " << totalProducts << endl;
								cout << "Total Transaction Volume: " << fixed << setprecision(2) << totalVolume << endl;
								cout << "===========================" << endl;
							}
							catch (const exception& e) {
								cout << "Failed to get system statistics: " << e.what() << endl;
							}
							break;
						}
						case 4: { // Log out
							cout << "Logging out of admin account..." << endl;
							adminLoggedIn = false;
							break;
						}
						default:
							cout << "Invalid choice. Please try again." << endl;
						}
					}
					delete adminUser;
				}
				else {
					if (adminUser) delete adminUser;
					cout << "Admin login failed. Invalid credentials or not an admin account." << endl;
				}
				break;
			}
			case 4: { // Exit
				cout << "Thank you for using our Bank System. Goodbye!" << endl;
				running = false;
				break;
			}
			default:
				cout << "Invalid choice. Please try again." << endl;
			}
		}
	}
	catch (const exception& e) {
		cerr << "Fatal error: " << e.what() << endl;
		return 1;
	}

	return 0;
}