#include <iostream>
#include <string>
#include <vector>
#include <iomanip>
#include <limits>
#include <algorithm>
#include <cctype>
#include <fstream>
#include <sstream>
#include "sqlite3.h" // The SQLite library header
#include "Cards.h"
#include "Sales.h"
using namespace std;

// Forward Declarations for all functions
void initializeDatabase(sqlite3* db);
void loadInventory(vector<CardCollection>& inventory, sqlite3* db);
void loadSalesLog(vector<soldCard>& sales, sqlite3* db);
void showDashboard(const vector <CardCollection>& inventory, const vector<soldCard>& salesLog);
void addCard(sqlite3* db);
void editCard(vector<CardCollection>& inventory, sqlite3* db);
void printInventory(sqlite3* db);
void analyzeInventory(sqlite3* db);
void analyzeSales(sqlite3* db);
void printSalesLog(sqlite3* db);
void displayCardDetails(const CardCollection& card);
void displaySoldCardDetails(const soldCard& sale);
void deleteSale(vector<soldCard>& sales, sqlite3* db);
void importFromCSV(sqlite3* db, const string& filename);




// ===================================================================
// MAIN FUNCTION - The entry point of the program
// ===================================================================
int main() {
    sqlite3* db; // This object represents the database connection.
    int rc = sqlite3_open("inventory.db", &db); // Open or create the database file.

    if (rc) {
        cerr << "Error opening database: " << sqlite3_errmsg(db) << endl;
        return 1;
    }

    initializeDatabase(db); // Create the tables if they don't exist.

    vector<CardCollection> inventory;
    vector<soldCard> salesLog;

    int choice = 0;
    while (true) {
        // Always load fresh data from the DB before showing the dashboard and menu.
        loadInventory(inventory, db);
        loadSalesLog(salesLog, db);

        showDashboard(inventory, salesLog);

        cout << "\n--- Main Menu ---\n";
        cout << "1. Add a new Card\n";
        cout << "2. Edit/Sell a Card\n";
        cout << "3. View/Search Inventory \n";
        cout << "4. Analyze inventory or Sale Logs\n";
        cout << "5. Import Cards from external CSV\n";
        cout << "0. Exit\n";
        cout << "Enter your choice: ";
        cin >> choice;

        if (cin.fail()) {
            cin.clear(); cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid input, please enter a number.\n";
            continue;
        }

        switch (choice) {
        case 1:
            addCard(db);
            break;
        case 2:
            editCard(inventory, db);
            break;
        case 3:
            printInventory(db);
            break;
        case 4: {
            int analysisChoice;
            cout << "1. Analyze Inventory\n";
            cout << "2. Sales Log Management\n"; // Renamed for clarity
            cout << "Enter your choice: ";
            cin >> analysisChoice;

            if (cin.fail()) {
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                cout << "Invalid choice. Returning to main menu.\n";
                break; // Break from the case 4, which is correct
            }

            if (analysisChoice == 1) {
                analyzeInventory(db);
            }
            else if (analysisChoice == 2) {
                cout << "\n--- Sales Log Management ---\n";
                cout << "1. View and Sort Sales\n";
                cout << "2. Delete a Sale\n";
                cout << "0. Go Back\n";
                cout << "Enter your choice: ";

                int salesLogChoice;
                cin >> salesLogChoice;

                switch (salesLogChoice) {
                case 1:
                    printSalesLog(db);
                    analyzeSales(db);
                    break;
                case 2:
                    deleteSale(salesLog, db);
                    break;
                case 0:
                    break;
                default:
                    cout << "Invalid choice.\n";
                    break;
                }
            }
            break;
        }
        case 5:
            importFromCSV(db, "import.csv");
                break;
        case 0:
            cout << "Exiting Program\n";
            sqlite3_close(db); // Close the database connection before exiting.
            return 0;
        default:
            cout << "That was not an option presented, please try again\n";
            break;
        }
    }
}

// ===================================================================
// UTILITY & HELPER FUNCTIONS
// ===================================================================
string trim(const string& str) {
    size_t first = str.find_first_not_of(" \t\n\r");
    if (first == string::npos) return "";
    size_t last = str.find_last_not_of(" \t\n\r");
    return str.substr(first, last - first + 1);
}

