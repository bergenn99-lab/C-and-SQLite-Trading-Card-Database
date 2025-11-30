# C++ and SQLite Trading Card Database

A high-performance console application built with C++ and SQLite for managing and tracking personal trading card collections.

[Add a single badge here later if you use one, e.g., Status: In Development]

---

## Features
This application provides robust capabilities for managing your collection data:

* **CRUD Operations:** Seamlessly Create, Read, Update, and Delete card and sale records.
* **Search & Query:** Query the database based on criteria like card name, rarity, set, and ownership status.
* **Data Integrity:** Utilizes SQLite transactions and prepared statements for secure and reliable data manipulation.
* **Collection Tracking:** Track quantities, condition, and purchase/sale prices for comprehensive collection value assessment.

## Technologies Used
* **Core Language:** C++ (using C++17 standards)
* **Database:** SQLite3 C/C++ API
* **Build System:** [Specify your build system, e.g., GCC/G++, CMake, Visual Studio]
* **Libraries:** [List any specific third-party C++ libraries you used, e.g., for parsing, networking, etc.]

## Installation and Setup
Follow these steps to get the project running on your local machine.

### Prerequisites
* A C++ Compiler (e.g., GCC or Clang)
* SQLite3 development libraries installed on your system.
    * *If using a Linux/Unix system:* `sudo apt-get install libsqlite3-dev`
    * *[Add specific instructions for Windows or Mac if needed]*

### Building the Project
1.  **Clone the repository:**
    ```bash
    git clone [Paste your repository URL here]
    cd C-and-SQLite-Trading-Card-Database
    ```

2.  **Compile the source files:**
    Use your compiler to link the C++ source files with the SQLite library.
    ```bash
    g++ ConsoleApplication2.cpp [Cards.cpp] [Sales.cpp] -o tcdb -lsqlite3
    # Replace the bracketed files if your project uses multiple .cpp files besides the main one.
    ```

### Running the Application
1.  **Initial Database Setup:**
    The application will automatically create the `cards.db` file and necessary tables upon first run if they don't exist.

2.  **Execute the program:**
    ```bash
    ./tcdb
    ```

## Usage
Once running, the application presents a command-line interface (CLI) with the following commands:

1.  **Add Card:** `add [Card Name] [Set] [Rarity] [Quantity]`
2.  **View Collection:** `list all`
3.  **Search:** `search name [partial name]`
4.  **Update Qty:** `update [Card Name] [New Quantity]`
5.  **Exit:** `exit`

## Contributing
Contributions are welcome! If you have suggestions for new features or find a bug, please open an issue or submit a pull request.

## Contact
* GitHub: [@bergenn99-lab](https://github.com/bergenn99-lab)
* [LinkedIn/Personal Website Link](www.linkedin.com/in/noah-bergen)
