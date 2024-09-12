#include "BSCalendarPopup.hpp"

using namespace geode::prelude;

BSCalendarPopup* BSCalendarPopup::create(DailyLevelPage* page, bool weekly) {
    auto ret = new BSCalendarPopup();
    if (ret->initAnchored(300.0f, 280.0f, page, weekly)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool BSCalendarPopup::setup(DailyLevelPage* page, bool weekly) {
    m_noElasticity = true;
    m_weekly = weekly;

    auto sundayFirst = Mod::get()->getSettingValue<bool>("sunday-first");
    createWeekdayLabel("Mon", (int)sundayFirst);
    createWeekdayLabel("Tue", (int)sundayFirst + 1);
    createWeekdayLabel("Wed", (int)sundayFirst + 2);
    createWeekdayLabel("Thu", (int)sundayFirst + 3);
    createWeekdayLabel("Fri", (int)sundayFirst + 4);
    createWeekdayLabel("Sat", (int)sundayFirst + 5);
    createWeekdayLabel("Sun", sundayFirst ? 0 : 6);

    auto glm = GameLevelManager::sharedState();
    glm->m_levelManagerDelegate = this;

    m_prevButton = CCMenuItemExt::createSpriteExtraWithFrameName("GJ_arrow_01_001.png", 1.0f, [this](auto) {
        if (m_year == m_firstYear && m_month == m_firstMonth) return;

        if (m_month == 1) {
            m_year--;
            m_month = 12;
        }
        else m_month--;
        loadMonth();
    });
    m_prevButton->setPosition(-34.5f, 140.0f);
    m_prevButton->setVisible(false);
    m_buttonMenu->addChild(m_prevButton);

    auto nextButtonSprite = CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
    nextButtonSprite->setFlipX(true);
    m_nextButton = CCMenuItemExt::createSpriteExtra(nextButtonSprite, [this](auto) {
        if (m_year == m_currentYear && m_month == m_currentMonth) return;
        if (m_month == 12) {
            m_year++;
            m_month = 1;
        }
        else m_month++;
        loadMonth();
    });
    m_nextButton->setPosition(334.5f, 140.0f);
    m_nextButton->setVisible(false);
    m_buttonMenu->addChild(m_nextButton);

    auto firstArrow = CCSprite::createWithSpriteFrameName("GJ_arrow_02_001.png");
    auto otherFirstArrow = CCSprite::createWithSpriteFrameName("GJ_arrow_02_001.png");
    otherFirstArrow->setPosition(firstArrow->getContentSize() / 2 - CCPoint { 20.0f, 0.0f });
    firstArrow->addChild(otherFirstArrow);
    firstArrow->setScale(0.4f);
    m_firstButton = CCMenuItemExt::createSpriteExtra(firstArrow, [this](auto) {
        m_year = m_firstYear;
        m_month = m_firstMonth;
        loadMonth();
    });
    m_firstButton->setPosition(-27.5f, 175.0f);
    m_firstButton->setVisible(false);
    m_buttonMenu->addChild(m_firstButton);

    auto lastArrow = CCSprite::createWithSpriteFrameName("GJ_arrow_02_001.png");
    lastArrow->setFlipX(true);
    auto otherLastArrow = CCSprite::createWithSpriteFrameName("GJ_arrow_02_001.png");
    otherLastArrow->setPosition(lastArrow->getContentSize() / 2 + CCPoint { 20.0f, 0.0f });
    otherLastArrow->setFlipX(true);
    lastArrow->addChild(otherLastArrow);
    lastArrow->setScale(0.4f);
    m_lastButton = CCMenuItemExt::createSpriteExtra(lastArrow, [this](auto) {
        m_year = m_currentYear;
        m_month = m_currentMonth;
        loadMonth();
    });
    m_lastButton->setPosition(327.5f, 175.0f);
    m_lastButton->setVisible(false);
    m_buttonMenu->addChild(m_lastButton);

    m_calendarMenu = CCMenu::create();
    m_calendarMenu->setPosition(150.0f, 140.0f);
    m_calendarMenu->setContentSize({ 300.0f, 280.0f });
    m_calendarMenu->ignoreAnchorPointForPosition(false);
    m_mainLayer->addChild(m_calendarMenu);

    m_monthLabel = CCLabelBMFont::create("", "goldFont.fnt");
    m_monthLabel->setScale(0.6f);
    m_monthButton = CCMenuItemExt::createSpriteExtra(m_monthLabel, [this](auto) {
        BSSelectPopup::create(m_year, m_month, m_firstYear, m_firstMonth, m_currentYear, m_currentMonth, [this](int year, int month) {
            m_year = year;
            m_month = month;
            loadMonth();
        })->show();
    });
    m_monthButton->setPosition(150.0f, 265.0f);
    m_monthButton->setEnabled(false);
    m_buttonMenu->addChild(m_monthButton);

    m_loadingCircle = LoadingCircle::create();
    m_loadingCircle->setParentLayer(this);
    m_loadingCircle->retain();
    m_loadingCircle->show();

    auto safeButton = CCMenuItemExt::createSpriteExtraWithFrameName("GJ_safeBtn_001.png", 1.0f, [this, page](auto sender) {
        page->onTheSafe(sender);
    });
    safeButton->setPosition(340.0f, 25.0f);
    safeButton->setTag(91508); // my birthday
    m_buttonMenu->addChild(safeButton);

    auto refreshButton = CCMenuItemExt::createSpriteExtraWithFrameName("GJ_updateBtn_001.png", 1.0f, [this, weekly](auto) {
        m_loadingCircle->setVisible(true);
        if (m_hoverNode) m_hoverNode->close();
        m_selected = nullptr;
        m_calendarMenu->removeAllChildren();
        m_prevButton->setVisible(false);
        m_nextButton->setVisible(false);
        m_firstButton->setVisible(false);
        m_lastButton->setVisible(false);
        m_monthButton->setEnabled(false);
        if (weekly) {
            BetterSafe::WEEKLY_SAFE.clear();
            BetterSafe::loadWeeklySafe(std::move(m_listener), m_loadingCircle, [this] {
                if (!BetterSafe::WEEKLY_SAFE.empty()) loadMonth();
            });
        }
        else {
            BetterSafe::DAILY_SAFE.clear();
            BetterSafe::loadDailySafe(std::move(m_listener), m_loadingCircle, [this] {
                if (!BetterSafe::DAILY_SAFE.empty()) loadMonth();
            });
        }
    });
    refreshButton->setPosition(340.0f, 80.0f);
    m_buttonMenu->addChild(refreshButton);

    time_t now = time(0);
    #ifdef GEODE_IS_WINDOWS
    struct tm timeinfo;
    localtime_s(&timeinfo, &now);
    m_year = timeinfo.tm_year + 1900;
    m_month = timeinfo.tm_mon + 1;
    #else
    auto timeinfo = std::localtime(&now);
    m_year = timeinfo->tm_year + 1900;
    m_month = timeinfo->tm_mon + 1;
    #endif
    if (weekly) {
        BetterSafe::loadWeeklySafe(std::move(m_listener), m_loadingCircle, [this] {
            if (BetterSafe::WEEKLY_SAFE.empty()) return;

            auto firstLevel = BetterSafe::WEEKLY_SAFE[0];
            auto lastLevel = BetterSafe::WEEKLY_SAFE[BetterSafe::WEEKLY_SAFE.size() - 1];
            auto firstDate = std::find_if(firstLevel.dates.begin(), firstLevel.dates.end(), [this](const SafeDate& date) {
                return date.year == m_year && date.month == m_month;
            });
            if (firstDate == firstLevel.dates.end()) {
                auto lastDate = !firstLevel.dates.empty() ? firstLevel.dates[firstLevel.dates.size() - 1] : SafeDate { m_year, m_month, 1 };
                m_currentYear = lastDate.year;
                m_currentMonth = lastDate.month;
            } else {
                m_currentYear = firstDate->year;
                m_currentMonth = firstDate->month;
            }
            auto startDate = !lastLevel.dates.empty() ? lastLevel.dates[0] : SafeDate { m_year, m_month, 1 };
            m_firstYear = startDate.year;
            m_firstMonth = startDate.month;
            loadMonth();
        });
    }
    else {
        BetterSafe::loadDailySafe(std::move(m_listener), m_loadingCircle, [this] {
            if (BetterSafe::DAILY_SAFE.empty()) return;

            auto firstLevel = BetterSafe::DAILY_SAFE[0];
            auto lastLevel = BetterSafe::DAILY_SAFE[BetterSafe::DAILY_SAFE.size() - 1];
            auto firstDate = !firstLevel.dates.empty() ? firstLevel.dates[0] : SafeDate { m_year, m_month, 1 };
            auto lastDate = !lastLevel.dates.empty() ? lastLevel.dates[0] : SafeDate { m_year, m_month, 1 };
            m_currentYear = firstDate.year;
            m_firstYear = lastDate.year;
            m_currentMonth = firstDate.month;
            m_firstMonth = lastDate.month;
            loadMonth();
        });
    }

    handleTouchPriority(this);

    return true;
}

void BSCalendarPopup::createWeekdayLabel(const char* text, int idx) {
    auto label = CCLabelBMFont::create(text, "bigFont.fnt");
    label->setPosition(idx * 38.0f + 36.0f, 249.0f);
    label->setScale(0.5f);
    m_mainLayer->addChild(label);
}

void BSCalendarPopup::loadMonth() {
    m_loadingCircle->setVisible(true);

    if (m_hoverNode) m_hoverNode->close();

    m_calendarMenu->removeAllChildren();
    m_selected = nullptr;
    m_monthLabel->setString(fmt::format("{} {}", MONTHS[m_month - 1], m_year).c_str());
    m_monthButton->updateSprite();
    m_monthButton->setEnabled(false);

    m_prevButton->setVisible(false);
    m_nextButton->setVisible(false);
    m_firstButton->setVisible(false);
    m_lastButton->setVisible(false);

    auto levelSafe = BetterSafe::getMonth(m_year, m_month, m_weekly);
    std::vector<std::string> ids;
    for (auto& level : levelSafe) ids.push_back(std::to_string(level.id));
    auto searchObject = GJSearchObject::create(SearchType::MapPackOnClick, string::join(ids, ","));
    auto glm = GameLevelManager::sharedState();
    if (auto storedLevels = glm->getStoredOnlineLevels(searchObject->getKey())) loadLevelsFinished(storedLevels, searchObject->getKey());
    else glm->getOnlineLevels(searchObject);
}

void BSCalendarPopup::loadLevelsFinished(CCArray* levels, const char*) {
    CC_SAFE_RELEASE(m_levels);
    m_levels = levels;
    m_levels->retain();
    setupMonth();
}

void BSCalendarPopup::setupMonth() {
    m_monthButton->setEnabled(true);
    m_loadingCircle->setVisible(false);

    m_prevButton->setVisible(m_year != m_firstYear || m_month != m_firstMonth);
    m_firstButton->setVisible(m_year != m_firstYear || m_month != m_firstMonth);
    m_nextButton->setVisible(m_year != m_currentYear || m_month != m_currentMonth);
    m_lastButton->setVisible(m_year != m_currentYear || m_month != m_currentMonth);

    auto daysInMonth = m_month == 2 && m_year % 4 == 0 && (m_year % 100 != 0 || m_year % 400 == 0) ? 29 : DAYS_IN_MONTH[m_month - 1];
    #ifdef GEODE_IS_WINDOWS
    struct tm timeinfo = { 0, 0, 0, 1, m_month - 1, m_year - 1900 };
    auto time = mktime(&timeinfo);
    localtime_s(&timeinfo, &time);
    auto firstWeekday = timeinfo.tm_wday;
    #else
    tm timeinfo = { 0, 0, 0, 1, m_month - 1, m_year - 1900 };
    auto time = mktime(&timeinfo);
    auto firstWeekday = std::localtime(&time)->tm_wday;
    #endif
    if (!Mod::get()->getSettingValue<bool>("sunday-first")) firstWeekday = (firstWeekday + 6) % 7;

    auto levelSafe = BetterSafe::getMonth(m_year, m_month, m_weekly);
    std::vector<GJGameLevel*> levels;
    for (auto level : CCArrayExt<GJGameLevel*>(m_levels)) levels.push_back(level);
    for (int i = 0; i < daysInMonth; i++) {
        auto safeLevelIt = std::find_if(levelSafe.begin(), levelSafe.end(), [this, i](auto const& level) {
            return std::any_of(level.dates.begin(), level.dates.end(), [this, i](auto const& date) {
                return date.year == m_year && date.month == m_month && date.day == i + 1;
            });
        });
        if (safeLevelIt == levelSafe.end()) continue;
        auto safeLevel = *safeLevelIt;
        auto gameLevelIt = std::find_if(levels.begin(), levels.end(), [safeLevel](auto level) {
            return level->m_levelID == safeLevel.id;
        });
        if (gameLevelIt == levels.end()) continue;
        auto gameLevel = *gameLevelIt;

        auto diffIcon = CCSprite::createWithSpriteFrameName(safeLevel.difficulty == -1 ?
            "diffIcon_auto_btn_001.png" : fmt::format("diffIcon_{:02d}_btn_001.png", safeLevel.difficulty).c_str());
        diffIcon->setScale(0.75f);
        auto featureFrame = "";
        switch (safeLevel.feature) {
            case 1: featureFrame = "GJ_featuredCoin_001.png"; break;
            case 2: featureFrame = "GJ_epicCoin_001.png"; break;
            case 3: featureFrame = "GJ_epicCoin2_001.png"; break;
            case 4: featureFrame = "GJ_epicCoin3_001.png"; break;
        }
        if (safeLevel.feature > 0) {
            auto featureIcon = CCSprite::createWithSpriteFrameName(featureFrame);
            featureIcon->setPosition(diffIcon->getContentSize() / 2 + CCPoint { 0.0f, -5.5f });
            diffIcon->addChild(featureIcon, -2);
        }
        auto hoverButton = CCMenuItemExt::createSpriteExtra(diffIcon, [this, gameLevel, safeLevel](auto sender) {
            if (m_hoverNode) m_hoverNode->close();
            if (sender != m_selected) {
                m_hoverNode = BSHoverNode::create(safeLevel, gameLevel, [this] {
                    m_hoverNode = nullptr;
                });
                m_hoverNode->setPosition(sender->getPosition() + CCPoint { 0.0f, sender->getContentHeight() / 2 + m_hoverNode->getContentHeight() / 2 + 5.0f });
                m_mainLayer->addChild(m_hoverNode, 200);
                m_selected = sender;
            }
            else m_selected = nullptr;
        });
        hoverButton->setPosition({ (i + firstWeekday) % 7 * 38.0f + 36.0f, 219.0f - floorf(((float)i + firstWeekday) / 7) * 38.0f });
        m_calendarMenu->addChild(hoverButton);
    }
}

BSCalendarPopup::~BSCalendarPopup() {
    CC_SAFE_RELEASE(m_levels);
    CC_SAFE_RELEASE(m_loadingCircle);
    auto glm = GameLevelManager::sharedState();
    if (glm->m_levelManagerDelegate == this) glm->m_levelManagerDelegate = nullptr;
}

BSSelectPopup* BSSelectPopup::create(int year, int month, int minYear, int minMonth, int maxYear, int maxMonth, MiniFunction<void(int, int)> callback) {
    auto ret = new BSSelectPopup();
    if (ret->initAnchored(250.0f, 150.0f, year, month, minYear, minMonth, maxYear, maxMonth, callback)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool BSSelectPopup::setup(int year, int month, int minYear, int minMonth, int maxYear, int maxMonth, MiniFunction<void(int, int)> callback) {
    setTitle("Select Date");

    m_noElasticity = true;
    m_year = year;
    m_month = month;

    auto monthLabel = CCLabelBMFont::create(BSCalendarPopup::MONTHS[month - 1].c_str(), "bigFont.fnt");
    monthLabel->setScale(0.9f);
    monthLabel->setPosition(125.0f, 100.0f);
    m_mainLayer->addChild(monthLabel);

    auto yearLabel = CCLabelBMFont::create(std::to_string(year).c_str(), "bigFont.fnt");
    yearLabel->setScale(0.9f);
    yearLabel->setPosition(125.0f, 65.0f);
    m_mainLayer->addChild(yearLabel);

    auto prevMonthButton = CCMenuItemExt::createSpriteExtraWithFrameName("GJ_arrow_03_001.png", 0.8f, [this, minMonth, minYear, monthLabel, yearLabel](auto) {
        if (m_year == minYear && m_month == minMonth) return;
        if (m_month == 1) {
            m_year--;
            m_month = 12;
        }
        else m_month--;
        monthLabel->setString(BSCalendarPopup::MONTHS[m_month - 1].c_str());
        yearLabel->setString(std::to_string(m_year).c_str());
    });
    prevMonthButton->setPosition(25.0f, 100.0f);
    m_buttonMenu->addChild(prevMonthButton);

    auto nextMonthButtonSprite = CCSprite::createWithSpriteFrameName("GJ_arrow_03_001.png");
    nextMonthButtonSprite->setFlipX(true);
    nextMonthButtonSprite->setScale(0.8f);
    auto nextMonthButton = CCMenuItemExt::createSpriteExtra(nextMonthButtonSprite, [this, maxMonth, maxYear, monthLabel, yearLabel](auto) {
        if (m_year == maxYear && m_month == maxMonth) return;
        if (m_month == 12) {
            m_year++;
            m_month = 1;
        }
        else m_month++;
        monthLabel->setString(BSCalendarPopup::MONTHS[m_month - 1].c_str());
        yearLabel->setString(std::to_string(m_year).c_str());
    });
    nextMonthButton->setPosition(225.0f, 100.0f);
    m_buttonMenu->addChild(nextMonthButton);

    auto prevYearButton = CCMenuItemExt::createSpriteExtraWithFrameName("GJ_arrow_03_001.png", 0.8f, [this, minMonth, minYear, monthLabel, yearLabel](auto) {
        if (m_year == minYear) return;
        m_year--;
        if (m_year == minYear && m_month < minMonth) m_month = minMonth;
        monthLabel->setString(BSCalendarPopup::MONTHS[m_month - 1].c_str());
        yearLabel->setString(std::to_string(m_year).c_str());
    });
    prevYearButton->setPosition(25.0f, 65.0f);
    m_buttonMenu->addChild(prevYearButton);

    auto nextYearButtonSprite = CCSprite::createWithSpriteFrameName("GJ_arrow_03_001.png");
    nextYearButtonSprite->setFlipX(true);
    nextYearButtonSprite->setScale(0.8f);
    auto nextYearButton = CCMenuItemExt::createSpriteExtra(nextYearButtonSprite, [this, maxMonth, maxYear, monthLabel, yearLabel](auto) {
        if (m_year == maxYear) return;
        m_year++;
        if (m_year == maxYear && m_month > maxMonth) m_month = maxMonth;
        monthLabel->setString(BSCalendarPopup::MONTHS[m_month - 1].c_str());
        yearLabel->setString(std::to_string(m_year).c_str());
    });
    nextYearButton->setPosition(225.0f, 65.0f);
    m_buttonMenu->addChild(nextYearButton);

    auto confirmButton = CCMenuItemExt::createSpriteExtra(ButtonSprite::create("Confirm", "goldFont.fnt", "GJ_button_01.png", 0.8f), [this, callback](auto) {
        callback(m_year, m_month);
        onClose(nullptr);
    });
    confirmButton->setPosition(125.0f, 25.0f);
    m_buttonMenu->addChild(confirmButton);

    handleTouchPriority(this);

    return true;
}