void displayCardDetails(const CardCollection& card) {
    cout << "Name: " << card.name << "\n";
    cout << "Set: " << card.setName << " (" << card.cardNumber << ")\n";
    cout << "Condition: " << card.condition << "\n";
    cout << "Purchase Price: $" << fixed << setprecision(2) << card.purchasePrice << "\n";
    cout << "Ebay Comp Value: $" << fixed << setprecision(2) << card.ebayCompValue << "\n";
    cout << "Quantity: " << card.quantity << "\n";
    cout << "-----------------------------\n";
}

void displaySoldCardDetails(const soldCard& sales) {
        cout << "Card: " << sales.name
            << " | Set: " << sales.setName
            << " | Qty: " << sales.quantitySold
            << " | Sold Price: $" << fixed << setprecision(2) << sales.finalSoldPrice
            << " | Profit Made: $" << sales.profitMade << "\n";
        cout << "--------------------------------------\n";
    }


// ===================================================================
// DATABASE FUNCTIONS
// ===================================================================

void initializeDatabase(sqlite3* db) {
    char* zErrMsg = 0;
    const char* sqlInventory =
        "CREATE TABLE IF NOT EXISTS inventory ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "type TEXT NOT NULL,"
        "name TEXT NOT NULL,"
        "setName TEXT NOT NULL,"
        "cardNumber TEXT,"
        "condition TEXT,"
        "purchasePrice REAL NOT NULL,"
        "ebayCompValue REAL NOT NULL,"
        "quantity INTEGER NOT NULL);";

    const char* sqlSales =
        "CREATE TABLE IF NOT EXISTS sales ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "type TEXT NOT NULL,"
        "name TEXT NOT NULL,"
        "setName TEXT NOT NULL,"
        "cardNumber TEXT,"
        "condition TEXT,"
        "purchasePrice REAL NOT NULL,"
        "finalSoldPrice REAL NOT NULL,"
        "profitMade REAL NOT NULL,"
        "quantitySold INTEGER NOT NULL);";

    sqlite3_exec(db, sqlInventory, 0, 0, &zErrMsg);
    sqlite3_exec(db, sqlSales, 0, 0, &zErrMsg);

    if (zErrMsg) {
        cerr << "SQL error initializing database: " << zErrMsg << endl;
        sqlite3_free(zErrMsg);
    }
}

void loadInventory(vector<CardCollection>& inventory, sqlite3* db) {
    inventory.clear();
    const char* sql = "SELECT id, type, name, setName, cardNumber, condition, purchasePrice, ebayCompValue, quantity FROM inventory;";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            CardCollection newCard;
            newCard.id = sqlite3_column_int(stmt, 0);
            newCard.type = (const char*)sqlite3_column_text(stmt, 1);
            newCard.name = (const char*)sqlite3_column_text(stmt, 2);
            newCard.setName = (const char*)sqlite3_column_text(stmt, 3);
            newCard.cardNumber = (const char*)sqlite3_column_text(stmt, 4);
            newCard.condition = (const char*)sqlite3_column_text(stmt, 5);
            newCard.purchasePrice = sqlite3_column_double(stmt, 6);
            newCard.ebayCompValue = sqlite3_column_double(stmt, 7);
            newCard.quantity = sqlite3_column_int(stmt, 8);
            inventory.push_back(newCard);
        }
    }
    sqlite3_finalize(stmt);
}

void loadSalesLog(vector<soldCard>& sales, sqlite3* db) {
    sales.clear();
    const char* sql = "SELECT id, type, name, setName, cardNumber, condition, purchasePrice, finalSoldPrice, profitMade, quantitySold FROM sales;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            soldCard newSale;
            newSale.id = sqlite3_column_int(stmt, 0);
            newSale.type = (const char*)sqlite3_column_text(stmt, 1);
            newSale.name = (const char*)sqlite3_column_text(stmt, 2);
            newSale.setName = (const char*)sqlite3_column_text(stmt, 3);
            newSale.cardNumber = (const char*)sqlite3_column_text(stmt, 4);
            newSale.condition = (const char*)sqlite3_column_text(stmt, 5);
            newSale.purchasePrice = sqlite3_column_double(stmt, 6);
            newSale.finalSoldPrice = sqlite3_column_double(stmt, 7);
            newSale.profitMade = sqlite3_column_double(stmt, 8);
            newSale.quantitySold = sqlite3_column_int(stmt, 9);
            sales.push_back(newSale);
        }
    }
    sqlite3_finalize(stmt);
}

// ===================================================================
// CORE FEATURE FUNCTIONS
// ===================================================================

