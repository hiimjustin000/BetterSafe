#include "BSHoverNode.hpp"

using namespace geode::prelude;

BSHoverNode* BSHoverNode::create(SafeLevel const& level, GJGameLevel* gameLevel, std::function<void()> const& callback) {
    auto ret = new BSHoverNode();
    if (ret->init(level, gameLevel, callback)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool BSHoverNode::init(SafeLevel const& level, GJGameLevel* gameLevel, std::function<void()> const& callback) {
    if (!CCLayer::init()) return false;

    setAnchorPoint({ 0.5f, 0.5f });
    setContentSize({ 80.0f, 70.0f });
    ignoreAnchorPointForPosition(false);

    CCTouchDispatcher::get()->registerForcePrio(this, 2);

    m_callback = callback;

    m_background = CCScale9Sprite::create("square02_001.png");
    m_background->setContentSize({ 80.0f, 70.0f });
    m_background->setPosition({ 40.0f, 35.0f });
    m_background->setColor({ 0, 0, 0 });
    m_background->setOpacity(150);
    addChild(m_background);

    auto timedName = "";
    switch (level.type) {
        case GJTimedLevelType::Daily: timedName = "Daily"; break;
        case GJTimedLevelType::Weekly: timedName = "Weekly"; break;
        case GJTimedLevelType::Event: timedName = "Event"; break;
        default: break;
    }
    auto dailyLabel = CCLabelBMFont::create(fmt::format("{} #{}", timedName, level.timelyID).c_str(), "goldFont.fnt");
    dailyLabel->setPosition({ 40.0f, 65.0f });
    dailyLabel->setScale(0.3f);
    addChild(dailyLabel);

    auto nameLabel = CCLabelBMFont::create(gameLevel->m_levelName.c_str(), "bigFont.fnt");
    nameLabel->setPosition({ 40.0f, 55.0f });
    nameLabel->setScale(0.5f);
    addChild(nameLabel);

    auto creatorLabel = CCLabelBMFont::create(("by " + std::string(gameLevel->m_creatorName)).c_str(), "goldFont.fnt");
    creatorLabel->setPosition({ 40.0f, 43.0f });
    creatorLabel->setScale(0.4f);
    addChild(creatorLabel);

    setContentWidth(std::max(nameLabel->getScaledContentWidth() + 6.0f, std::max(creatorLabel->getScaledContentWidth() + 6.0f, 80.0f)));

    m_background->setContentWidth(getContentWidth());
    m_background->setPositionX(getContentWidth() / 2);
    dailyLabel->setPositionX(getContentWidth() / 2);
    nameLabel->setPositionX(getContentWidth() / 2);
    creatorLabel->setPositionX(getContentWidth() / 2);

    auto starLayout = CCNode::create();
    starLayout->setPosition({ getContentWidth() / 2, 30.0f });
    starLayout->setContentSize({ 80.0f, 15.0f });
    starLayout->setAnchorPoint({ 0.5f, 0.5f });
    starLayout->setLayout(RowLayout::create()->setGap(1.75f)->setAutoScale(false));
    addChild(starLayout);

    auto gsm = GameStatsManager::get();
    auto starsLabel = CCLabelBMFont::create(std::to_string(gameLevel->m_stars.value()).c_str(), "bigFont.fnt");
    starsLabel->setScale(0.4f);
    auto levelID = gameLevel->m_levelID.value();
    auto completedLevel = gsm->m_completedLevels->objectForKey(fmt::format("c_{}", levelID).c_str());
    starsLabel->setColor(completedLevel ? ccColor3B { 255, 255, 50 } : ccColor3B { 255, 255, 255 });
    starLayout->addChild(starsLabel);

    starLayout->addChild(CCSprite::createWithSpriteFrameName("star_small01_001.png"));

    auto coinsVerified = gameLevel->m_coinsVerified.value() > 0;
    for (int i = 1; i <= gameLevel->m_coins; i++) {
        auto coinStr = fmt::format("{}_{}", levelID, i);
        auto hasCoin = gsm->hasUserCoin(coinStr.c_str()) || gsm->hasPendingUserCoin(coinStr.c_str());
        auto coinSprite = CCSprite::createWithSpriteFrameName("usercoin_small01_001.png");
        if (coinsVerified) coinSprite->setColor(hasCoin ? ccColor3B { 255, 255, 255 } : ccColor3B { 165, 165, 165 });
        else coinSprite->setColor(hasCoin ? ccColor3B { 255, 175, 75 } : ccColor3B { 165, 113, 48 });
        starLayout->addChild(coinSprite);
    }

    starLayout->updateLayout();

    auto viewMenu = CCMenu::create();
    viewMenu->setPosition({ getContentWidth() / 2, 12.0f });
    addChild(viewMenu);

    auto viewSprite = ButtonSprite::create("View", "goldFont.fnt", "GJ_button_01.png", 0.8f);
    viewSprite->setScale(0.7f);
    viewMenu->addChild(CCMenuItemExt::createSpriteExtra(viewSprite, [this, gameLevel](auto) {
        GameLevelManager::get()->gotoLevelPage(gameLevel);
    }));

    auto closeButton = CCMenuItemExt::createSpriteExtraWithFrameName("GJ_closeBtn_001.png", 0.5f, [this] (auto) { close(); });
    closeButton->setPosition(viewMenu->convertToNodeSpace(convertToWorldSpace({ 0.0f, 70.0f })));
    viewMenu->addChild(closeButton);

    setTouchEnabled(true);
    setKeypadEnabled(true);
    handleTouchPriority(this);

    return true;
}

void BSHoverNode::close() {
    setTouchEnabled(false);
    setKeypadEnabled(false);
    removeFromParent();
    m_callback();
}

void BSHoverNode::keyBackClicked() {
    close();
}

void BSHoverNode::registerWithTouchDispatcher() {
    CCTouchDispatcher::get()->addTargetedDelegate(this, -500, true);
}

// Thanks Firee https://github.com/FireMario211/Object-Workshop/blob/v1.1.3/src/nodes/ExtPreviewBG.cpp#L40
bool BSHoverNode::ccTouchBegan(CCTouch* touch, CCEvent* event) {
    if (!CCLayer::ccTouchBegan(touch, event)) return false;

    auto touchPos = m_background->convertToNodeSpace(CCDirector::get()->convertToGL(touch->getLocationInView()));
    auto boundingBox = m_background->boundingBox();
    return CCRect {
        boundingBox.getMinX(),
        boundingBox.getMinY(),
        boundingBox.getMaxX(),
        boundingBox.getMaxY()
    }.intersectsRect({ touchPos.x, touchPos.y, touchPos.x, touchPos.y });
}

BSHoverNode::~BSHoverNode() {
    CCTouchDispatcher::get()->unregisterForcePrio(this);
}
