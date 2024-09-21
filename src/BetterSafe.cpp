#include "BetterSafe.hpp"

#define DAILY_SAFE_URL "https://raw.githubusercontent.com/hiimjustin000/the-safe/master/daily.json"
#define WEEKLY_SAFE_URL "https://raw.githubusercontent.com/hiimjustin000/the-safe/master/weekly.json"

SafeDate BetterSafe::parseDate(const std::string& date) {
    auto parts = string::split(date, "-");
    return {
        .year = parts.size() > 0 ? numFromString<int>(parts[0]).value_or(1970) : 1970,
        .month = parts.size() > 1 ? numFromString<int>(parts[1]).value_or(1) : 1,
        .day = parts.size() > 2 ? numFromString<int>(parts[2]).value_or(1) : 1
    };
}

#define PROPERTY_OR_DEFAULT(obj, prop, isFunc, asFunc, def) (obj.contains(prop) && obj[prop].isFunc() ? obj[prop].asFunc() : def)

void BetterSafe::loadDailySafe(EventListener<web::WebTask>&& listenerRef, LoadingCircle* circle, MiniFunction<void()> callback) {
    if (DAILY_SAFE.size()) return callback();

    auto&& listener = std::move(listenerRef);

    listener.bind([callback, circle](web::WebTask::Event* e) {
        if (auto res = e->getValue()) {
            if (res->ok()) {
                auto str = res->string().value();
                std::string error;
                auto json = matjson::parse(str, error).value_or(matjson::Array());
                for (auto& level : json.as_array()) {
                    std::vector<SafeDate> dates;
                    dates.push_back(parseDate(PROPERTY_OR_DEFAULT(level, "date", is_string, as_string, "1970-01-01")));
                    if (dates[0].year == 1970 && dates[0].month == 1 && dates[0].day == 1) dates.clear();
                    DAILY_SAFE.push_back({
                        .id = PROPERTY_OR_DEFAULT(level, "id", is_number, as_int, 0),
                        .timelyID = PROPERTY_OR_DEFAULT(level, "dailyID", is_number, as_int, 0),
                        .dates = dates,
                        .weekly = false,
                        .tier = PROPERTY_OR_DEFAULT(level, "tier", is_number, as_int, 0)
                    });
                }
                callback();
            }
            else {
                FLAlertLayer::create(fmt::format("Load Failed ({})", res->code()).c_str(), "Failed to load the daily safe. Please try again later.", "OK")->show();
                circle->setVisible(false);
            }
        }
    });

    listener.setFilter(web::WebRequest().get(DAILY_SAFE_URL));
}

void BetterSafe::loadWeeklySafe(EventListener<web::WebTask>&& listenerRef, LoadingCircle* circle, MiniFunction<void()> callback) {
    if (WEEKLY_SAFE.size()) return callback();

    auto&& listener = std::move(listenerRef);

    listener.bind([callback, circle](web::WebTask::Event* e) {
        if (auto res = e->getValue()) {
            if (res->ok()) {
                auto str = res->string().value();
                std::string error;
                auto json = matjson::parse(str, error).value_or(matjson::Array());
                for (auto& level : json.as_array()) {
                    std::vector<SafeDate> dates;
                    if (level.contains("dates") && level["dates"].is_array()) {
                        for (auto& date : level["dates"].as_array()) {
                            dates.push_back(parseDate(date.as_string()));
                        }
                    }
                    WEEKLY_SAFE.push_back({
                        .id = PROPERTY_OR_DEFAULT(level, "id", is_number, as_int, 0),
                        .timelyID = PROPERTY_OR_DEFAULT(level, "weeklyID", is_number, as_int, 0),
                        .dates = dates,
                        .weekly = true,
                        .tier = PROPERTY_OR_DEFAULT(level, "tier", is_number, as_int, 0)
                    });
                }
                callback();
            }
            else {
                FLAlertLayer::create(fmt::format("Load Failed ({})", res->code()).c_str(), "Failed to load the weekly safe. Please try again later.", "OK")->show();
                circle->setVisible(false);
            }
        }
    });

    listener.setFilter(web::WebRequest().get(WEEKLY_SAFE_URL));
}

std::vector<SafeLevel> BetterSafe::getMonth(int year, int month, bool weekly) {
    std::vector<SafeLevel> levels;
    auto& safe = weekly ? WEEKLY_SAFE : DAILY_SAFE;
    for (auto& level : safe) {
        for (auto& date : level.dates) {
            if (date.year == year && date.month == month) levels.push_back(level);
        }
    }
    return levels;
}

int BetterSafe::getDifficultyFromLevel(GJGameLevel* level) {
    if (level->m_demon > 0) return level->m_demonDifficulty > 0 ? level->m_demonDifficulty + 4 : 6;
    else if (level->m_autoLevel) return -1;
    else if (level->m_ratings < 5) return 0;
    else return level->m_ratingsSum / level->m_ratings;
}
