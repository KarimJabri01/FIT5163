#ifndef CHECK_CARD
#define CHECK_CARD

class CheckCard {
public:
    void getCurrentMonthYear(int &currentMonth, int &currentYear);
    bool isCardExpired(int expMonth, int expYear);
};

#endif