#include <iostream>
#include <string>
#include <ctime>
#include <limits>
#include <cstring>
#include <sstream>
#include "cryptopp/rsa.h"
#include "cryptopp/osrng.h"
#include "cryptopp/hex.h"
#include "cryptopp/base64.h"
#include "cryptopp/files.h"
#include "cryptopp/filters.h"
#include <filesystem>  // Use standard filesystem for C++17
namespace fs = std::filesystem;


class UserData{ /// changes include replacing C type arras to more secure cpp ones
    public:
    UserData(const std::string& firstName, const std::string& lastName, const std::string& address)
             : fname(firstName), lname(lastName), address(address) {
                acc_num=getLastRowOfColumn()+1;
             }
    void DisplayUserInfo() const{
        std::cout << fname << " " << lname << " residing at " << address << std::endl;
    }

    // CSV initialisation
    void SaveToCSV() const {
        if (!fs::exists(filename)) {  // Using experimental filesystem
            std::ofstream file(filename, std::ios::out);
            file << "Account Number,First Name,Last Name,Balance,Address,Currency\n";  // Add headers
        }
        std::ofstream file(filename, std::ios::app);  // Append to the file
       file << acc_num << "," << fname << "," << lname << "," << address << "\n";
    }

    private: // private for user security.
    std::string fname;
    std::string lname; 
    std::string address;
    // data for account number: 
    int acc_num{};  // Declare acc_num here
    const std::string filename = "userdata.csv";
    
    int getLastRowOfColumn() {
        std::ifstream file(filename);
        std::string line;
        int last_acc_num = 1000;

        while (getline(file, line)) {
            std::stringstream lineStream(line);
            std::string cell;
            std::vector<std::string> row;

            while (getline(lineStream, cell, ',')) {
                row.push_back(cell);
            }

            if (!row.empty()) {
                last_acc_num = std::stoi(row[0]);
            }
        }
        return last_acc_num;
    }
};   
// card dummy
class Card { // const kept, even in private for greater security.
private:
    const std::string card_number;
    const int cvv;
    const std::string exp_date;
     double balance;
    std::string currency;
    const int account_nb;
    int pin;
    std::string password;
    std::string token;
    // Constructor to initialize the card number
    
    void GetCurrentMonthYear(int &currentmonth, int &currentyear) const{
        time_t t = time (0);
        struct tm*now = localtime(&t);
        currentmonth = (now ->tm_mon) + 1; // tm-mon is zero based, add 1 for allowing twelve month.
        currentyear = (now ->tm_year) % 100; // gets last two digits by using modulo and 1900.
    }

    bool IsCardExpired(int expmonth, int expyear) const {
        int currentmonth, currentyear; // access time in format.
        GetCurrentMonthYear(currentmonth, currentyear);
        return (expyear < currentyear) || (expyear == currentyear && expmonth == currentmonth); // backchecks if dates match.
    }
 ///luhn greater strcture and comments. + Log (n) better results. 
    bool ValidatedLuhn() const {
        if (card_number.empty()) {
        std::cout << "Card number is empty" << std::endl;
        return false;
    }
    if (!std::all_of(card_number.begin(), card_number.end(), ::isdigit)) {
        std::cout << "Card number contains invalid characters" << std::endl;
        return false;
    }
    int sum = 0;
    bool alternate = false;
    // Iterate over card number from the rightmost digit
    for (int i = card_number.length() - 1; i >= 0; --i) {
        int n = card_number[i] - '0';

        // If it's an alternate digit, double it
        if (alternate) {
            n *= 2;
            // If the result is greater than 9, subtract 9
            if (n > 9) {
                n -= 9;
            }
        }
        
        sum += n; 
        alternate = !alternate;  // Flip the alternate flag
    }
return (sum % 10 == 0);
}
 //finish of lunhu
    std::string DetermineCardType () const {
        if (card_number[0] == '4' && (card_number.length() == 13 || card_number.length() == 16 || card_number.length() == 19)) {
        return "The Card is Visa";
    }
     int first_two_digits = std::stoi(card_number.substr(0,2));
     int first_four_digits = std::stoi(card_number.substr(0,4));

     if (card_number.length() == 16 && ((first_two_digits >= 51 && first_two_digits <= 55) || (first_four_digits > 2221 && first_four_digits <= 2720))) { // check for args
        return " The Card is MasterCard/EuroCard"; // determine card 
     }

     return "Invalid Card Type"; //else invalid card
}
public:
    Card(const std::string &in_card_number, const int &in_cvv, const std::string &in_exp_date, const std::string & in_currency, const int &in_account_nb) 
        : card_number(in_card_number), cvv(in_cvv), exp_date(in_exp_date), currency(in_currency), account_nb(in_account_nb) {
        if (cvv < 100 || cvv > 999){  // check length using three digits.
            throw std::invalid_argument("CVV code is incorrect"); // error if ccv is incorrect
        }

        std::stringstream ss(exp_date);
        int expiry_month, expiry_year;
        char delimiter;
        ss >> expiry_month >> delimiter >> expiry_year;

        if (IsCardExpired(expiry_month, expiry_year)) { // function check car is expired
            throw std::runtime_error("Your card is expired");
        }
    }

