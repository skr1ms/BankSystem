#pragma once

#include "common.h"

struct TransactionData {
    int id;
    string type;
    double amount;
    string description;
    string createdAt;

    TransactionData(int _id, string _type, double _amount, string _desc, string _date) : 
        id(_id), type(_type), amount(_amount), description(_desc), createdAt(_date) {}
};
