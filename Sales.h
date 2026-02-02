#ifndef SALES_H
#define SALES_H

#include <string>

struct soldCard {
    int id;
    std::string type;
    std::string name;
    std::string setName;
    int year;
    std::string cardNumber;
    std::string condition;
    std::string reference;
    double purchasePrice;
    double finalSoldPrice;
    double profitMade;
    int quantitySold;
};


#endif // SALES_H

