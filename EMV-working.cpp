#include <iostream>
#include <string>
#include <random>
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
#include "cryptopp/pwdbased.h"
#include "cryptopp/sha.h"
#include <filesystem>  // Use standard filesystem for C++17
namespace fs = std::filesystem;
//// Global constant:
const int MAX_ATTEMPTS = 3;
const std::string ERROR_MESSAGE = "Authentication failed 3 times. Payment failed.";
const std::string SUCCESS_MESSAGE = "Authentication successful! Proceeding with payment.";
const int AES_128_KEY_SIZE = CryptoPP::AES::DEFAULT_KEYLENGTH;
const int AES_192_KEY_SIZE = 24;
const int AES_256_KEY_SIZE = CryptoPP::AES::MAX_KEYLENGTH;
CryptoPP::RSA::PublicKey BANK_PUBLIC_KEY{};
CryptoPP::SecByteBlock AES_IV{};

/// RSA ALGORITHM START
// RSA key generation and encryption/decryption functions

void generateRSAKeys(CryptoPP::RSA::PublicKey &publicKey, CryptoPP::RSA::PrivateKey &privateKey) {
    CryptoPP::AutoSeededRandomPool rng;
    privateKey.GenerateRandomWithKeySize(rng, 4096);
    publicKey.AssignFrom(privateKey);
}

std::string encryptRSA(const std::string &plaintext, CryptoPP::RSA::PublicKey &publicKey) {
    CryptoPP::AutoSeededRandomPool rng;
    std::string cipher;
    CryptoPP::RSAES_OAEP_SHA256_Encryptor encryptor(publicKey);
    CryptoPP::StringSource ss1(plaintext, true, new CryptoPP::PK_EncryptorFilter(rng, encryptor, new CryptoPP::StringSink(cipher)));
    std::string encodedCipher;
    CryptoPP::StringSource ss4(cipher, true, new CryptoPP::Base64Encoder(new CryptoPP::StringSink(encodedCipher)));
    
    return encodedCipher;
}

