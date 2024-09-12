#include "BSCalendarPopup.hpp"

#include <Geode/modify/DailyLevelPage.hpp>
class $modify(BSDailyLevelPage, DailyLevelPage) {
    void onTheSafe(CCObject* sender) {
        if (sender->getTag() == 91508) DailyLevelPage::onTheSafe(sender);
        else BSCalendarPopup::create(this, m_type == GJTimedLevelType::Weekly)->show();
    }
};
