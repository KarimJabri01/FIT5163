#include <iostream>
#include <fstream>
#include <string>
#include <ctime>
#include <limits>
#include <cstring>
#include <sstream>
// #include <cryptopp/rsa.h>
// #include <cryptopp/osrng.h>
// #include <cryptopp/hex.h>
// #include <cryptopp/base64.h>
// #include <cryptopp/files.h>
// #include <cryptopp/filters.h>
#include <filesystem>


class UserData{ /// changes include replacing C type arras to more secure cpp ones
    public:
    
    UserData(const std::string& firstName, const std::string& lastName, double balance, const std::string& address, const std::string& currency)
             : fname(firstName), lname(lastName), balance(balance), address(address), location_currency(currency) {}
    void DisplayUserInfo() const{
        std::cout << fname << " " << lname << " " << " has " <<  balance << "  " << location_currency << " residing at " << address << std::endl;
    }

     void SaveToCSV(UserData user) const {
        std::string filename="user_data.csv";
        
        // Check if file exists; if not, add headers
        if (!std::filesystem::exists(filename)) {
            std::ofstream file(filename);
            file.open(filename, std::ios::out);
            file << "First Name,Last Name,Balance,Address,Currency\n"; // Add headers
            file.close();
        } else {
            std::fstream file(filename, std::ios::app); // Append if the file exists
            file << fname << "," << lname << "," << balance << "," << address << "," << location_currency << "\n";
            file.close();
        }
        
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

class Card { /// define better scope
private:
    const std::string card_number;
    const std::string cvv;
    const std::string exp_date;

public:
    // Constructor to initialize the card number
    Card(const std::string &in_card_number, const std::string &in_cvv, const std::string &in_exp_date) 
    : card_number(in_card_number), cvv(in_cvv), exp_date(in_exp_date)
    {
        if (cvv.length() != 3) {
            std::cout << "CVV code is INCORRECT!" << std::endl;
            return;
        }

        std::stringstream ss(exp_date);
        int exp_mon, exp_yr;
        char delimeter;
        ss >> exp_mon >> delimeter >> exp_yr;
        if (isCardExpired(exp_mon, exp_yr)) {
            std::cout << "Your card is EXPIRED!" << std::endl;
        }
    } //: card_number(card_number) {}
    int card_len = card_number.length();
    void check() {
        if (card_number.length() < 13) {
            std::cout << "Card number is too short" << std::endl;
            return;
        }
        /*
        // Extract the first 5 characters
        std::string first_five_digits = card_number.substr(0, 5);
        int carddet = std::stoi(first_five_digits);
        */

       // Luhn Algorithm to validate the card
       int luhn_v = 0;
       int count = 1;

       for (char digit_c : card_number) {
            int digit = digit_c - '0';
            if (count % 2 == 1) {
                int doubling =  digit * 2;
                luhn_v = (doubling % 10) + (doubling / 10);
            }
            else {
                luhn_v += digit;
            }
            count++;
       }
       
        if (luhn_v % 10 == 0) {
            std::cout << "Card Number is VALID" << std::endl;
        }
        else {
            std::cout << "Card Number is INVALID!" << std::endl;
            return;
        }

        std::cout << card_check(card_number) << std::endl;
    }

    // Check the type of the card (Visa, MasterCard/EuroCard)
    std::string card_check(const std::string& card_number) {
        // check for Visa card
        if (card_number[0] == '4' && 
            (card_len == 13 || card_len == 16 || card_len == 19)) {
            return "visa";
        }

        // check for MasterCard and EuroCard
        int first_two_digits = std::stoi(card_number.substr(0, 2));
        int first_four_digits = std::stoi(card_number.substr(0, 4));

        if (card_len == 16 && 
            (first_two_digits >= 51 && first_two_digits <= 55) || (first_four_digits >= 2221 && first_four_digits <= 2720)) {
                return "MasterCard/EuroCard";
            }

        return "Card is INVALID!";
    }

    // Function to get the current month and year
    void getCurrentMonthYear(int &currentMonth, int &currentYear) {
        time_t t = time(0);
        struct tm *now = localtime(&t);
        
        currentMonth = now->tm_mon + 1;  // tm_mon is zero-based (0-11), so add 1
        currentYear = (now->tm_year + 1900) % 100; // Get last two digits of year
    }

    // Function to check if the card is expired
    bool isCardExpired(int expMonth, int expYear) {
        int currentMonth, currentYear;
        getCurrentMonthYear(currentMonth, currentYear);

        // If the expiration year is less than the current year, the card is expired
        if (expYear < currentYear) {
            return true;  // Card is expired
        }

        // If the expiration year is the same, but the expiration month is less than the current month
        if (expYear == currentYear && expMonth < currentMonth) {
            return true;  // Card is expired
        }

        // If the expiration month is the current month or in the future, it's valid
        return false;  // Card is not expired
    }

    /*void card_check(int carddet) const {
        if (carddet > 5000) {
            std::cout << "Card is Mastercard" << std::endl;
        }
        else if (carddet >= 4000 && carddet < 5000) {
            std::cout << "Card is Visa" << std::endl;
        }
        else if (carddet >= 3000 && carddet < 4000) {
            std::cout << "Card is EuroMaster" << std::endl;
        }
        else {
            std::cout << "Card is invalid" << std::endl;
        }
    }*/

};

   

 
 // reminder for semi-colon

class terminal_PODL{
    // what does the terminal gotta do?
    //  amount, the country code, the currency, the date,the transaction type, and the terminal's random number UN 
    //  called Unpredictable Number in EMV's terminology
    int TUN [30];
    char currency [4];
    char date [8]; /// idk how to create a better time getter. for now only use time stamp. 
    char transaction_type [2];

};

class bank {
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
    
    
    public:
        bank(const std::string& )
};

/// maybe another class for the initial payment?


void generateRSAKeys(CryptoPP::RSA::PublicKey &publicKey, CryptoPP::RSA::PrivateKey &privateKey){

    CryptoPP::AutoSeededRandomPool rng;
    privateKey.GenerateRandomWithKeySize(rng, 2048);
    publicKey.AssignFrom(privateKey);
}

std::string encryptCardNumber(const std::string &cardNumber, CryptoPP::RSA::PublicKey &publicKey){
    CryptoPP::AutoSeededRandomPool rng;
    std::string cipher;

    CryptoPP::RSAES_OAEP_SHA_Encryptor encryptor(publicKey);
    CryptoPP::StringSource ss1(cardNumber, true, new CryptoPP::PK_EncryptorFilter(rng, encryptor, new CryptoPP::StringSink(cipher)));
    return cipher;
}

std::string decryptCardNumber(const std::string &cipher, CryptoPP::RSA::PrivateKey &privateKey){
    CryptoPP::AutoSeededRandomPool rng;
    std::string recovered;

    CryptoPP::RSAES_OAEP_SHA_Decryptor decryptor(privateKey);
    CryptoPP::StringSource ss4(cipher,true, new CryptoPP::PK_DecryptorFilter(rng, decryptor, new CryptoPP::StringSink(recovered)));
    return recovered;
}

int main () {
    CryptoPP::RSA::PublicKey publicKey;
    CryptoPP::RSA::PrivateKey privateKey;
    generateRSAKeys(publicKey, privateKey);
    ///// this gotta be not an input but maybe store in a class.
    std::string card_number;
    std::cout << "Input Card Number: ";
    std::cin >> card_number;
    ///
    std::string cvv;
    std::cout << "Input CVV: ";
    std::cin >> cvv;
    /// Create Data
    std::string expiry_date;
    std::cout << "Input your Expiry Date:  ";
    std::cin >> expiry_date;
    // Create a Card object and check the card
    Card card(card_number, cvv, expiry_date);
    card.check();


     // Intialised user data
    user_data user1; /// safety measure over char to prevet buffer overflows.
    strncpy(user1.fname, "Bob,", sizeof(user1.fname) - 1);
    user1.fname[sizeof(user1.fname) - 1] = '\0';
    strncpy(user1.lname, "Star", sizeof(user1.lname) - 1);
    user1.lname[sizeof(user1.lname) - 1] = '\0';
    user1.balance = 120;
    strncpy(user1.address, "11 silly street, Switzerland", sizeof(user1.address) - 1);
    user1.address[sizeof(user1.address) - 1] = '\0';
    strncpy(user1.location_currency, "Swiss francs", sizeof(user1.location_currency) -1);
    user1.location_currency[sizeof(user1.location_currency) - 1] = '\0';

    // print statement to make sure all is good.
    std::cout << user1.fname << user1.lname << " has " << user1.balance << " CHF " << " and lives in " << user1.address << " using " << user1.location_currency << std::endl;
    
    
    // intialise payment method request.
    
    int x;
    
    std::cout << "Hi, customer. What type of method would you like to use? Choose by pressing 1 [contact] or 0 [contactless]:   ";
    
    while (true) {
        std::cin >> x;

        // Check if the input is valid and either 1 or 2
        if (std::cin.fail()) {
            std::cin.clear(); // Clear the error flag
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Discard invalid input
            std::cout << "Invalid input. Please enter 1 or 2: ";
        } else if (x == 0 || x == 1) {
            break; // Valid input, exit the loop
        } else {
            std::cout << "Your selection is invalid, please select 1 or 0: ";
        }
    }

    std::cout << "Your selected payment method is: " << (x == 1 ? "Contact" : "Contactless") << std::endl;



    return 0; 
}