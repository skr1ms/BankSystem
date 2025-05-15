#pragma once
#include "common.h"
#include "FinancialProduct.h"

class Wallet : public FinancialProduct {
private:
	string cardNumber;

public:
	Wallet(int _id = 0, int _userId = 0, double _balance = 0.0,bool _isActive = true, string _createdAt = "", string _cardNumber = "") : 
		FinancialProduct(_id, _userId, _balance, _isActive, _createdAt), cardNumber(_cardNumber) {}

	string getCardNumber() const { return cardNumber; }

	void setCardNumber(string _cardNumber) { cardNumber = _cardNumber; }

	void displayInfo() const override {
		cout << "Wallet Info:" << endl;
		cout << "ID: " << id << endl;
		cout << "Card Number: " << cardNumber << endl;
		cout << "Balance: " << fixed << setprecision(2) << balance << endl;
		cout << "Status: " << (isActive ? "Active" : "Inactive") << endl;
		cout << "Created At: " << createdAt << endl;
	}

	string generateCardNumber() {
		cardNumber = "CARD-" + to_string(userId) + "-" + to_string(rand() % 10000);
		return cardNumber;
	}

	string generateCardNumber(string prefix) {
		cardNumber = prefix + "-" + to_string(userId) + "-" + to_string(rand() % 10000);
		return cardNumber;
	}
};