void showDashboard(const vector<CardCollection>& inventory, const vector<soldCard>& salesLog) {
    cout << "\n--- Your Dashboard ---\n";
    int totCards = 0;
    double totMarketValue = 0.0;
    int totSoldCards = 0;
    double totSaleValue = 0.0;
    double totNetProfit = 0.0;

    for (const auto& card : inventory) {
        totCards += card.quantity;
        totMarketValue += (card.ebayCompValue * card.quantity);
    }
    for (const auto& sold : salesLog) {
        totSoldCards += sold.quantitySold;
        totSaleValue += sold.finalSoldPrice;
        totNetProfit += sold.profitMade;
    }

    cout << fixed << setprecision(2);
    cout << "Total Cards in Inventory: " << totCards << "\n";
    cout << "Total Estimated Market Value: $" << totMarketValue << "\n";
    cout << "Total Cards Sold: " << totSoldCards << "\n";
    cout << "Total Value of Sales: $" << totSaleValue << "\n";
    cout << "Total Realized Profit: $" << totNetProfit << "\n";
    cout << "----------------------------------\n";
}

void addCard(sqlite3* db) {
    CardCollection newCard;
    cout << "\n--- Add a new card ---\n";
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    cout << "What Sport or TCG: "; getline(cin, newCard.type); newCard.type = trim(newCard.type);
    cout << "Name: "; getline(cin, newCard.name); newCard.name = trim(newCard.name);
    cout << "Set: "; getline(cin, newCard.setName); newCard.setName = trim(newCard.setName);
    cout << "Condition: "; getline(cin, newCard.condition); newCard.condition = trim(newCard.condition);

    sqlite3_stmt* stmt;
    const char* sql_check = "SELECT id, quantity FROM inventory WHERE type = ? AND name = ? AND setName = ? AND condition = ?;";
    sqlite3_prepare_v2(db, sql_check, -1, &stmt, NULL);
    sqlite3_bind_text(stmt, 1, newCard.type.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, newCard.name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, newCard.setName.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, newCard.condition.c_str(), -1, SQLITE_TRANSIENT);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        int existing_id = sqlite3_column_int(stmt, 0);
        int existing_quantity = sqlite3_column_int(stmt, 1);
        sqlite3_finalize(stmt);

        cout << "\nThis card already exists in your inventory.\n";
        int quantityToAdd = 0;
        while (true) {
            cout << "How many would you like to add to the existing quantity of " << existing_quantity << "? ";
            cin >> quantityToAdd;
            if (!cin.fail() && quantityToAdd > 0) break;
            cin.clear(); cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid input.\n";
        }

        int new_quantity = existing_quantity + quantityToAdd;
        const char* sql_update = "UPDATE inventory SET quantity = ? WHERE id = ?;";
        sqlite3_prepare_v2(db, sql_update, -1, &stmt, NULL);
        sqlite3_bind_int(stmt, 1, new_quantity);
        sqlite3_bind_int(stmt, 2, existing_id);
        if (sqlite3_step(stmt) != SQLITE_DONE) {
            cerr << "Error updating card: " << sqlite3_errmsg(db) << endl;
        }
        else {
            cout << "Quantity updated. New total: " << new_quantity << "\n";
        }
        sqlite3_finalize(stmt);
    }
    else {
        sqlite3_finalize(stmt);
        cout << "This is a new card. Please provide remaining details.\n";
        cout << "Number: "; getline(cin, newCard.cardNumber); newCard.cardNumber = trim(newCard.cardNumber);
        while (true) { cout << "Quantity: "; cin >> newCard.quantity; if (!cin.fail() && newCard.quantity > 0) break; cin.clear(); cin.ignore(numeric_limits<streamsize>::max(), '\n'); cout << "Invalid input.\n"; }
        while (true) { cout << "Price Paid: $"; cin >> newCard.purchasePrice; if (!cin.fail()) break; cin.clear(); cin.ignore(numeric_limits<streamsize>::max(), '\n'); cout << "Invalid input.\n"; }
        while (true) { cout << "Ebay Comp Value: $"; cin >> newCard.ebayCompValue; if (!cin.fail()) break; cin.clear(); cin.ignore(numeric_limits<streamsize>::max(), '\n'); cout << "Invalid input.\n"; }

        const char* sql_insert = "INSERT INTO inventory (type, name, setName, cardNumber, condition, purchasePrice, ebayCompValue, quantity) VALUES (?, ?, ?, ?, ?, ?, ?, ?);";
        sqlite3_prepare_v2(db, sql_insert, -1, &stmt, NULL);
        sqlite3_bind_text(stmt, 1, newCard.type.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, newCard.name.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 3, newCard.setName.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 4, newCard.cardNumber.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 5, newCard.condition.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_double(stmt, 6, newCard.purchasePrice);
        sqlite3_bind_double(stmt, 7, newCard.ebayCompValue);
        sqlite3_bind_int(stmt, 8, newCard.quantity);

        if (sqlite3_step(stmt) != SQLITE_DONE) {
            cerr << "Error inserting card: " << sqlite3_errmsg(db) << endl;
        }
        else {
            cout << "You have now successfully added " << newCard.name << " to your inventory\n";
        }
        sqlite3_finalize(stmt);
    }
}

