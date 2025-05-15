#pragma once
#include "common.h"

class Deposit : public FinancialProduct {
private:
	double interestRate;
	int durationMonths;
	string closedAt;

public:
	Deposit(int _id = 0, int _userId = 0, double _balance = 0.0, bool _isActive = true, string _createdAt = "", double _interestRate = 0.0, int _durationMonths = 0, string _closedAt = "") : 
		FinancialProduct(_id, _userId, _balance, _isActive, _createdAt), interestRate(_interestRate), durationMonths(_durationMonths), closedAt(_closedAt) {}

	double getInterestRate() const { return interestRate; }

	int getDurationMonths() const { return durationMonths; }

	string getClosedAt() const { return closedAt; }

	void setInterestRate(double _interestRate) { interestRate = _interestRate; }

	void setDurationMonths(int _durationMonths) { durationMonths = _durationMonths; }

	void setClosedAt(string _closedAt) { closedAt = _closedAt; }

	void displayInfo() const override {
		cout << "Deposit Info:" << endl;
		cout << "ID: " << id << endl;
		cout << "Balance: " << fixed << setprecision(2) << balance << endl;
		cout << "Interest Rate: " << interestRate << "%" << endl;
		cout << "Duration: " << durationMonths << " months" << endl;
		cout << "Status: " << (isActive ? "Active" : "Inactive") << endl;
		cout << "Created At: " << createdAt << endl;
		if (!isActive && !closedAt.empty()) {
			cout << "Closed At: " << closedAt << endl;
		}
	}

	double calculateExpectedProfit() const {
		return balance * (interestRate / 100.0) * (static_cast<double>(durationMonths) / 12.0);
	}

	double calculateActualProfit(const string& currentDate) const {
		return calculateExpectedProfit();
	}
};