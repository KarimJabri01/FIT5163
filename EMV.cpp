#include <iostream>
#include <string>
#include <ctime>
#include <limits>
#include <cstring>

class user_data{ /// AID, could rename later  
    public:
    char fname[20];
    char lname[20]; /// trivial number
    float balance;
    char address[100];
    char location_currency[50];
};
    
///maybe do getter class
    // card dummy

class Card { /// define better scope
public:
    // Constructor to initialize the card number
    Card(const std::string& card_number) : card_number(card_number) {}

    void check() {
        if (card_number.length() < 15) {
            std::cout << "Card number is too short" << std::endl;
            return;
        }

        // Extract the first 5 characters
        std::string first_five_digits = card_number.substr(0, 5);
        int carddet = std::stoi(first_five_digits);

        card_check(carddet); // Call card_check with the extracted digits
    }

private:
    
    std::string card_number;

    void card_check(int carddet) const {
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
    }
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
    //meow
};

/// maybe another class for the initial payment?
int main () {
    ///// this gotta be not an input but maybe store in a class.
    std::string card_number;
    std::cout << "Input Card Number: ";
    std::cin >> card_number;
    ///
    std::string ccv;
    std::cout << "Input CCV: ";
    std::cin >> ccv;
    /// Create Data
    std::string expiry_date;
    std::cout << "Input your Expiry Date:  ";
    std::cin >> expiry_date;
    // Create a Card object and check the card
    Card card(card_number);
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