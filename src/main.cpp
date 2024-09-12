#include "BSCalendarPopup.hpp"

#include <Geode/modify/MenuLayer.hpp>
class $modify(BSMenuLayer, MenuLayer) {
    static void onModify(auto& self) {
        (void)self.setHookPriority("MenuLayer::init", INT_MIN / 2);
    }

    bool init() {
        if (!MenuLayer::init()) return false;

        auto redashMenu = getChildByID("ninxout.redash/redash-menu");
        if (!redashMenu) return true;

        auto dailiesMenu = redashMenu->getChildByID("ninxout.redash/dailies-menu");
        if (!dailiesMenu) return true;

        auto dailyNode = dailiesMenu->getChildByID("daily-node");
        auto dailySafeButton = static_cast<CCMenuItemSpriteExtra*>(dailyNode ? dailyNode->getChildByIDRecursive("safe-button") : nullptr);
        if (dailyNode && dailySafeButton) dailySafeButton->setTarget(this, menu_selector(BSMenuLayer::onTheDailySafe));

        auto weeklyNode = dailiesMenu->getChildByID("weekly-node");
        auto weeklySafeButton = static_cast<CCMenuItemSpriteExtra*>(weeklyNode ? weeklyNode->getChildByIDRecursive("safe-button") : nullptr);
        if (weeklyNode && weeklySafeButton) weeklySafeButton->setTarget(this, menu_selector(BSMenuLayer::onTheWeeklySafe));

        return true;
    }

    void onTheDailySafe(CCObject*) {
        BSCalendarPopup::create(this, menu_selector(BSMenuLayer::onTheOriginalDailySafe), false)->show();
    }

    void onTheWeeklySafe(CCObject*) {
        BSCalendarPopup::create(this, menu_selector(BSMenuLayer::onTheOriginalWeeklySafe), true)->show();
    }

    void onTheOriginalDailySafe(CCObject*) {
        switchToScene(LevelBrowserLayer::create(GJSearchObject::create(SearchType::DailySafe)));
    }

    void onTheOriginalWeeklySafe(CCObject*) {
        switchToScene(LevelBrowserLayer::create(GJSearchObject::create(SearchType::WeeklySafe)));
    }
};

#include <Geode/modify/DailyLevelPage.hpp>
class $modify(BSDailyLevelPage, DailyLevelPage) {
    void onTheSafe(CCObject* sender) {
        if (sender->getTag() == 91508) DailyLevelPage::onTheSafe(sender);
        else BSCalendarPopup::create(this, menu_selector(DailyLevelPage::onTheSafe), m_type == GJTimedLevelType::Weekly)->show();
    }
};
