#ifndef SALES_H
#define SALES_H

#include <string>

struct soldCard {
    int id; // <-- ADD THIS LINE
    std::string type;
    std::string name;
    std::string setName;
    int year;
    std::string cardNumber;
    std::string condition;
    double purchasePrice;
    double finalSoldPrice;
    double profitMade;
    int quantitySold;
};

#endif // SALES_H