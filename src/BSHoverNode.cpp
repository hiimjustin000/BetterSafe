#include "BSHoverNode.hpp"

BSHoverNode* BSHoverNode::create(SafeLevel const& level, GJGameLevel* gameLevel, MiniFunction<void()> callback) {
    auto ret = new BSHoverNode();
    if (ret->init(level, gameLevel, callback)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool BSHoverNode::init(SafeLevel const& level, GJGameLevel* gameLevel, MiniFunction<void()> callback) {
    if (!CCLayer::init()) return false;

    setAnchorPoint({ 0.5f, 0.5f });
    setContentSize({ 80.0f, 70.0f });
    ignoreAnchorPointForPosition(false);

    CCTouchDispatcher::get()->registerForcePrio(this, 2);

    m_callback = callback;

    auto bg = CCScale9Sprite::createWithSpriteFrameName("square02_001.png");
    bg->setContentSize({ 80.0f, 70.0f });
    bg->setPosition(40.0f, 35.0f);
    bg->setColor({ 0, 0, 0 });
    bg->setOpacity(150);
    addChild(bg);

    auto dailyLabel = CCLabelBMFont::create(fmt::format("{} #{}", level.weekly ? "Weekly" : "Daily", level.timelyID).c_str(), "goldFont.fnt");
    dailyLabel->setPosition(40.0f, 65.0f);
    dailyLabel->setScale(0.3f);
    addChild(dailyLabel);

    auto nameLabel = CCLabelBMFont::create(level.name.c_str(), "bigFont.fnt");
    nameLabel->setPosition(40.0f, 55.0f);
    nameLabel->setScale(0.5f);
    addChild(nameLabel);

    auto creatorLabel = CCLabelBMFont::create(fmt::format("by {}", level.creator).c_str(), "goldFont.fnt");
    creatorLabel->setPosition(40.0f, 43.0f);
    creatorLabel->setScale(0.4f);
    addChild(creatorLabel);

    setContentWidth(std::max(nameLabel->getScaledContentWidth() + 6.0f, std::max(creatorLabel->getScaledContentWidth() + 6.0f, 80.0f)));

    bg->setContentWidth(getContentWidth());
    bg->setPositionX(getContentWidth() / 2);
    dailyLabel->setPositionX(getContentWidth() / 2);
    nameLabel->setPositionX(getContentWidth() / 2);
    creatorLabel->setPositionX(getContentWidth() / 2);

    auto starLayout = CCNode::create();
    starLayout->setPosition(getContentWidth() / 2, 30.0f);
    starLayout->setContentSize({ 80.0f, 15.0f });
    starLayout->setAnchorPoint({ 0.5f, 0.5f });
    starLayout->setLayout(RowLayout::create()->setGap(1.75f)->setAutoScale(false));
    addChild(starLayout);

    auto gsm = GameStatsManager::sharedState();
    auto starsLabel = CCLabelBMFont::create(std::to_string(level.stars).c_str(), "bigFont.fnt");
    starsLabel->setScale(0.4f);
    auto completedLevel = gsm->m_completedLevels->objectForKey(fmt::format("c_{}", level.id));
    starsLabel->setColor(completedLevel ? ccColor3B { 255, 255, 50 } : ccColor3B { 255, 255, 255 });
    starLayout->addChild(starsLabel);

    starLayout->addChild(CCSprite::createWithSpriteFrameName("star_small01_001.png"));

    for (int i = 1; i <= level.coins; i++) {
        auto coinStr = fmt::format("{}_{}", level.id, i);
        auto hasCoin = gsm->hasUserCoin(coinStr.c_str()) || gsm->hasPendingUserCoin(coinStr.c_str());
        auto coinSprite = CCSprite::createWithSpriteFrameName("usercoin_small01_001.png");
        if (level.coinsVerified) coinSprite->setColor(hasCoin ? ccColor3B { 255, 255, 255 } : ccColor3B { 165, 165, 165 });
        else coinSprite->setColor(hasCoin ? ccColor3B { 255, 175, 75 } : ccColor3B { 165, 113, 48 });
        starLayout->addChild(coinSprite);
    }

    starLayout->updateLayout();

    auto viewMenu = CCMenu::create();
    viewMenu->setPosition(getContentWidth() / 2, 12.0f);
    addChild(viewMenu);

    auto viewSprite = ButtonSprite::create("View", "goldFont.fnt", "GJ_button_01.png", 0.8f);
    viewSprite->setScale(0.7f);
    viewMenu->addChild(CCMenuItemExt::createSpriteExtra(viewSprite, [this, gameLevel](auto) {
        GameLevelManager::sharedState()->gotoLevelPage(gameLevel);
    }));

    auto closeButton = CCMenuItemExt::createSpriteExtraWithFrameName("GJ_closeBtn_001.png", 0.5f, [this] (auto) { close(); });
    closeButton->setPosition(viewMenu->convertToNodeSpace(convertToWorldSpace({ 0.0f, 70.0f })));
    viewMenu->addChild(closeButton);

    setTouchEnabled(true);
    handleTouchPriority(this);

    return true;
}

void BSHoverNode::registerWithTouchDispatcher() {
    CCTouchDispatcher::get()->addTargetedDelegate(this, -500, true);
}

void BSHoverNode::keyBackClicked() {
    close();
}

void BSHoverNode::close() {
    setTouchEnabled(false);
    removeFromParent();
    m_callback();
}

BSHoverNode::~BSHoverNode() {
    CCTouchDispatcher::get()->unregisterForcePrio(this);
}