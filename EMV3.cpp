#include <iostream>
#include <string>
#include <ctime>
#include <limits>
#include <chrono>
#include <thread>
#include <cstring>
#include <sstream>
#include "cryptopp/rsa.h"
#include "cryptopp/osrng.h"
#include "cryptopp/hex.h"
#include "cryptopp/base64.h"
#include "cryptopp/files.h"
#include "cryptopp/filters.h"
#include "cryptopp/aes.h"
#include "cryptopp/modes.h"
#include "cryptopp/secblock.h"
#include <filesystem>  // Use standard filesystem for C++17
namespace fs = std::filesystem;
//// Global constant:
const int MAX_ATTEMPTS = 3;
const std::string ERROR_MESSAGE = "Authentication failed 3 times. Payment failed.";
const std::string SUCCESS_MESSAGE = "Authentication successful! Proceeding with payment.";
// clear input buffer;
void ClearInputBuffer() {
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

class UserData{ /// changes include replacing C type arras to more secure cpp ones
    public:
    UserData(const std::string& firstName, const std::string& lastName, const std::string& address)
             : fname(firstName), lname(lastName), address(address) {
                acc_num=getLastRowOfColumn()+1;
             }
    void DisplayUserInfo() const{
        std::cout << fname << " " << lname << " residing at " << address << std::endl;
    }
    std::string GetAuthenticationChoice() const {
        int choice;
        std::cout << "Choose your transaction authentication method: press 1 for PIN, 2 for Password, and 3 for TOKEN: ";
        while (true) {
            std::cin >> choice;
            if (std::cin.fail() || (choice != 1 && choice != 2 && choice != 3)) {
                ClearInputBuffer();
                std::cout << "Invalid input. Please press 1 for PIN, 2 for Password, and 3 for TOKEN: ";
            } else {
                break;
            }
        }
        ClearInputBuffer();  // Clear buffer after a valid selection
        switch (choice) {
            case 1: return "PIN";
            case 2: return "Password";
            case 3: return "TOKEN";
            default: return "";  // This should never happen
        }
    }

    // Function to get the PIN from the user
    int GetPIN() const {
        int pin;
        std::cout << "Enter your PIN: ";
        std::cin >> pin;
        ClearInputBuffer();
        return pin;
    }

    // Function to get the Password from the user
    std::string GetPassword() const {
        std::string password;
        std::cout << "Enter your Password: ";
        std::getline(std::cin, password);
        return password;
    }

    // Function to get the Token from the user
    std::string GetToken() const {
        std::string token;
        std::cout << "Enter your Token: ";
        std::getline(std::cin, token);
        return token;
    }

    private: // private for user security.
    std::string fname;
    std::string lname; 
    std::string address;
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
    bool validate=false;

    enum AuthMethod{
        PIN,
        PASSWORD,
        TOKEN
    };
    AuthMethod auth_meth;


    // Masking Function:

    std::string getMaskedCardNumber() const{
        if (card_number.length()<4){
            return std::string(card_number.length(), '*');
        }
        return std::string(card_number.length()-4, '*') + card_number.substr(card_number.length()-4);
    }
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

    //Validate CVV

    void validateCVV(int cvv) const{
        if(cvv < 100 || cvv > 999){
            throw std::invalid_argument("CVV code is incorrect");
        }
    }


    //Validate Experation Date

    void validateExperationDate(const std::string &exp_date) const{
        std::stringstream ss(exp_date);
        int expiry_month, expiry_year;
        char delimiter;
        ss>>expiry_month>>delimiter>>expiry_year;
        if (IsCardExpired(expiry_month, expiry_year)){
            throw std::runtime_error("Your card is expired");
        }
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
        bool alternate = true;
        // Iterate over card number from the rightmost digit
        for (int i = 0; i < card_number.length(); ++i) {
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

        std::string auth_meth; // 1,2,3
        std::string authenticator;
            //if (auth_meth == 1) --> PIN, == 2 --> password, == 3 --> token
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

    // Card Getter functions
    std::string getCardNumber() const {
        return card_number;
    }
    std::string getAccountNb() const {
        return std::to_string(account_nb);
    }
    std::string getCVV() const {
        return std::to_string(cvv);
    }
    std::string getExpDate() const {
        return exp_date;
    }
    std::string getCurrency() const {
        return currency;
    }
    std::string getBalance() const {
        return std::to_string(balance);
    }
};

class bank {
    private:
        CryptoPP::RSA::PublicKey publicKey;
        CryptoPP::RSA::PrivateKey privateKey;
        CryptoPP::SecByteBlock secretKey = GenerateAESKey(AES_256_KEY_SIZE);
        std::string encSecretKey = encryptRSA(std::string(reinterpret_cast<const char*>(secretKey.data(), secretKey.size())), publicKey);
        const int correctPin = 1234;                 // Correct PIN (in a real app, these values would be securely stored)
        const std::string correctPassword = "Password123";
        const std::string correctToken = "Token123";
    public:
        // validators:

    bool ValidatePIN(int userPin) const {
        return userPin == correctPin;
    }

    // Function to validate Password
    bool ValidatePassword(const std::string& userPassword) const {
        return userPassword == correctPassword;
    }

    // Function to validate Token
    bool ValidateToken(const std::string& userToken) const {
        return userToken == correctToken;
    }

    // Function to handle the overall authentication process
    bool Authenticate(const std::string& method, const UserData& user) {
        int attempts = 0;
        while (attempts < MAX_ATTEMPTS) {
            attempts++;
            bool success = false;

            if (method == "PIN") {
                int userPin = user.GetPIN();
                success = ValidatePIN(userPin);
            } else if (method == "Password") {
                std::string userPassword = user.GetPassword();
                success = ValidatePassword(userPassword);
            } else if (method == "TOKEN") {
                std::string userToken = user.GetToken();
                success = ValidateToken(userToken);
            }

            if (success) {
                std::cout << SUCCESS_MESSAGE << std::endl;
                return true;
            } else {
                std::cout << "Authentication failed." << std::endl;
            }

            if (attempts < MAX_ATTEMPTS) {
                std::cout << "Please try again. You have " << MAX_ATTEMPTS - attempts << " attempt(s) remaining.\n";
            }
        }

        std::cout << ERROR_MESSAGE << std::endl;
        return false;
    }
        bank (){
            generateBankRSAKeys();
        }

        void generateBankRSAKeys() {
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
        std::cout <<"==================================================" << std::endl;
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
        //customer.SaveToCSV();
        customer.DisplayUserInfo();
    }

    std::string balanceChange(const std::string card_number, double cost) {
        std::string filename = "card_data.csv";
        std::ifstream file_in(filename);
        if (!file_in.is_open()) {
            std::cerr << "Erroring opening file!" << std::endl;
        }

        std::vector<std::string> file_content;
        std::string line;
        bool found = false;
        
        while (getline(file_in, line)) {
            std::vector<std::string> values = split(line, ',');

            if (values.size() > 1 && values[0] == card_number) {
                found = true;
                double balance = std::stoi(values[5]);
                if (balance <= cost) {
                    return "There is not enough money in the card!";
                }
                else {
                    values[5] = balance - cost; 
                }
            }

            file_content.push_back(join(values, ','));
        }

        file_in.close();

        if (!found) {
            return "Card number not valid.";
        }

        std::ofstream file_out(filename, std::ios::trunc);
        if (!file_out.is_open()) {
            std::cerr << "Error opening file to write!" << std::endl;
        }

        for (const auto& line : file_content) {
            file_out << line << "\n";
        }

        file_out.close();

        return "Transaction was successful";
    }


    bool checkCardDetail(const Card& card) {
        std::string filename = "card_data.csv";
        std::ifstream file_in(filename);
        if (!file_in.is_open()) {
            std::cerr << "Erroring opening file!" << std::endl;
        }

        std::vector<std::string> file_content;
        std::string line;
        bool found = false;
        
        while (getline(file_in, line)) {
            std::vector<std::string> values = split(line, ',');
            if (values[0] == card.getCardNumber() && values[1] == card.getAccountNb() && values[2] == card.getCVV() &&
                values[3] == card.getExpDate() && values[4] == card.getCurrency()) {
                found = true;
                break;
            }
        }
        return found;
    }

    std::vector<std::string> split(const std::string& line, char delimiter) {
        std::vector<std::string> values;
        std::string value;
        std::stringstream ss(line);
        
        while (getline(ss, value, delimiter)) {
            values.push_back(value);
        }
        
        return values;
    }

    std::string join(const std::vector<std::string>& values, char delimiter) {
        std::string result;
        for (size_t i = 0; i < values.size(); ++i) {
            if (i != 0) result += delimiter;
            result += values[i];
        }

        return result;
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
            std::string transaction_type;
            std::string transaction_date;  // "Contactless" or "Contact"

            // Constructor
            Transaction(long long int tun, const std::string& curr, const std::string& d, const std::string& t_type, const std::string& date)
            : TUN(tun), currency(curr), data(d), transaction_type(t_type), transaction_date(date) {}

        };

        // List to hold all transactions
        std::list<Transaction> transaction_data;

    public:
        // Constructor
        terminal() {
            // Perform initialization if necessary
        }

        // Add a transaction (with user input for transaction type: "Contactless" or "Contact")
        void add_transaction(long long int tun, const std::string& curr, const std::string& data, std::string& date) {
            std::string transaction_type = selectTransactionType();  // Get transaction type from user
            Transaction new_transaction(tun, curr, data, transaction_type, date);
            transaction_data.push_back(new_transaction);
        }

        // Display all transactions
        void display_transactions() const {
            for (const auto& trans : transaction_data) {
                std::cout << "=================================================" << std::endl;
                std::cout << "=========Your transaction information is :=======" << std::endl;
                std::cout << "TUN: " << trans.TUN << ", Currency: " << trans.currency << ", Transaction location: " << trans.data 
                << ", Transaction Type: " << trans.transaction_type << ", Transaction Date: " << trans.transaction_date << std::endl;
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

std::string encryptRSA(const std::string &plaintext, CryptoPP::RSA::PublicKey &publicKey) {
    CryptoPP::AutoSeededRandomPool rng;
    std::string cipher;
    CryptoPP::RSAES_OAEP_SHA_Encryptor encryptor(publicKey);
    CryptoPP::StringSource ss1(plaintext, true, new CryptoPP::PK_EncryptorFilter(rng, encryptor, new CryptoPP::StringSink(cipher)));
    return cipher;
}

std::string decryptRSA(const std::string &cipher, CryptoPP::RSA::PrivateKey &privateKey) {
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

//AES Key Generation Function

const int AES_128_KEY_SIZE = CryptoPP::AES::DEFAULT_KEYLENGTH;
const int AES_192_KEY_SIZE = 24;
const int AES_256_KEY_SIZE = CryptoPP::AES::MAX_KEYLENGTH;

CryptoPP::SecByteBlock GenerateAESKey(int key_length) {
    if (key_length != AES_128_KEY_SIZE && key_length != AES_192_KEY_SIZE && key_length != AES_256_KEY_SIZE) {
        throw std::invalid_argument("Invalid AES key length, please choose between 128, 192, 256 bits.");
    }

    CryptoPP::SecByteBlock key(key_length);

    CryptoPP::AutoSeededRandomPool rng;
    rng.GenerateBlock(key, key.size());

    return key;
}

//AES Encryption Function

std::string encryptValue(const std::string &plaintext, const CryptoPP::SecByteBlock &key, const CryptoPP::SecByteBlock &iv){
    std::string cipher;
    CryptoPP::AES::Encryption aesEncryption(key, key.size());
    CryptoPP::CBC_Mode_ExternalCipher::Encryption cbcEncryption(aesEncryption, iv);

    CryptoPP::StringSource ss(plaintext,true, new CryptoPP::StreamTransformationFilter(cbcEncryption, new CryptoPP::StringSink(cipher)));
    return cipher;
}

//AES Decryption Function

std::string decryptValue(const std::string &cipher, const CryptoPP::SecByteBlock &key, const CryptoPP::SecByteBlock& iv) {
    std::string decrypted;
    CryptoPP::AES::Decryption aesDecryption(key, key.size());
    CryptoPP::CBC_Mode_ExternalCipher::Decryption cbcDecription(aesDecryption, iv);

    CryptoPP::StringSource ss(cipher,true, new CryptoPP::StreamTransformationFilter(cbcDecription, new CryptoPP::StringSink(decrypted)));
    return decrypted;
}

// Read, Write and Encrypt CSV
std::vector<std::vector<std::string>> readCSV(const std::string& filename) {
    std::ifstream file(filename);
    std::vector<std::vector<std::string>> data;
    std::string line;

    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string value;
        std::vector<std::string> row;
        while (std::getline(ss, value, ',')) {
            row.push_back(value);
        }
        data.push_back(row);
    }
    
    file.close();
    return data;
}

void writeCSV(const std::string& filename, const std::vector<std::vector<std::string>>& data) {
    std::ofstream file(filename);
    for (const auto& row : data) {
        for (size_t i = 0; i < row.size(); ++i) {
            file << row[i];
            if (i != row.size() - 1) {
                file << ",";
            }
        }
        file << "\n";
    }
    file.close();
}

void encryptCSV(const std::string& inputFilename, const std::string& outputFilename, const CryptoPP::SecByteBlock& key, const CryptoPP::SecByteBlock& iv) {
    std::vector<std::vector<std::string>> data = readCSV(inputFilename);

    // Encrypt each value in the CSV
    for (auto& row : data) {
        for (auto& value : row) {
            value = encryptValue(value, key, iv);
        }
    }

    writeCSV(outputFilename, data);
}

int main() {
    terminal myTerminal;
    bank myBank;
    UserData myUser("Bob", "Star", "11 Park Avenue, Switzerland");
    CryptoPP::RSA::PublicKey publicKey;
    CryptoPP::RSA::PrivateKey privateKey;
    generateRSAKeys(publicKey, privateKey);

// tun int
    long long int transaction_id = 6236423672646472;
    std::string transaction_date = "20/05/2024";
    std::string card_number, expiry_date;
    int cvv;
    double balance;
    std::string currency;
    int account_nb;
    

    std::cout << "=================================================" << std::endl;
    std::cout << "======Please Select Your payment method==========" << std::endl;
    std::cout << "=================================================" << std::endl;

    myTerminal.add_transaction(transaction_id, "USD", "Store Purchase", transaction_date); 

    std::cout << "=================================================" << std::endl;
    std::string authMethod = myUser.GetAuthenticationChoice();
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
    std::cout << "User data is: " << std::endl;
    myUser.DisplayUserInfo(); // shows information about the user.
    myTerminal.display_transactions();    /// shows TUN and info.
    std::cout << "=================================================" << std::endl;
    /// transaction data:
    std::string encryptedCardNumber = encryptRSA(card_number, publicKey);
    std::cout << " Encrypted Card Number: " << encryptedCardNumber << std::endl;
    std::cout << "=================================================" << std::endl;
    std::string decryptedCardNumber = decryptRSA(encryptedCardNumber, privateKey);
    std::cout << " Decrypted Card Number: " << decryptedCardNumber << std::endl;
    std::cout << "=================================================" << std::endl;
   /// now athentication method
    
    
    bool isAuthenticated = myBank.Authenticate(authMethod, myUser);
        if (isAuthenticated) {
            std::cout << "Payment successful!" << std::endl;
        } else {
            std::cout << "Too many failed attempts. Please try again later.\n";
            std::this_thread::sleep_for(std::chrono::seconds(5));  // Simulate lockout
        }

    std::cout << "=================================================" << std::endl;
    std::cout << " Compiled successfully!" << std::endl;
    std::cout << "=================================================" << std::endl;
    std::cout <<" The banks keys are" << std::endl;
    std::cout <<"==================================================" << std::endl;
    myBank.printKeys(); 
    std::cout <<"==================================================" << std::endl;
    return 0;
}