std::string decryptRSA(const std::string &encodedCipher, CryptoPP::RSA::PrivateKey &privateKey) {
    CryptoPP::AutoSeededRandomPool rng;
    std::string cipher, recovered;
    CryptoPP::StringSource ss4(encodedCipher, true, new CryptoPP::Base64Decoder(new CryptoPP::StringSink(cipher)));
    CryptoPP::RSAES_OAEP_SHA256_Decryptor decryptor(privateKey);
    CryptoPP::StringSource ss1(cipher, true, new CryptoPP::PK_DecryptorFilter(rng, decryptor, new CryptoPP::StringSink(recovered)));
    
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

CryptoPP::SecByteBlock GenerateAESKey(int key_length) {
    if (key_length != AES_128_KEY_SIZE && key_length != AES_192_KEY_SIZE && key_length != AES_256_KEY_SIZE) {
        throw std::invalid_argument("Invalid AES key length, please choose between 128, 192, 256 bits.");
    }

    CryptoPP::SecByteBlock key(key_length);

    CryptoPP::AutoSeededRandomPool rng;
    rng.GenerateBlock(key, key.size());

    return key;
}

CryptoPP::SecByteBlock GenerateAESIV() {
    CryptoPP::SecByteBlock iv(CryptoPP::AES::BLOCKSIZE);
    CryptoPP::AutoSeededRandomPool rng;
    rng.GenerateBlock(iv, iv.size());
    return iv;
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

// Hash function with PBKDF2
std::string HashPass(std::string pass) {
    std::string hash_value;

    CryptoPP::SHA384 hash;

    CryptoPP::StringSource(pass, true, new CryptoPP::HashFilter(hash, new CryptoPP::HexEncoder(new CryptoPP::StringSink(hash_value), true)));

    return hash_value;
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
        std::cout << "Choose your transaction authentication method: press 1) for PIN or 2) for Password: ";
        while (true) {
            std::cin >> choice;
            if (std::cin.fail() || (choice != 1 && choice != 2)) {
                ClearInputBuffer();
                std::cout << "Invalid input. Please press 1) for PIN or 2) for Password: ";
            } else {
                break;
            }
        }
        ClearInputBuffer();  // Clear buffer after a valid selection
        switch (choice) {
            case 1: return "PIN";
            case 2: return "Password";
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
        PASSWORD
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
    //Validate Experation Date

    void validateExpirationDate(const std::string &exp_date) const{
        std::stringstream ss(exp_date);
        int expiry_month, expiry_year;
        char delimiter;
        ss>>expiry_month>>delimiter>>expiry_year;
        if (IsCardExpired(expiry_month, expiry_year)){
            throw std::runtime_error("Your card is expired");
        }
    }
    
    Card(const std::string &in_card_number, const int &in_cvv, const std::string &in_exp_date, const std::string & in_currency, const int &in_account_nb) 
        : card_number(in_card_number), cvv(in_cvv), exp_date(in_exp_date), currency(in_currency), account_nb(in_account_nb) {
        if (cvv < 100 || cvv > 999){  // check length using three digits.
            throw std::invalid_argument("CVV code is incorrect"); // error if ccv is incorrect
        }

        std::stringstream ss(exp_date);
        int expiry_month, expiry_year;
        char delimiter;
        ss >> expiry_month >> delimiter >> expiry_year;

        // validateExpirationDate(in_exp_date);
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
        return encryptRSA(card_number, BANK_PUBLIC_KEY);
    }
    std::string getAccountNb() const {
        return encryptRSA(std::to_string(account_nb), BANK_PUBLIC_KEY);
    }
    std::string getCVV() const {
        return encryptRSA(std::to_string(cvv), BANK_PUBLIC_KEY);
    }
    std::string getExpDate() const {
        return encryptRSA(exp_date, BANK_PUBLIC_KEY);
    }
    std::string getCurrency() const {
        return encryptRSA(currency, BANK_PUBLIC_KEY);
    }
    std::string getBalance() const {
        return encryptRSA(std::to_string(balance), BANK_PUBLIC_KEY);
    }
};

class bank {
    private:
        private:
        CryptoPP::RSA::PublicKey publicKey;
        CryptoPP::RSA::PrivateKey privateKey;
        CryptoPP::SecByteBlock secretKey = GenerateAESKey(AES_256_KEY_SIZE);
        CryptoPP::SecByteBlock iv = GenerateAESIV();
        const int correctPin = 1234;                 // Correct PIN (in a real app, these values would be securely stored)
        const std::string correctPassword = "Password123";

    public:

    struct CardDetails {
        std::string card_no;
        std::string acc_no;
        std::string card_cvv;
        std::string card_expDate;
        std::string card_currency;
        std::string card_auth_meth;
        std::string card_auth;
        std::string card_bal;
    };

    std::vector<CardDetails> encDB;

    void setterSecretKey() {
        secretKey = GenerateAESKey(AES_256_KEY_SIZE);
    }
        // validators:

    bool ValidatePIN(int userPin) const {
        return userPin == correctPin;
    }

    // Function to validate Password
    bool ValidatePassword(const std::string& userPassword) const {
        return userPassword == correctPassword;
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

    bank () {
        generateBankRSAKeys();
        BANK_PUBLIC_KEY = GetPublicKey();
        AES_IV = GetIV();
        bankEncryptCSV("credit_card.csv");
    }

    CryptoPP::RSA::PublicKey GetPublicKey() {
        return publicKey;
    }

    CryptoPP::SecByteBlock GetIV() {
        return iv;
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

    // Get the authentication method from the card details of the provided card
    std::string GetCardAuthMeth(Card &card) {
        std::string decCardNo = decryptRSA(card.getCardNumber(), privateKey);
        for (CardDetails each : encDB) {
            std::string tmp = decryptValue(each.card_no, secretKey, iv);

            if (tmp == decCardNo) {
                return decryptValue(each.card_auth_meth, secretKey, iv);
            }
        }

        return "Invalid card number!";
    }

    // Verifies the PIN or PASS of the card
    bool verify_pass(Card &card, std::string pass) {
        bool verified = false;
        std::string decCardNo = decryptRSA(card.getCardNumber(), privateKey);
        for (CardDetails each : encDB) {
            std::string tmp = decryptValue(each.card_no, secretKey, iv);

            if (tmp == decCardNo) {
                std::string decPass = decryptValue(each.card_auth, secretKey, iv);
                std::cout << decPass << std::endl;
                if (HashPass(decPass) == pass) {
                    verified = true;
                    break;
                    // std::cout << "Authentication successful" << std::endl;
                }
                // else {
                //     std::cout << "Payment DECLINED! Please start over" << std::endl;
                // }
            }
        }

        return verified;
    }

    // std::string balanceChange(const std::string card_number, double cost) {
    //     std::string filename = "card_data.csv";
    //     std::ifstream file_in(filename);
    //     if (!file_in.is_open()) {
    //         std::cerr << "Erroring opening file!" << std::endl;
    //     }

    //     std::vector<std::string> file_content;
    //     std::string line;
    //     bool found = false;
        
    //     while (getline(file_in, line)) {
    //         std::vector<std::string> values = split(line, ',');

    //         if (values.size() > 1 && values[0] == card_number) {
    //             found = true;
    //             double balance = std::stoi(values[5]);
    //             if (balance <= cost) {
    //                 return "There is not enough money in the card!";
    //             }
    //             else {
    //                 values[5] = balance - cost; 
    //             }
    //         }

    //         file_content.push_back(join(values, ','));
    //     }

    //     file_in.close();

    //     if (!found) {
    //         return "Card number not valid.";
    //     }

    //     std::ofstream file_out(filename, std::ios::trunc);
    //     if (!file_out.is_open()) {
    //         std::cerr << "Error opening file to write!" << std::endl;
    //     }

    //     for (const auto& line : file_content) {
    //         file_out << line << "\n";
    //     }

    //     file_out.close();

    //     return "Transaction was successful";
    // }


    bool checkCardDetail(const Card& card) {
        std::string cardNum = decryptRSA(card.getCardNumber(), privateKey);
        std::string accNum = decryptRSA(card.getAccountNb(), privateKey);
        std::string cardCVV = decryptRSA(card.getCVV(), privateKey);
        std::string cardExpD = decryptRSA(card.getExpDate(), privateKey);
        std::string cardCurr = decryptRSA(card.getCurrency(), privateKey);

        card.validateExpirationDate(cardExpD);

        bool found = false;

        for (CardDetails each : encDB) {
            std::vector<std::string> details(8);
            details[0] = decryptValue(each.card_no, secretKey, iv);
            details[1] = decryptValue(each.acc_no, secretKey, iv);
            details[2] = decryptValue(each.card_cvv, secretKey, iv);
            details[3] = decryptValue(each.card_expDate, secretKey, iv);
            details[4] = decryptValue(each.card_currency, secretKey, iv);
            details[5] = decryptValue(each.card_auth_meth, secretKey, iv);
            details[6] = decryptValue(each.card_auth, secretKey, iv);
            details[7] = decryptValue(each.card_bal, secretKey, iv);

            if (details[0] == cardNum && details[1] == accNum && details[2] == cardCVV &&
                details[3] == cardExpD && details[4] == cardCurr) {
                found = true;
                break;
                }
        }

        // std::cout << "card checked" << std::endl;
        return found;
    }

    void bankEncryptCSV(std::string filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Error opening file: " << filename << std::endl;
            return;
        }

        std::string value;
        std::string line;
        
        
        while (std::getline(file, line)) {
            std::vector<std::string> values;
            std::stringstream ss(line);

            std::string card_no, acc_no, card_cvv, card_expDate, card_currency, card_auth_meth, card_auth, card_bal;

            while (std::getline(ss, value, ',')) {
                values.push_back(value);
            }

            card_no = encryptValue(values[0], secretKey, iv);
            acc_no = encryptValue(values[1], secretKey, iv);
            card_cvv = encryptValue(values[2], secretKey, iv);
            card_expDate = encryptValue(values[3], secretKey, iv);
            card_currency = encryptValue(values[4], secretKey, iv);
            card_auth_meth = encryptValue(values[5], secretKey, iv);
            card_auth = encryptValue(values[6], secretKey, iv);
            card_bal = encryptValue(values[7], secretKey, iv);

            CardDetails encCard = {card_no, acc_no, card_cvv, card_expDate, card_currency, card_auth_meth, card_auth, card_bal};

            encDB.push_back(encCard);
        }

        file.close();
    }
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

        long long int generateTUN(){
            std::random_device rd;
            std::mt19937_64 gen(rd());
            std::uniform_int_distribution<long long int> dist(1000000000000000, 9999999999999999);
            return dist(gen);
        }

        // Checking the Authentication Method and validate it through bank
        bool validate_authentication(bank bank, Card &card) {
            bool valid = false;
            std::string authM = bank.GetCardAuthMeth(card);
            if (authM == "PIN") {
                std::string pin;
                std::cout << "Enter your PIN: ";
                std::cin >> pin;
                if (bank.verify_pass(card, HashPass(pin))) {
                    valid = true;
                }
            }
            else if (authM == "PASS") {
                std::string pass;
                std::cout << "Enter your Password: ";
                std::cin >> pass;
                if (bank.verify_pass(card, HashPass(pass))){
                    valid = true;
                }
            }

            return valid;
        }

        // Add a transaction (with user input for transaction type: "Contactless" or "Contact")
        void add_transaction(const std::string& curr, const std::string& data, std::string& date) {
            long long int tun = generateTUN();
            std::string transaction_type = selectTransactionType();  // Get transaction type from user
            Transaction new_transaction(tun, curr, data, transaction_type, date);
            transaction_data.push_back(new_transaction);
        }

        // Display all transactions
        void display_transactions() const {
            for (const auto& trans : transaction_data) {
                std::cout << "=================================================" << std::endl;
                std::cout << "=========Your transaction information is :=======" << std::endl;
        
                std::cout << "TUN: " << trans.TUN 
                  << ", Currency: " << trans.currency 
                  << ", Transaction location: " << trans.data 
                  << std::endl;
                std::cout << "Transaction Type: " << trans.transaction_type 
                << ", Transaction Date: " << trans.transaction_date << std::endl;
        
                // Add a space line for consistent length
            }
        }
        // Function to select transaction type ("Contactless" or "Contact")
        std::string selectTransactionType() {
            int transactionType;
            std::cout << " Choose 1) for Contactless or 2) for Contact: ";

            while (true) {
                std::cin >> transactionType;

                // Check if the input is valid
                if (std::cin.fail() || (transactionType != 1 && transactionType != 2)) {
                    std::cin.clear();  // Clear the error flag on cin
                    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');  // Ignore invalid input
                    std::cout << "Invalid input. Please enter 1) for Contactless or 2) for Contact: ";
                } else {
                    break;
                }
            }

            // Return the transaction type as a string
            return (transactionType == 1) ? "Contactless" : "Contact";
        }
};


int main() {
    terminal myTerminal;
    bank myBank;
    UserData myUser("Bob", "Star", "11 Park Avenue, Switzerland");
    // CryptoPP::RSA::PublicKey publicKey;
    // CryptoPP::RSA::PrivateKey privateKey;
    // generateRSAKeys(publicKey, privateKey);

    // Create two card objects, one valid and one invalid
    Card validCard("4716893064521783", 234, "11/26", "EUR", 12345679 );  // Valid card
    Card invalidCard("987654321234567",191, "01/28","YEN", 12345678); // Invalid card (incorrect length for card number)


    //Check if validCard details are correct:

    bool isValidCard = myBank.checkCardDetail(validCard);
    if (isValidCard) {
        std::cout << "Valid card details found!" << std::endl;
    } else {
        std::cout << "Valid card details are not correct." << std::endl;
    }

    // Check if invalidCard details are correct
    bool isInvalidCard = myBank.checkCardDetail(invalidCard);
    if (isInvalidCard) {
        std::cout << "Invalid card details found! (Should not happen)" << std::endl;
    } else {
        std::cout << "Invalid card details are not correct." << std::endl;
    }


// tun int
    
    std::string transaction_date = "20/05/2024";
    // std::string card_number, expiry_date;
    // int cvv;
    // double balance;
    // std::string currency;
    // int account_nb;
    

    std::cout << "=================================================" << std::endl;
    std::cout << "======Please Select Your payment method==========" << std::endl;
    std::cout << "=================================================" << std::endl;

    myTerminal.add_transaction("USD", "Store Purchase", transaction_date); 

    std::cout << "=================================================" << std::endl;
    if (myTerminal.validate_authentication(myBank, validCard)) {
        std::cout << "valid" << std::endl;
    }
    else {
        std::cout << "invalid" << std::endl;
    }
    // std::string authMethod = myUser.GetAuthenticationChoice();
    // inputCardDetails(card_number, cvv, expiry_date);
    // try {
    //     Card card(card_number, cvv, expiry_date, currency, account_nb);
    //     card.check();
    // } catch (const std::exception& e) {
    //     std::cout << e.what() << std::endl;
    //     return 1;
    // }
    // Prints:
    std::cout << "=================================================" << std::endl;
    std::cout << "User data is: " << std::endl;
    // myUser.DisplayUserInfo(); // shows information about the user.
    myTerminal.display_transactions();    /// shows TUN and info.
    std::cout << "=================================================" << std::endl;
    /// transaction data:
    // std::string encryptedCardNumber = encryptRSA(card_number, publicKey);
    // std::cout << " Encrypted Card Number: " << encryptedCardNumber << std::endl;
    std::cout << "=================================================" << std::endl;
    // std::string decryptedCardNumber = decryptRSA(encryptedCardNumber, privateKey);
    // std::cout << " Decrypted Card Number: " << decryptedCardNumber << std::endl;
    std::cout << "=================================================" << std::endl;
   /// now athentication method
    
    
    // bool isAuthenticated = myBank.Authenticate(authMethod, myUser);
    //     if (isAuthenticated) {
    //         std::cout << "Payment successful!" << std::endl;
    //     } else {
    //         std::cout << "Too many failed attempts. Please try again later.\n";
    //         std::this_thread::sleep_for(std::chrono::seconds(5));  // Simulate lockout
    //     }

    std::cout << "=================================================" << std::endl;
    std::cout << " Compiled successfully!" << std::endl;
    std::cout << "=================================================" << std::endl;
    std::cout <<" The banks keys are" << std::endl;
    std::cout <<"==================================================" << std::endl;
    // myBank.printKeys(); 
    std::cout <<"==================================================" << std::endl;
    return 0;
};