    void check() const {
     if (card_number.empty()) {
            std::cout << "Card number is empty" << std::endl; // Check for empty input
            return;
    }

    if (card_number.length() < 13) {
        std::cout << "Card number is too short" << std::endl; 
        return;
    }

    if (card_number.length() > 19) {
        std::cout << "Card number is too long" << std::endl; 
        return;
    }

    // Ensure all characters are digits
    if (!std::all_of(card_number.begin(), card_number.end(), ::isdigit)) {
        std::cout << "Card number contains invalid characters" << std::endl; 
        return;
    }

    if (ValidatedLuhn()) {
        std::cout << "Card Number is VALID" << std::endl;
        std::cout << "Card Type: " << DetermineCardType() << std::endl;
    } else {
        std::cout << "Card Number is INVALID!" << std::endl;
    }
}
};

class bank {
    private:
        CryptoPP::RSA::PublicKey publicKey;
        CryptoPP::RSA::PrivateKey privateKey;

    public:

        bank (){
            generateRSAKeys();
        }

        void generateRSAKeys() {
        CryptoPP::AutoSeededRandomPool rng;
        privateKey.GenerateRandomWithKeySize(rng, 2048);
        publicKey.AssignFrom(privateKey);
    }

    // Helper function to encode a key to a string (Base64)
    std::string encodeKeyToBase64(const CryptoPP::RSA::PublicKey& key) {
        std::string encoded;
        CryptoPP::Base64Encoder encoder(new CryptoPP::StringSink(encoded));
        key.DEREncode(encoder); // DER encoding of the key
        encoder.MessageEnd();
        return encoded;
    }

    std::string encodeKeyToBase64(const CryptoPP::RSA::PrivateKey& key) {
        std::string encoded;
        CryptoPP::Base64Encoder encoder(new CryptoPP::StringSink(encoded));
        key.DEREncode(encoder); // DER encoding of the key
        encoder.MessageEnd();
        return encoded;
    }

    // Print the public and private keys in Base64 encoded form
    void printKeys() {
        std::string encodedPublicKey = encodeKeyToBase64(publicKey);
        std::string encodedPrivateKey = encodeKeyToBase64(privateKey);

        std::cout << "Public Key (Base64 Encoded): \n" << encodedPublicKey << "\n";
        std::cout << "Private Key (Base64 Encoded): \n" << encodedPrivateKey << "\n";
    }


        void CreatUser() {
            std::string fname;
            std::cout << "Enter first name: " << std::endl;
            std::cin >> fname;

            std::string lname;
            std::cout << "Enter last name: " << std::endl;
            std::cin >> lname;

            std::string currency;
            std::cout << "Enter currency: " << std::endl;
            std::cin >> currency;

            std::string address;
            std::cout << "Enter address: " << std::endl;
            std::cin >> address;

            double balance = 0;

            UserData customer(fname, lname, address);
            customer.SaveToCSV();
            customer.DisplayUserInfo();
        }
        /*
        read and write the data from csv file (geters and setters) x
        fetch the data x
        bank should call encryption and decryption functions
        then check the functions
        respond to terminal  
        hash function that hash the passowords in the terminal and the bank 
        compare the hash values inside the terminal 
        
        private key only for the bank and public key for everyone
        */
        
        
        //public:
            //bank(const std::string& );
};

class terminal {
    private:
        // Transaction struct to store details
        struct Transaction {
            long long int TUN;  // Transaction unique number
            std::string currency;
            std::string data;  // Transaction data
            std::string transaction_type;  // "Contactless" or "Contact"

            // Constructor
            Transaction(long long int tun, const std::string& curr, const std::string& d, const std::string& t_type)
                : TUN(tun), currency(curr), data(d), transaction_type(t_type) {}
        };

        // List to hold all transactions
        std::list<Transaction> transaction_data;

    public:
        // Constructor
        terminal() {
            // Perform initialization if necessary
        }

        // Add a transaction (with user input for transaction type: "Contactless" or "Contact")
        void add_transaction(long long int tun, const std::string& curr, const std::string& data) {
            std::string transaction_type = selectTransactionType();  // Get transaction type from user
            Transaction new_transaction(tun, curr, data, transaction_type);
            transaction_data.push_back(new_transaction);
        }

        // Display all transactions
        void display_transactions() const {
            for (const auto& trans : transaction_data) {
                std::cout << "TUN: " << trans.TUN
                        << ", Currency: " << trans.currency
                        << ", Transaction location: " << trans.data
                        << ", Transaction Type: " << trans.transaction_type
                        << std::endl;
            }
        }

