#pragma once
#include "common.h"
#include "Wallet.h"

class User {
private:
	int id;
	string login;
	string password;
	string fullName;
	vector<shared_ptr<FinancialProduct>> products;

public:
	User(int _id = 0, string _login = "", string _password = "", string _fullName = "") : 
		id(_id), login(_login), password(_password), fullName(_fullName) {}

	int getId() const { return id; }

	string getLogin() const { return login; }

	string getPassword() const { return password; }

	string getFullName() const { return fullName; }

	vector<shared_ptr<FinancialProduct>>& getProducts() { return products; }

	void setId(int _id) { id = _id; }

	void setLogin(string _login) { login = _login; }

	void setPassword(string _password) { password = _password; }

	void setFullName(string _fullName) { fullName = _fullName; }

	void addProduct(shared_ptr<FinancialProduct> product) {
		products.push_back(product);
	}

	bool removeProduct(int productId) {
		auto it = find_if(products.begin(), products.end(),
			[productId](const shared_ptr<FinancialProduct>& p) {
				return p->getId() == productId;
			});
		if (it != products.end()) {
			products.erase(it);
			return true;
		}
		return false;
	}

	shared_ptr<FinancialProduct> getProduct(int productId) const {
		for (const auto& product : products) {
			if (product->getId() == productId) {
				return product;
			}
		}
		return nullptr;
	}

	void displayAllProducts() const {
		if (products.empty()) {
			cout << "No financial products found." << endl;
			return;
		}

		cout << "Financial Products for " << fullName << ":" << endl;
		for (const auto& product : products) {
			cout << "------------------------" << endl;
			product->displayInfo();
		}
		cout << "------------------------" << endl;
	}

	static string hashPassword(const string& password) {
		unsigned char hash[SHA256_DIGEST_LENGTH];
		SHA256(reinterpret_cast<const unsigned char*>(password.c_str()), password.size(), hash);
		stringstream ss;
		for (int i = 0; i < SHA256_DIGEST_LENGTH; i++)
			ss << hex << setw(2) << setfill('0') << static_cast<int>(hash[i]);
		return ss.str();
	}

	static bool validatePassword(const string& password) {
		if (password.length() < 8) {
			return false;
		}

		regex specialChar("[^A-Za-z0-9]");
		if (!regex_search(password, specialChar)) {
			return false;
		}

		return true;
	}
};