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
#include "cryptopp/aes.h"
#include "cryptopp/modes.h"
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
            file << "Account Number,First Name,Last Name,Address\n";  // Add headers
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
    
///maybe do getter class
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
    // std::string getAuthenticator() const {
    //     return Authenticator;
    // }

};

class bank {
private:
   //std::string &pk;
   // generate
   // std::string &sk;

public:
    void CreatUser() {
        std::string fname;
        std::cout << "Enter first name: " << std::endl;
        std::cin >> fname;

        std::string lname;
        std::cout << "Enter last name: " << std::endl;
        std::cin >> lname;

        // std::string currency;
        // std::cout << "Enter currency: " << std::endl;
        // std::cin >> currency;

        std::string address;
        std::cout << "Enter address: " << std::endl;
        std::cin >> address;

        // double balance = 0;

        UserData customer(fname, lname,address);
        customer.SaveToCSV();
        customer.DisplayUserInfo();
    }


    /*
    read and write the data from csv file (geters and setters)
    fetch the data
    bank should call encryption and decryption functions
    then check the functions
    respond to terminal  
    hash function that hash the passowords in the terminal and the bank 
    compare the hash values inside the terminal 
    
    private key only for the bank and public key for everyone
    */
    
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
    //public:
        //bank(const std::string& );
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
    std::cout << "Input Card Number: ";
    std::cin >> cardNumber;

    std::cout << "Input CVV: ";
    std::cin >> cvv;

    std::cout << "Input Expiry Date (MM/YY): ";
    std::cin >> expiryDate;
}

/// RSA finished.

//AES Encryption Function

std::string encryptValue(const std::string &plaintext, const CryptoPP::SecByteBlock &key, const CryptoPP::SecByteBlock &iv){
    std::string cipher;
    CryptoPP::AES::Encryption aesEncryption(key, CryptoPP::AES::DEFAULT_KEYLENGTH);
    CryptoPP::CBC_Mode_ExternalCipher::Encryption cbcEncryption(aesEncryption, iv);

    CryptoPP::StringSource ss(plaintext,true, new CryptoPP::StreamTransformationFilter(cbcEncryption, new CryptoPP::StringSink(cipher)));
    return cipher;
}

//AES Decryption Function

std::string decryptValue(const std::string &cipher, const CryptoPP::SecByteBlock &key, const CryptoPP::SecByteBlock& iv) {
    std::string decrypted;
    CryptoPP::AES::Decryption aesDecryption(key, CryptoPP::AES::DEFAULT_KEYLENGTH);
    CryptoPP::CBC_Mode_ExternalCipher::Decryption cbcDecription(aesDecryption, iv);

    CryptoPP::StringSource ss(cipher,true, new CryptoPP::StreamTransformationFilter(cbcDecription, new CryptoPP::StringSink(decrypted)));
    return decrypted;
}

// PaymenyMethod
void selectPaymentMethod() {
    int paymentMethod;
    std::cout << "Hi, customer. What type of method would you like to use? Choose by pressing 1 [contact] or 0 [contactless]: ";

    while (true) {
        std::cin >> paymentMethod;

        if (std::cin.fail() || (paymentMethod != 0 && paymentMethod != 1)) {
            std::cin.clear(); 
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); 
            std::cout << "Invalid input. Please enter 1 or 0: ";
        } else {
            break;
        }
    }

    std::cout << "Your selected payment method is: " << (paymentMethod == 1 ? "Contact" : "Contactless") << std::endl;
}
// Payment method end. 

int main() {
    CryptoPP::RSA::PublicKey publicKey;
    CryptoPP::RSA::PrivateKey privateKey;
    generateRSAKeys(publicKey, privateKey);

    std::string card_number, expiry_date;
    int cvv;
    double balance;
    std::string currency;
    inputCardDetails(card_number, cvv, expiry_date);

    try {
        Card card(card_number, cvv, expiry_date, balance, currency);
        card.check();
    } catch (const std::exception& e) {
        std::cout << e.what() << std::endl;
        return 1;
    }

    UserData user("Bob", "Star", "11 Park Avenue, Switzerland");
    user.DisplayUserInfo();

    selectPaymentMethod();

    std::string encryptedCardNumber = encryptCardNumber(card_number, publicKey);
    std::cout << "Encrypted Card Number: " << encryptedCardNumber << std::endl;

    std::string decryptedCardNumber = decryptCardNumber(encryptedCardNumber, privateKey);
    std::cout << "Decrypted Card Number: " << decryptedCardNumber << std::endl;


    std::cout << "Compiled successfully!" << std::endl;
    return 0;
}

//modify the getters and return the encrypted values using symetric encryption