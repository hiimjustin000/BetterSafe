#include "BSHoverNode.hpp"

class BSCalendarPopup : public geode::Popup<DailyLevelPage*, bool>, public LevelManagerDelegate {
public:
    inline static std::vector<int> DAYS_IN_MONTH = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    inline static std::vector<std::string> MONTHS = {
        "January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"
    };
protected:
    EventListener<web::WebTask> m_listener;
    bool m_weekly;
    int m_year;
    int m_firstYear;
    int m_currentYear;
    int m_month;
    int m_firstMonth;
    int m_currentMonth;
    CCArray* m_levels;
    CCMenu* m_calendarMenu;
    CCLabelBMFont* m_monthLabel;
    CCMenuItemSpriteExtra* m_monthButton;
    BSHoverNode* m_hoverNode;
    LoadingCircle* m_loadingCircle;
    CCMenuItemSpriteExtra* m_selected;
    CCMenuItemSpriteExtra* m_prevButton;
    CCMenuItemSpriteExtra* m_nextButton;
    CCMenuItemSpriteExtra* m_firstButton;
    CCMenuItemSpriteExtra* m_lastButton;

    bool setup(DailyLevelPage*, bool) override;

    void createWeekdayLabel(const char* text, int idx);
    void loadMonth();
    void setupMonth();
public:
    static BSCalendarPopup* create(DailyLevelPage*, bool);

    void loadLevelsFinished(CCArray* levels, const char* key, int) override {
        loadLevelsFinished(levels, key);
    }
    void loadLevelsFinished(CCArray*, const char*) override;
    void loadLevelsFailed(const char* key, int) override {
        loadLevelsFailed(key);
    }
    void loadLevelsFailed(const char*) override {
        m_loadingCircle->setVisible(false);
        m_loadingCircle->fadeAndRemove();
        FLAlertLayer::create("Load Failed", "Failed to load safe levels. Please try again later.", "OK")->show();
    }
    void setupPageInfo(gd::string, const char*) override {}

    ~BSCalendarPopup() override;
};

class BSSelectPopup : public geode::Popup<int, int, int, int, int, int, MiniFunction<void(int, int)>> {
protected:
    int m_year;
    int m_month;

    bool setup(int, int, int, int, int, int, MiniFunction<void(int, int)>) override;
public:
    static BSSelectPopup* create(int, int, int, int, int, int, MiniFunction<void(int, int)>);
};
