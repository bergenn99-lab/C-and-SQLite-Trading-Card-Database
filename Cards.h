#ifndef CARDS_H
#define CARDS_H

#include <string>

struct CardCollection {
    int id; // <-- ADD THIS LINE
    std::string type;
    std::string name;
    std::string setName;
    int year;
    std::string cardNumber;
    std::string condition;
    std::string reference;
    double purchasePrice;
    double ebayCompValue;
    int quantity;
};


#endif // CARDS_H
