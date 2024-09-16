#include <iostream>
#include <ctime> 
#include <sstream> 

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

int main() {
    // Example expiration date (MM/YY)
    int expMonth, expYear;
    std::string expDate;

    std::cout << "Enter the card's expiration date (MM/YY): ";
    std::cin >> expDate;

    // Parsing the expiration date from the input string
    std::stringstream ss(expDate);
    char delimiter;
    ss >> expMonth >> delimiter >> expYear;

    // Check if the card is expired
    bool expired = isCardExpired(expMonth, expYear);
    std::cout << std::boolalpha << expired << std::endl;

    return 0;
}
