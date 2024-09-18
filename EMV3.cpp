#include <iostream>
#include <string>
#include <ctime>
#include <limits>
#include <cstring>
#include <sstream>
// #include "cryptopp/rsa.h"
// #include "cryptopp/osrng.h"
// #include "cryptopp/hex.h"
// #include "cryptopp/base64.h"
// #include "cryptopp/files.h"
// #include "cryptopp/filters.h"




class UserData{ /// changes include replacing C type arras to more secure cpp ones
    public:
    
    UserData(const std::string& firstName, const std::string& lastName, double balance, const std::string& address, const std::string& currency)
             : fname(firstName), lname(lastName), balance(balance), address(address), location_currency(currency) {}
    void DisplayUserInfo() const{
        std::cout << fname << " " << lname << " " << " has " <<  balance << "  " << location_currency << " residing at " << address << std::endl;
    }

    private: // private for user security.
    std::string fname;
    std::string lname;
    double balance; 
    std::string address;
    std::string location_currency;
};
    
///maybe do getter class
// card dummy

class Card { // const kept, even in private for greater security.

private:
    const std::string card_number;
    const std::string cvv;
    const std::string exp_date;
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
   
   bool ValidatedLunh() const {
    int sum = 0; // application of lunhs card recognition algorithm
    bool alternate = false;
    for (int i = card_number.length() - 1; i >= 0; i--) { //add explanation when you guys can.
            int n = card_number[i] - '0';
            if (alternate) {
                n *= 2;
                if (n > 9) n -= 9;
            }
            sum += n;
            alternate = !alternate;
        }
        return (sum % 10 == 0);
    }
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
    Card(const std::string &in_card_number, const std::string &in_cvv, const std::string &in_exp_date) 
        : card_number(in_card_number), cvv(in_cvv), exp_date(in_exp_date) {
        if (cvv.length() != 3) { // check length
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
        if (card_number.length() < 13) {
            std::cout << "Card number is too short" << std::endl; /// error if card is too short
            return;
        }

        if (ValidatedLunh()) {
            std::cout << "Card Number is VALID" << std::endl;  /// valid cards
            std::cout << "Card Type: " << DetermineCardType() << std::endl; // calling function 
        } else {
            std::cout << "Card Number is INVALID!" << std::endl; // error handling
        }
    }
};

/// RSA ALGORITHM START
// RSA key generation and encryption/decryption functions

// void generateRSAKeys(CryptoPP::RSA::PublicKey &publicKey, CryptoPP::RSA::PrivateKey &privateKey) {
//     CryptoPP::AutoSeededRandomPool rng;
//     privateKey.GenerateRandomWithKeySize(rng, 2048);
//     publicKey.AssignFrom(privateKey);
// }

// std::string encryptCardNumber(const std::string &cardNumber, CryptoPP::RSA::PublicKey &publicKey) {
//     CryptoPP::AutoSeededRandomPool rng;
//     std::string cipher;
//     CryptoPP::RSAES_OAEP_SHA_Encryptor encryptor(publicKey);
//     CryptoPP::StringSource ss1(cardNumber, true, new CryptoPP::PK_EncryptorFilter(rng, encryptor, new CryptoPP::StringSink(cipher)));
//     return cipher;
// }

// std::string decryptCardNumber(const std::string &cipher, CryptoPP::RSA::PrivateKey &privateKey) {
//     CryptoPP::AutoSeededRandomPool rng;
//     std::string recovered;
//     CryptoPP::RSAES_OAEP_SHA_Decryptor decryptor(privateKey);
//     CryptoPP::StringSource ss4(cipher, true, new CryptoPP::PK_DecryptorFilter(rng, decryptor, new CryptoPP::StringSink(recovered)));
//     return recovered;
// }

void inputCardDetails(std::string& cardNumber, std::string& cvv, std::string& expiryDate) {
    std::cout << "Input Card Number: ";
    std::cin >> cardNumber;

    std::cout << "Input CVV: ";
    std::cin >> cvv;

    std::cout << "Input Expiry Date (MM/YY): ";
    std::cin >> expiryDate;
}

/// RSA finished.

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
    // CryptoPP::RSA::PublicKey publicKey;
    // CryptoPP::RSA::PrivateKey privateKey;
    // generateRSAKeys(publicKey, privateKey);

    std::string card_number, cvv, expiry_date;
    inputCardDetails(card_number, cvv, expiry_date);

    try {
        Card card(card_number, cvv, expiry_date);
        card.check();
    } catch (const std::exception& e) {
        std::cout << e.what() << std::endl;
        return 1;
    }

    UserData user("Bob", "Star", 120.0, "11 Silly Street, Switzerland", "CHF");
    user.DisplayUserInfo();

    selectPaymentMethod();

    // std::string encryptedCardNumber = encryptCardNumber(card_number, publicKey);
    // std::cout << "Encrypted Card Number: " << encryptedCardNumber << std::endl;

    // std::string decryptedCardNumber = decryptCardNumber(encryptedCardNumber, privateKey);
    // std::cout << "Decrypted Card Number: " << decryptedCardNumber << std::endl;

    return 0;
}