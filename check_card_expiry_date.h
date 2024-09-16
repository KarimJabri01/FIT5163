#ifndef CHECK_CARD
#define CHECK_CARD

class CheckCard {
public:
    static void getCurrentMonthYear(int &currentMonth, int &currentYear);
    static bool isCardExpired(int expMonth, int expYear);
};

#endif