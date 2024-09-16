#ifndef CHECK_CARD
#define CHECK_CARD

class CHECK_CARD {
public:
    void getCurrentMonthYear(int &currentMonth, int &currentYear);
    bool isCardExpired(int expMonth, int expYear);
};

#endif