void editCard(vector<CardCollection>& inventory, sqlite3* db) {
    if (inventory.empty()) {
        cout << "\nThere is nothing Currently in your inventory\n";
        return;
    }
    cout << "\n--- Select a card to edit ---\n";
    for (size_t i = 0; i < inventory.size(); ++i) {
        cout << i + 1 << ". " << inventory[i].name << " #" << inventory[i].cardNumber << "\n";
    }
    cout << "------------------\n";
    int cardChoice = 0;
    while (true) {
        cout << "Enter the number of the card to edit (or 0 to cancel):";
        cin >> cardChoice;
        if (!cin.fail() && cardChoice >= 0 && (size_t)cardChoice <= inventory.size()) break;
        cin.clear(); cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Invalid Input.\n";
    }
    if (cardChoice == 0) { return; }
    int index = cardChoice - 1;
    CardCollection& cardToEdit = inventory[index];
    int card_db_id = cardToEdit.id;

    cout << "\nEditing: " << cardToEdit.name << "\n";
    cout << "1.Type\n 2.Name\n3. Set \n4. Card Number\n5. Condition\n6. Purchase Price\n"
        << "7. Ebay Comp Value\n8. Quantity\n9. Delete\n10. Mark as Sold\n0. Cancel\n"
        << "Enter your Choice: ";
    int editChoice;
    cin >> editChoice;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    sqlite3_stmt* stmt = nullptr;

    switch (editChoice) {
    case 1: { string val; cout << "New Sport or TCG: "; getline(cin, val); const char* sql = "UPDATE inventory SET type = ? WHERE id = ?"; sqlite3_prepare_v2(db, sql, -1, &stmt, NULL); sqlite3_bind_text(stmt, 1, val.c_str(), -1, SQLITE_TRANSIENT); sqlite3_bind_int(stmt, 2, card_db_id); break;}
    case 2: { string val; cout << "New Name: "; getline(cin, val); const char* sql = "UPDATE inventory SET name = ? WHERE id = ?;"; sqlite3_prepare_v2(db, sql, -1, &stmt, NULL); sqlite3_bind_text(stmt, 1, val.c_str(), -1, SQLITE_TRANSIENT); sqlite3_bind_int(stmt, 2, card_db_id); break; }
    case 3: { string val; cout << "New Set: "; getline(cin, val); const char* sql = "UPDATE inventory SET setName = ? WHERE id = ?;"; sqlite3_prepare_v2(db, sql, -1, &stmt, NULL); sqlite3_bind_text(stmt, 1, val.c_str(), -1, SQLITE_TRANSIENT); sqlite3_bind_int(stmt, 2, card_db_id); break; }
    case 4: { string val; cout << "New Card Number: "; getline(cin, val); const char* sql = "UPDATE inventory SET cardNumber = ? WHERE id = ?;"; sqlite3_prepare_v2(db, sql, -1, &stmt, NULL); sqlite3_bind_text(stmt, 1, val.c_str(), -1, SQLITE_TRANSIENT); sqlite3_bind_int(stmt, 2, card_db_id); break; }
    case 5: { string val; cout << "New Condition: "; getline(cin, val); const char* sql = "UPDATE inventory SET condition = ? WHERE id = ?;"; sqlite3_prepare_v2(db, sql, -1, &stmt, NULL); sqlite3_bind_text(stmt, 1, val.c_str(), -1, SQLITE_TRANSIENT); sqlite3_bind_int(stmt, 2, card_db_id); break; }
    case 6: { double val; cout << "New Purchase Price: $"; cin >> val; const char* sql = "UPDATE inventory SET purchasePrice = ? WHERE id = ?;"; sqlite3_prepare_v2(db, sql, -1, &stmt, NULL); sqlite3_bind_double(stmt, 1, val); sqlite3_bind_int(stmt, 2, card_db_id); break; }
    case 7: { double val; cout << "New Ebay Comp Value: $"; cin >> val; const char* sql = "UPDATE inventory SET ebayCompValue = ? WHERE id = ?;"; sqlite3_prepare_v2(db, sql, -1, &stmt, NULL); sqlite3_bind_double(stmt, 1, val); sqlite3_bind_int(stmt, 2, card_db_id); break; }
    case 8: { int val; cout << "New Quantity: "; cin >> val; const char* sql = "UPDATE inventory SET quantity = ? WHERE id = ?;"; sqlite3_prepare_v2(db, sql, -1, &stmt, NULL); sqlite3_bind_int(stmt, 1, val); sqlite3_bind_int(stmt, 2, card_db_id); break; }
    case 9: {
        char confirm = 'n';
        cout << "Are you sure you want to delete '" << cardToEdit.name << "'? (y/n): ";
        cin >> confirm;
        if (confirm == 'y' || confirm == 'Y') {
            const char* sql = "DELETE FROM inventory WHERE id = ?;";
            sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
            sqlite3_bind_int(stmt, 1, card_db_id);
            if (sqlite3_step(stmt) != SQLITE_DONE) cerr << "Error deleting card: " << sqlite3_errmsg(db) << endl;
            else cout << "Card deleted.\n";
        }
        else { cout << "Deletion cancelled.\n"; }
        sqlite3_finalize(stmt);
        return;
    }
    case 10: {
        int quantityToSell = 0;
        if (cardToEdit.quantity > 1) { while (true) { cout << "You have " << cardToEdit.quantity << " copies. How many are you selling? "; cin >> quantityToSell; if (!cin.fail() && quantityToSell > 0 && quantityToSell <= cardToEdit.quantity) break; cin.clear(); cin.ignore(numeric_limits<streamsize>::max(), '\n'); cout << "Invalid input.\n"; } }
        else { quantityToSell = 1; }
        double salePricePerCard = 0.0;
        while (true) { cout << "Enter the final sale price PER CARD: $"; cin >> salePricePerCard; if (!cin.fail() && salePricePerCard >= 0) break; cin.clear(); cin.ignore(numeric_limits<streamsize>::max(), '\n'); cout << "Invalid input.\n"; }

        soldCard newSoldCard;
        newSoldCard.type = cardToEdit.type;
        newSoldCard.name = cardToEdit.name; newSoldCard.setName = cardToEdit.setName; newSoldCard.cardNumber = cardToEdit.cardNumber;
        newSoldCard.condition = cardToEdit.condition; newSoldCard.purchasePrice = cardToEdit.purchasePrice;
        newSoldCard.quantitySold = quantityToSell; newSoldCard.finalSoldPrice = salePricePerCard * quantityToSell;
        newSoldCard.profitMade = (salePricePerCard - newSoldCard.purchasePrice) * quantityToSell;

        const char* sql_insert = "INSERT INTO sales ( type, name, setName, cardNumber, condition, purchasePrice, finalSoldPrice, profitMade, quantitySold) VALUES (?,?,?,?,?,?,?,?,?);";
        sqlite3_prepare_v2(db, sql_insert, -1, &stmt, NULL);
        sqlite3_bind_text(stmt, 1, newSoldCard.type.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, newSoldCard.name.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 3, newSoldCard.setName.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 4, newSoldCard.cardNumber.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 5, newSoldCard.condition.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_double(stmt, 6, newSoldCard.purchasePrice);
        sqlite3_bind_double(stmt, 7, newSoldCard.finalSoldPrice);
        sqlite3_bind_double(stmt, 8, newSoldCard.profitMade);
        sqlite3_bind_int(stmt, 9, newSoldCard.quantitySold);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);

        int remaining_qty = cardToEdit.quantity - quantityToSell;
        if (remaining_qty <= 0) {
            const char* sql_delete = "DELETE FROM inventory WHERE id = ?;";
            sqlite3_prepare_v2(db, sql_delete, -1, &stmt, NULL);
            sqlite3_bind_int(stmt, 1, card_db_id);
            sqlite3_step(stmt);
            sqlite3_finalize(stmt);
            cout << "All copies sold. Card removed from inventory.\n";
        }
        else {
            const char* sql_update = "UPDATE inventory SET quantity = ? WHERE id = ?;";
            sqlite3_prepare_v2(db, sql_update, -1, &stmt, NULL);
            sqlite3_bind_int(stmt, 1, remaining_qty);
            sqlite3_bind_int(stmt, 2, card_db_id);
            sqlite3_step(stmt);
            sqlite3_finalize(stmt);
            cout << "Remaining quantity: " << remaining_qty << "\n";
        }
        cout << "Sale recorded successfully.\n";
        return;
    }
    case 0: cout << "Edit Cancelled.\n"; return;
    default: cout << "Invalid choice.\n"; return;
    }

    if (stmt) {
        if (sqlite3_step(stmt) == SQLITE_DONE) {
            cout << "Card updated successfully.\n";
        }
        else {
            cerr << "Error updating card: " << sqlite3_errmsg(db) << endl;
        }
        sqlite3_finalize(stmt);
    }
}