        // Function to select transaction type ("Contactless" or "Contact")
        std::string selectTransactionType() {
            int transactionType;
            std::cout << " Choose 1 for Contactless or 2 for Contact: ";

            while (true) {
                std::cin >> transactionType;

                // Check if the input is valid
                if (std::cin.fail() || (transactionType != 1 && transactionType != 2)) {
                    std::cin.clear();  // Clear the error flag on cin
                    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');  // Ignore invalid input
                    std::cout << "Invalid input. Please enter 1 for Contactless or 2 for Contact: ";
                } else {
                    break;
                }
            }

            // Return the transaction type as a string
            return (transactionType == 1) ? "Contactless" : "Contact";
        }
};





/// RSA ALGORITHM START
// RSA key generation and encryption/decryption functions

void generateRSAKeys(CryptoPP::RSA::PublicKey &publicKey, CryptoPP::RSA::PrivateKey &privateKey) {
    CryptoPP::AutoSeededRandomPool rng;
    privateKey.GenerateRandomWithKeySize(rng, 2048);
    publicKey.AssignFrom(privateKey);
}

std::string encryptCardNumber(const std::string &cardNumber, CryptoPP::RSA::PublicKey &publicKey) {
    CryptoPP::AutoSeededRandomPool rng;
    std::string cipher;
    CryptoPP::RSAES_OAEP_SHA_Encryptor encryptor(publicKey);
    CryptoPP::StringSource ss1(cardNumber, true, new CryptoPP::PK_EncryptorFilter(rng, encryptor, new CryptoPP::StringSink(cipher)));
    return cipher;
}

std::string decryptCardNumber(const std::string &cipher, CryptoPP::RSA::PrivateKey &privateKey) {
    CryptoPP::AutoSeededRandomPool rng;
    std::string recovered;
    CryptoPP::RSAES_OAEP_SHA_Decryptor decryptor(privateKey);
    CryptoPP::StringSource ss4(cipher, true, new CryptoPP::PK_DecryptorFilter(rng, decryptor, new CryptoPP::StringSink(recovered)));
    return recovered;
}

void inputCardDetails(std::string& cardNumber, int &cvv, std::string& expiryDate) {
    std::cout << "=================================================" << std::endl;
    std::cout << "Please input your card details" << std::endl;
    std::cout << "=================================================" << std::endl;
    std::cout << "Input Card Number: ";
    std::cin >> cardNumber;

    std::cout << "Input CVV: ";
    std::cin >> cvv;

    std::cout << "Input Expiry Date (MM/YY): ";
    std::cin >> expiryDate;
    std::cout << "=================================================" << std::endl;
    std::cout << "===================Thanks========================" << std::endl;
    std::cout << "=================================================" << std::endl;

}

/// RSA finished.


int main() {
    terminal myTerminal;
    bank myBank;
    UserData myUser("Bob", "Star", "11 Park Avenue, Switzerland");
    CryptoPP::RSA::PublicKey publicKey;
    CryptoPP::RSA::PrivateKey privateKey;
    generateRSAKeys(publicKey, privateKey);

// tun int
    long long int transaction_id = 6236423672646472;

    std::string card_number, expiry_date;
    int cvv;
    double balance;
    std::string currency;
    int account_nb;

    inputCardDetails(card_number, cvv, expiry_date);

    try {
        Card card(card_number, cvv, expiry_date, currency, account_nb);
        card.check();
    } catch (const std::exception& e) {
        std::cout << e.what() << std::endl;
        return 1;
    }
    
    // Prints:
    std::cout << "=================================================" << std::endl;
    std::cout << "User data is:" << std::endl;
    myUser.DisplayUserInfo();
    std::cout << "=================================================" << std::endl;

    /// transaction data:

    std::cout << "=================================================" << std::endl;
    std::string encryptedCardNumber = encryptCardNumber(card_number, publicKey);
    std::cout << " Encrypted Card Number: " << encryptedCardNumber << std::endl;
    std::cout << "=================================================" << std::endl;
    std::string decryptedCardNumber = decryptCardNumber(encryptedCardNumber, privateKey);
    std::cout << " Decrypted Card Number: " << decryptedCardNumber << std::endl;
    std::cout << "=================================================" << std::endl;
   
    std::cout << "Your transaction information is" << std::endl;
    myTerminal.add_transaction(transaction_id, "USD", "Store Purchase");
    std::cout << "=================================================" << std::endl;
    myTerminal.display_transactions();
    std::cout << "=================================================" << std::endl;
    std::cout << " Compiled successfully!" << std::endl;
    std::cout << "=================================================" << std::endl;
    std::cout <<" The banks keys are" << std::endl;
    std::cout << "=================================================" << std::endl;
    myBank.printKeys(); 
    std::cout <<  std::endl;
     std::cout << "=================================================" << std::endl;
    return 0;
}