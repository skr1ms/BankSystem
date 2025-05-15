#pragma once
#include "common.h"
#include "FinancialProduct.h"

class Account : public FinancialProduct {
private:
	string accountType;
	double interestRate;
	int durationMonths;

public:
	Account(int _id = 0, int _userId = 0, double _balance = 0.0, bool _isActive = true, string _createdAt = "", string _accountType = "", double _interestRate = 0.0, int _durationMonths = 0) : 
		FinancialProduct(_id, _userId, _balance, _isActive, _createdAt), accountType(_accountType), interestRate(_interestRate), durationMonths(_durationMonths) {}

	string getAccountType() const { return accountType; }

	double getInterestRate() const { return interestRate; }

	int getDurationMonths() const { return durationMonths; }

	void setAccountType(string _accountType) { accountType = _accountType; }

	void setInterestRate(double _interestRate) { interestRate = _interestRate; }

	void setDurationMonths(int _durationMonths) { durationMonths = _durationMonths; }

	bool withdrawFunds(double amount) override {
		throw runtime_error("Cannot withdraw funds from a fixed account before maturity");
		return false;
	}

	void displayInfo() const override {
		cout << "Account Info:" << endl;
		cout << "ID: " << id << endl;
		cout << "Account Type: " << accountType << endl;
		cout << "Balance: " << fixed << setprecision(2) << balance << endl;
		cout << "Interest Rate: " << interestRate << "%" << endl;
		cout << "Duration: " << durationMonths << " months" << endl;
		cout << "Status: " << (isActive ? "Active" : "Inactive") << endl;
		cout << "Created At: " << createdAt << endl;
	}

	double calculateExpectedProfit() const {
		return balance * (interestRate / 100.0) * (static_cast<double>(durationMonths) / 12.0);
	}
};