void printInventory(sqlite3* db) {
    cout << "\n- - - Your Card Inventory - - - \n";
    cout << "1. Search by Name\n2. Alphabetically (A-Z)\n3. By Highest Value\n4. By Highest Potential Profit\n5. Search by Set\n6. By order of Entry (Newest First)\n6.Sport or TCG\n0. Cancel\n";
    int sortChoice;
    cin >> sortChoice;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    string sql = "SELECT id, type, name, setName, cardNumber, condition, purchasePrice, ebayCompValue, quantity FROM inventory ";
    string search_term;

    switch (sortChoice) {
    case 1:
        cout << "Enter Card to search: "; getline(cin, search_term);
        sql += "WHERE name LIKE ? ORDER BY name;";
        break;
    case 2: sql += "ORDER BY name;"; break;
    case 3: sql += "ORDER BY ebayCompValue DESC;"; break;
    case 4: sql += "ORDER BY (ebayCompValue - purchasePrice) * quantity DESC;"; break;
    case 5:
        cout << "Enter desired Set Name: "; getline(cin, search_term);
        sql += "WHERE setName LIKE ? ORDER BY name;";
        break;
    case 6: sql += "ORDER BY id DESC;"; break;
    case 7: sql += "ORDER BY type; "; break;
    case 0: cout << "Cancelled.\n"; return;
    default: cout << "Invalid choice.\n"; return;
    }

    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL);

    if (sortChoice == 1 || sortChoice == 5) {
        string pattern = "%" + search_term + "%";
        sqlite3_bind_text(stmt, 1, pattern.c_str(), -1, SQLITE_TRANSIENT);
    }

    cout << "\n- - - Displaying Inventory - - -\n";
    int count = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        CardCollection card;
        card.id = sqlite3_column_int(stmt, 0);
        card.type = (const char*)sqlite3_column_text(stmt, 1);
        card.name = (const char*)sqlite3_column_text(stmt, 2);
        card.setName = (const char*)sqlite3_column_text(stmt, 3);
        card.cardNumber = (const char*)sqlite3_column_text(stmt, 4);
        card.condition = (const char*)sqlite3_column_text(stmt, 5);
        card.purchasePrice = sqlite3_column_double(stmt, 6);
        card.ebayCompValue = sqlite3_column_double(stmt, 7);
        card.quantity = sqlite3_column_int(stmt, 8);
        displayCardDetails(card);
        count++;
    }
    if (count == 0) {
        cout << "No cards found matching your criteria.\n";
    }
    sqlite3_finalize(stmt);
}

