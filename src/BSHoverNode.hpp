#include "BetterSafe.hpp"

class BSHoverNode : public CCLayer {
protected:
    CCScale9Sprite* m_background;
    MiniFunction<void()> m_callback;

    bool init(SafeLevel const&, GJGameLevel*, MiniFunction<void()>);
public:
    static BSHoverNode* create(SafeLevel const&, GJGameLevel*, MiniFunction<void()>);

    void close();
    void keyBackClicked() override;
    void registerWithTouchDispatcher() override;
    bool ccTouchBegan(CCTouch*, CCEvent*) override;

    ~BSHoverNode() override;
};
