#include <Geode/Geode.hpp>
#include <Geode/utils/web.hpp>

using namespace geode::prelude;

struct SafeDate {
    int year;
    int month;
    int day;
};

struct SafeLevel {
    int id;
    int timelyID;
    std::vector<SafeDate> dates;
    std::string name;
    std::string creator;
    int stars;
    int difficulty;
    int feature;
    int coins;
    bool coinsVerified;
    bool weekly;
};

class BetterSafe {
public:
    inline static std::vector<SafeLevel> DAILY_SAFE;
    inline static std::vector<SafeLevel> WEEKLY_SAFE;

    static SafeDate parseDate(const std::string&);
    static void loadDailySafe(EventListener<web::WebTask>&&, LoadingCircle*, MiniFunction<void()>);
    static void loadWeeklySafe(EventListener<web::WebTask>&&, LoadingCircle*, MiniFunction<void()>);
    static std::vector<SafeLevel> getMonth(int year, int month, bool weekly);
};
