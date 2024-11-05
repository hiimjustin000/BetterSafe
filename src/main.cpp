#include "BSCalendarPopup.hpp"

using namespace geode::prelude;

#include <Geode/modify/MenuLayer.hpp>
class $modify(BSMenuLayer, MenuLayer) {
    struct Fields {
        CCObject* m_dailySafeListener;
        SEL_MenuHandler m_dailySafeSelector;
        CCObject* m_weeklySafeListener;
        SEL_MenuHandler m_weeklySafeSelector;
    };

    static void onModify(auto& self) {
        (void)self.setHookPriority("MenuLayer::init", INT_MIN / 2);
    }

    bool init() {
        if (!MenuLayer::init()) return false;

        auto redashMenu = getChildByID("ninxout.redash/redash-menu");
        if (!redashMenu) return true;

        auto dailiesMenu = redashMenu->getChildByID("ninxout.redash/dailies-menu");
        if (!dailiesMenu) return true;
        
        auto f = m_fields.self();

        auto dailyNode = dailiesMenu->getChildByID("daily-node");
        auto dailySafeButton = static_cast<CCMenuItemSpriteExtra*>(dailyNode ? dailyNode->getChildByIDRecursive("safe-button") : nullptr);
        if (dailyNode && dailySafeButton) {
            f->m_dailySafeListener = dailySafeButton->m_pListener;
            f->m_dailySafeSelector = dailySafeButton->m_pfnSelector;
            dailySafeButton->setTarget(this, menu_selector(BSMenuLayer::onTheDailySafe));
        }

        auto weeklyNode = dailiesMenu->getChildByID("weekly-node");
        auto weeklySafeButton = static_cast<CCMenuItemSpriteExtra*>(weeklyNode ? weeklyNode->getChildByIDRecursive("safe-button") : nullptr);
        if (weeklyNode && weeklySafeButton) {
            f->m_weeklySafeListener = weeklyNode;
            f->m_weeklySafeSelector = weeklySafeButton->m_pfnSelector;
            weeklySafeButton->setTarget(this, menu_selector(BSMenuLayer::onTheWeeklySafe));
        }

        return true;
    }

    void onTheDailySafe(CCObject*) {
        auto f = m_fields.self();
        BSCalendarPopup::create(f->m_dailySafeListener, f->m_dailySafeSelector, false)->show();
    }

    void onTheWeeklySafe(CCObject*) {
        auto f = m_fields.self();
        BSCalendarPopup::create(f->m_weeklySafeListener, f->m_weeklySafeSelector, true)->show();
    }
};

#include <Geode/modify/DailyLevelPage.hpp>
class $modify(BSDailyLevelPage, DailyLevelPage) {
    void onTheSafe(CCObject* sender) {
        if (sender->getTag() == 91508) DailyLevelPage::onTheSafe(sender);
        else BSCalendarPopup::create(this, menu_selector(DailyLevelPage::onTheSafe), m_type == GJTimedLevelType::Weekly)->show();
    }
};
