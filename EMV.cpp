#include <iostream>
#include <string>
#include <ctime>

class user_data{
    public:
    char user_name[40];  /// trivial number
    float balance;
    char user_address[100];
    char location_currency[50];

}; // reminder for semi-colon

class terminal_PODL{
    // what does the terminal gotta do?
    //  amount, the country code, the currency, the date,the transaction type, and the terminal’s random number UN 
    //  called Unpredictable Number in EMV’s terminology
    int tun [30];
    char currency [4];
    char date [8]; /// idk how to create a better time getter. for now only use time stamp. 
    char transaction_type [2];

}

/// maybe another class for the initial payment?
int main () {
     // Intialised user data
    user_data user1;
    strncpy(user1.user_name, "Bob,", sizeof(user1.user_name) - 1);
    user1.user_name[sizeof(user1.user_name) - 1] = '\0';
    user1.balance = 120;
    strncpy(user1.user_address, "11 silly street,", sizeof(user1.user_address) - 1);
    user1.user_address[sizeof(user1.user_name) - 1] = '\0';
    strncpy(user1.location_currency, "Switzerland,", sizeof(user1.location_currency) -1);
    user1.location_currency[sizeof(user1.user_name) - 1] = '\0';

    // print statement to make sure all is good.
    std::cout << user1.user_name << " has " << user1.balance << " Swiz Francs" << " lives in " << user1.user_address << " from " << user1.location_currency << std::endl;
    
    
    // intialise payment method request.
    
    int x;
    
    std::cout << "Hi, customer. What type of method would you like to use? Choose by pressing 1 [contact] or 2 [contactless]:   ";
    
    while (true) {
        std::cin >> x;

        // Check if the input is valid and either 1 or 2
        if (std::cin.fail()) {
            std::cin.clear(); // Clear the error flag
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Discard invalid input
            std::cout << "Invalid input. Please enter 1 or 2: ";
        } else if (x == 1 || x == 2) {
            break; // Valid input, exit the loop
        } else {
            std::cout << "Your selection is invalid, please select 1 or 2: ";
        }
    }

    std::cout << "Your selected payment method is: " << (x == 1 ? "Contact" : "Contactless") << std::endl;



    return 0; 
}

