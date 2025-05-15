#pragma once
#include "common.h"

class Transaction {
private:
	int id;
	int senderProductId;
	int receiverProductId;
	string senderType; 
	string receiverType;
	string type;
	double amount;
	string description;
	string createdAt;

public:
	Transaction(int _id = 0, int _senderProductId = 0, int _receiverProductId = 0, string _senderType = "", string _receiverType = "", string _type = "", double _amount = 0.0, string _description = "", string _createdAt = "") : 
		id(_id), senderProductId(_senderProductId), receiverProductId(_receiverProductId), senderType(_senderType), receiverType(_receiverType), type(_type), amount(_amount), description(_description), createdAt(_createdAt) {}

	int getId() const { return id; }

	int getSenderProductId() const { return senderProductId; }

	int getReceiverProductId() const { return receiverProductId; }

	string getSenderType() const { return senderType; }

	string getReceiverType() const { return receiverType; }

	string getType() const { return type; }

	double getAmount() const { return amount; }

	string getDescription() const { return description; }

	string getCreatedAt() const { return createdAt; }

	void displayInfo() const {
		cout << "Transaction Info:" << endl;
		cout << "ID: " << id << endl;
		cout << "Type: " << type << endl;
		cout << "Amount: " << fixed << setprecision(2) << amount << endl;

		if (senderProductId != 0) {
			cout << "Sender (" << senderType << " ID): " << senderProductId << endl;
		}

		if (receiverProductId != 0) {
			cout << "Receiver (" << receiverType << " ID): " << receiverProductId << endl;
		}

		cout << "Description: " << description << endl;
		cout << "Date: " << createdAt << endl;
	}

	bool operator==(const Transaction& other) const {
		return id == other.id;
	}

	bool operator!=(const Transaction& other) const {
		return !(*this == other);
	}
};