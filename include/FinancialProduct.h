#pragma once
#include "common.h"

class FinancialProduct {
protected:
	int id;
	int userId;
	double balance;
	bool isActive;
	string createdAt;

public:
	FinancialProduct(int _id = 0, int _userId = 0, double _balance = 0.0,bool _isActive = true, string _createdAt = "") : 
		id(_id), userId(_userId), balance(_balance), isActive(_isActive), createdAt(_createdAt) {}

	virtual ~FinancialProduct() {}

	int getId() const { return id; }

	int getUserId() const { return userId; }

	double getBalance() const { return balance; }

	bool getIsActive() const { return isActive; }

	string getCreatedAt() const { return createdAt; }

	void setId(int _id) { id = _id; }

	void setUserId(int _userId) { userId = _userId; }

	void setBalance(double _balance) { balance = _balance; }

	void setIsActive(bool _isActive) { isActive = _isActive; }

	void setCreatedAt(string _createdAt) { createdAt = _createdAt; }

	virtual void addFunds(double amount) {
		if (amount <= 0) {
			throw invalid_argument("Amount must be positive");
		}
		balance += amount;
	}

	virtual bool withdrawFunds(double amount) {
		if (amount <= 0) {
			throw invalid_argument("Amount must be positive");
		}
		if (amount > balance) {
			return false;
		}
		balance -= amount;
		return true;
	}

	virtual void displayInfo() const = 0;

	bool operator==(const FinancialProduct& other) const {
		return id == other.id && userId == other.userId;
	}

	bool operator!=(const FinancialProduct& other) const {
		return !(*this == other);
	}

	FinancialProduct& operator+=(const double amount) {
		if (amount <= 0) {
			throw invalid_argument("Amount must be positive");
		}
		balance += amount;
		return *this;
	}

	FinancialProduct& operator-=(const double amount) {
		if (amount <= 0) {
			throw invalid_argument("Amount must be positive");
		}
		if (amount > balance) {
			throw runtime_error("Insufficient funds");
		}
		balance -= amount;
		return *this;
	}
};