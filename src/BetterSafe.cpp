#include "BetterSafe.hpp"

using namespace geode::prelude;

#define DAILY_SAFE_URL "https://raw.githubusercontent.com/hiimjustin000/the-safe/master/v2/daily.json"
#define WEEKLY_SAFE_URL "https://raw.githubusercontent.com/hiimjustin000/the-safe/master/v2/weekly.json"
#define EVENT_SAFE_URL "https://raw.githubusercontent.com/hiimjustin000/the-safe/master/v2/event.json"

SafeDate BetterSafe::parseDate(const std::string& date) {
    auto parts = string::split(date, "-");
    return {
        .year = parts.size() > 0 ? numFromString<int>(parts[0]).unwrapOr(1970) : 1970,
        .month = parts.size() > 1 ? numFromString<int>(parts[1]).unwrapOr(1) : 1,
        .day = parts.size() > 2 ? numFromString<int>(parts[2]).unwrapOr(1) : 1
    };
}

SafeDate BetterSafe::dateFromTime(time_t time) {
    #ifdef GEODE_IS_WINDOWS
    struct tm timeinfo;
    localtime_s(&timeinfo, &time);
    #else
    auto timeinfo = *std::localtime(&time);
    #endif
    return { timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday };
}

void BetterSafe::loadSafe(GJTimedLevelType type, EventListener<web::WebTask>&& listenerRef, LoadingCircle* circle, std::function<void()> const& callback) {
    if (!getSafeLevels(type).empty()) return callback();

    auto&& listener = std::move(listenerRef);

    listener.bind([callback, circle, type](web::WebTask::Event* e) {
        if (auto res = e->getValue()) {
            if (!res->ok()) {
                FLAlertLayer::create(fmt::format("Load Failed ({})", res->code()).c_str(), "Failed to load the safe. Please try again later.", "OK")->show();
                circle->setVisible(false);
                return;
            }

            auto json = res->json().unwrapOr(std::vector<matjson::Value>());
            if (!json.isArray()) {
                FLAlertLayer::create("Parse Failed", "Failed to parse the safe. Please try again later.", "OK")->show();
                circle->setVisible(false);
                return;
            }

            for (auto& level : json.asArray().unwrap()) {
                std::vector<SafeDate> dates;
                if (level.contains("dates") && level["dates"].isArray()) {
                    for (auto& date : level["dates"].asArray().unwrap()) {
                        dates.push_back(parseDate(date.asString().unwrapOr("1970-01-01")));
                    }
                }
                getSafeLevels(type).push_back({
                    .id = (int)level["id"].asInt().unwrapOr(0),
                    .timelyID = (int)level["timelyID"].asInt().unwrapOr(0),
                    .dates = dates,
                    .type = type,
                    .tier = (int)level["tier"].asInt().unwrapOr(0)
                });
            }

            if (type != GJTimedLevelType::Event || EVENT_SAFE.size() < 2) {
                callback();
                return;
            }

            auto lastEventDate = EVENT_SAFE[1].dates.back();
            tm timeinfo = { 0, 0, 0, lastEventDate.day, lastEventDate.month - 1, lastEventDate.year - 1900 };
            auto currentDate = dateFromTime(time(0));
            tm currentTimeinfo = { 0, 0, 0, currentDate.day, currentDate.month - 1, currentDate.year - 1900 };
            auto truncatedTime = mktime(&currentTimeinfo);
            for (auto lastEventTime = mktime(&timeinfo) + 86400; lastEventTime <= truncatedTime; lastEventTime += 86400) {
                EVENT_SAFE[0].dates.push_back(dateFromTime(lastEventTime));
            }

            callback();
        }
    });

    switch (type) {
        case GJTimedLevelType::Daily: listener.setFilter(web::WebRequest().get(DAILY_SAFE_URL)); break;
        case GJTimedLevelType::Weekly: listener.setFilter(web::WebRequest().get(WEEKLY_SAFE_URL)); break;
        case GJTimedLevelType::Event: listener.setFilter(web::WebRequest().get(EVENT_SAFE_URL)); break;
        default: break;
    }
}

std::vector<SafeLevel> BetterSafe::getMonth(int year, int month, GJTimedLevelType type) {
    std::vector<SafeLevel> levels;
    auto& safe = getSafeLevels(type);
    for (auto& level : safe) {
        for (auto& date : level.dates) {
            if (date.year == year && date.month == month) levels.push_back(level);
        }
    }
    return levels;
}

std::vector<SafeLevel>& BetterSafe::getSafeLevels(GJTimedLevelType type) {
    switch (type) {
        case GJTimedLevelType::Daily: return DAILY_SAFE;
        case GJTimedLevelType::Weekly: return WEEKLY_SAFE;
        case GJTimedLevelType::Event: return EVENT_SAFE;
        default: {
            static std::vector<SafeLevel> empty;
            return empty;
        }
    }
}

int BetterSafe::getDifficultyFromLevel(GJGameLevel* level) {
    if (level->m_demon > 0) return level->m_demonDifficulty > 0 ? level->m_demonDifficulty + 4 : 6;
    else if (level->m_autoLevel) return -1;
    else if (level->m_ratings < 5) return 0;
    else return level->m_ratingsSum / level->m_ratings;
}