void analyzeInventory(sqlite3* db) {
    cout << "\n--- Inventory Analysis ---\n";
    const char* sql = "SELECT SUM(purchasePrice * quantity), SUM(ebayCompValue * quantity), SUM((ebayCompValue - purchasePrice) * quantity) FROM inventory;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW && sqlite3_column_type(stmt, 0) != SQLITE_NULL) {
            double totalPurchasePrice = sqlite3_column_double(stmt, 0);
            double totalMarketValue = sqlite3_column_double(stmt, 1);
            double totalPotentialProfit = sqlite3_column_double(stmt, 2);

            cout << "--------------------------------\n";
            cout << "Total Purchase Price: $" << fixed << setprecision(2) << totalPurchasePrice << endl;
            cout << "Total Market Value: $" << fixed << setprecision(2) << totalMarketValue << endl;
            cout << "Total Potential Profit: $" << fixed << setprecision(2) << totalPotentialProfit << endl;
            cout << "--------------------------------\n";
        }
        else {
            cout << "No inventory to analyze.\n";
        }
    }
    sqlite3_finalize(stmt);
}

void printSalesLog(sqlite3* db) {
    cout << "\n--- Detailed Sales Log ---\n";
    cout << "How would you like to sort?\n";
    cout << "1. By Highest Profit\n";
    cout << "2. Alphabetically (A-Z)\n";
    cout << "3. By Sale Order (Newest First)\n";
    cout << "4. Search by Name\n";
    cout << "0. Go Back\n";
    cout << "Enter your choice: ";

    int sortChoice;
    cin >> sortChoice;

    if (cin.fail()) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Invalid input.\n";
        return;
    }

    // Return to menu if user chooses 0
    if (sortChoice == 0) {
        return;
    }

    string sql = "SELECT id, type, name, setName, cardNumber, condition, purchasePrice, finalSoldPrice, profitMade, quantitySold FROM sales ";
    string search_term;

    switch (sortChoice) {
    case 1:
        sql += "ORDER BY profitMade DESC;";
        break;
    case 2:
        sql += "ORDER BY name ASC;";
        break;
    case 3:
        sql += "ORDER BY id DESC;";
        break;
    case 4: {
        cout << "Enter card name to search for: ";
        // Clear the newline character left by 'cin >> sortChoice;'
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        getline(cin, search_term);
        sql += "WHERE name LIKE ? ORDER BY name;";
        break;
    }
    default:
        cout << "Invalid choice.\n";
        return;
    }

    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL);

    // Bind the search term only if the user chose the search option
    if (sortChoice == 4) {
        string pattern = "%" + search_term + "%";
        sqlite3_bind_text(stmt, 1, pattern.c_str(), -1, SQLITE_TRANSIENT);
    }

    cout << "\n--- Displaying Sales ---\n";
    int count = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        soldCard sale;
        sale.id = sqlite3_column_int(stmt, 0);
        sale.type = (const char*)sqlite3_column_text(stmt, 1);
        sale.name = (const char*)sqlite3_column_text(stmt, 2);
        sale.setName = (const char*)sqlite3_column_text(stmt, 3);
        sale.cardNumber = (const char*)sqlite3_column_text(stmt, 4);
        sale.condition = (const char*)sqlite3_column_text(stmt, 5);
        sale.purchasePrice = sqlite3_column_double(stmt, 6);
        sale.finalSoldPrice = sqlite3_column_double(stmt, 7);
        sale.profitMade = sqlite3_column_double(stmt, 8);
        sale.quantitySold = sqlite3_column_int(stmt, 9);

        displaySoldCardDetails(sale); // Call the display function
        count++;
    }

    if (count == 0) {
        cout << "No sales found matching your criteria.\n";
    }
    sqlite3_finalize(stmt);
}
void analyzeSales(sqlite3* db) {
    const char* sql = "SELECT SUM(finalSoldPrice), SUM(profitMade) FROM sales;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW && sqlite3_column_type(stmt, 0) != SQLITE_NULL) {
            double totalSalesValue = sqlite3_column_double(stmt, 0);
            double totalProfitMade = sqlite3_column_double(stmt, 1);

            cout << "\n- - - Lifetime Sales Summary - - -\n";
            cout << "Total Sales Value: $" << fixed << setprecision(2) << totalSalesValue << endl;
            cout << "Total Profit Made: $" << fixed << setprecision(2) << totalProfitMade << endl;
            cout << "--------------------------------\n";
        }
        else {
            cout << "No sales to analyze.\n";
        }
    }
    sqlite3_finalize(stmt); 
}
void deleteSale(vector<soldCard>& sales, sqlite3* db) {
    cout << "\n --- Select a Sale to Delete ---\n";
    if (sales.empty()) {
        cout << "There are no sales to delete. \n";
        return;
    }
    cout << "0. Cancel\n";
    for (size_t i = 0; i < sales.size(); ++i) {
        cout << i + 1 << ". " << sales[i].name
            << " (Sold for $" << fixed << setprecision(2) << sales[i].finalSoldPrice << ")\n";
    }
    cout << "----------------------------------\n";
    cout << "Select the Number of cards you would like to delete\n";
    int choice = 0;
    while (true) {
        cout << "Enter the number of the sale to delete (or 0 to cancel): ";
        cin >> choice;

        if (cin.fail()) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid input. Please enter a number.\n";
            continue; 
        }

        if (choice >= 0 && (size_t)choice <= sales.size()) {
            break; 
        }
        else {
            cout << "Invalid number. Please try again.\n";
        }
    }

    if (choice == 0) {
        cout << "Deletion cancelled.\n";
        return; 
    }
    int sale_id_to_delete = sales[choice - 1].id;
    string sale_name_to_delete = sales[choice - 1].name;

    char confirm = 'n';
    cout << "Are you sure you want to delete the sale of '" << sale_name_to_delete << "'? (y/n): ";
    cin >> confirm;
    cin.ignore(numeric_limits<streamsize>::max(), '\n'); 

    if (confirm == 'y' || confirm == 'Y') {
        const char* sql = "DELETE FROM sales WHERE id = ?;";
        sqlite3_stmt* stmt;

        sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

        sqlite3_bind_int(stmt, 1, sale_id_to_delete);

        if (sqlite3_step(stmt) == SQLITE_DONE) {
            cout << "Sale deleted successfully.\n";
        }
        else {
            cerr << "Error deleting sale: " << sqlite3_errmsg(db) << endl;
        }

        sqlite3_finalize(stmt);

    }
    else {
        cout << "Deletion cancelled.\n";
    }
}

