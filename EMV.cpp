#include <iostream>
#include <string>
#include <ctime>

class user_data{ /// AID, could rename later  
    public:
    char fname[20];
    char lname[20]; /// trivial number
    float balance;
    char address[100];
    char location_currency[50];
    

    // card dummy
    void card(){
        int cardno [4] = {5454, 6465, 8665, 9786}; 
        int ccv[3] = {7, 6, 1};
        int exp_date[5] = {1, 2, 0, 5};

    };

}; // reminder for semi-colon

class terminal_PODL{
    // what does the terminal gotta do?
    //  amount, the country code, the currency, the date,the transaction type, and the terminal’s random number UN 
    //  called Unpredictable Number in EMV’s terminology
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
};



