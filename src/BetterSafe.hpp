#include <Geode/utils/web.hpp>

struct SafeDate {
    int year;
    int month;
    int day;
};

struct SafeLevel {
    int id;
    int timelyID;
    std::vector<SafeDate> dates;
    bool weekly;
    int tier;
};

class BetterSafe {
public:
    inline static std::vector<SafeLevel> DAILY_SAFE;
    inline static std::vector<SafeLevel> WEEKLY_SAFE;

    static SafeDate parseDate(const std::string&);
    static void loadDailySafe(geode::EventListener<geode::utils::web::WebTask>&&, LoadingCircle*, std::function<void()> const&);
    static void loadWeeklySafe(geode::EventListener<geode::utils::web::WebTask>&&, LoadingCircle*, std::function<void()> const&);
    static std::vector<SafeLevel> getMonth(int year, int month, bool weekly);
    static int getDifficultyFromLevel(GJGameLevel* level);
};
