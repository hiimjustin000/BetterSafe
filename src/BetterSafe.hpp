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
    GJTimedLevelType type;
    int tier;
};

class BetterSafe {
public:
    inline static std::vector<SafeLevel> DAILY_SAFE;
    inline static std::vector<SafeLevel> WEEKLY_SAFE;
    inline static std::vector<SafeLevel> EVENT_SAFE;

    static SafeDate parseDate(const std::string&);
    static SafeDate dateFromTime(time_t);
    static void loadSafe(GJTimedLevelType, geode::EventListener<geode::utils::web::WebTask>&&, LoadingCircle*, std::function<void()> const&);
    static std::vector<SafeLevel> getMonth(int year, int month, GJTimedLevelType type);
    static std::vector<SafeLevel>& getSafeLevels(GJTimedLevelType type);
    static int getDifficultyFromLevel(GJGameLevel* level);
};
