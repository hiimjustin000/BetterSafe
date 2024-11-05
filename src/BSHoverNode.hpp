#include "BetterSafe.hpp"

class BSHoverNode : public cocos2d::CCLayer {
protected:
    cocos2d::extension::CCScale9Sprite* m_background;
    std::function<void()> m_callback;

    bool init(SafeLevel const&, GJGameLevel*, std::function<void()> const&);
public:
    static BSHoverNode* create(SafeLevel const&, GJGameLevel*, std::function<void()> const&);

    void close();
    void keyBackClicked() override;
    void registerWithTouchDispatcher() override;
    bool ccTouchBegan(cocos2d::CCTouch*, cocos2d::CCEvent*) override;

    ~BSHoverNode() override;
};