void importFromCSV(sqlite3* db, const string& filename) {
    cout << "\n--- Importing from " << filename << " ---\n";
    ifstream inputFile(filename); // Open the specified CSV file

    if (!inputFile.is_open()) {
        cerr << "Error: Could not open file " << filename << endl;
        return;
    }

    string line;
    int successCount = 0;
    int failCount = 0;

    // Skip the header row
    getline(inputFile, line);

    // Read the file line by line
    while (getline(inputFile, line)) {
        stringstream ss(line);
        string field;
        vector<string> fields;

        // Split the line by commas
        while (getline(ss, field, ',')) {
            fields.push_back(field);
        }

        if (fields.size() == 8) {
            CardCollection newCard;
            try {
                // Assign and convert data from the CSV fields
                newCard.type = trim(fields[0]);
                newCard.name = trim(fields[1]);
                newCard.setName = trim(fields[2]);
                newCard.cardNumber = trim(fields[3]);
                newCard.condition = trim(fields[4]);
                newCard.purchasePrice = stod(fields[5]); // stod = string to double
                newCard.ebayCompValue = stod(fields[6]);
                newCard.quantity = stoi(fields[7]);      // stoi = string to integer

                // Prepare the SQL INSERT statement
                const char* sql_insert = "INSERT INTO inventory (type, name, setName, cardNumber, condition, purchasePrice, ebayCompValue, quantity) VALUES (?, ?, ?, ?, ?, ?, ?, ?);";
                sqlite3_stmt* stmt;
                sqlite3_prepare_v2(db, sql_insert, -1, &stmt, NULL);

                // Bind the values to the statement
                sqlite3_bind_text(stmt, 1, newCard.type.c_str(), -1, SQLITE_TRANSIENT);
                sqlite3_bind_text(stmt, 2, newCard.name.c_str(), -1, SQLITE_TRANSIENT);
                sqlite3_bind_text(stmt, 3, newCard.setName.c_str(), -1, SQLITE_TRANSIENT);
                sqlite3_bind_text(stmt, 4, newCard.cardNumber.c_str(), -1, SQLITE_TRANSIENT);
                sqlite3_bind_text(stmt, 5, newCard.condition.c_str(), -1, SQLITE_TRANSIENT);
                sqlite3_bind_double(stmt, 6, newCard.purchasePrice);
                sqlite3_bind_double(stmt, 7, newCard.ebayCompValue);
                sqlite3_bind_int(stmt, 8, newCard.quantity);

                if (sqlite3_step(stmt) == SQLITE_DONE) {
                    successCount++;
                }
                else {
                    failCount++;
                }
                sqlite3_finalize(stmt);

            }
            catch (const std::invalid_argument& e) {
                // This will catch errors if stod() or stoi() fail (e.g., bad number format)
                failCount++;
            }
        }
        else {
            failCount++;
        }
    }

    inputFile.close();
    cout << "Import complete.\nSuccessfully imported " << successCount << " cards.\nFailed to import " << failCount << " rows.\n";
}
