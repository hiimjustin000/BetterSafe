#include "BetterSafe.hpp"

class BSHoverNode : public CCLayer {
protected:
    MiniFunction<void()> m_callback;

    bool init(SafeLevel const&, GJGameLevel*, MiniFunction<void()>);
public:
    static BSHoverNode* create(SafeLevel const&, GJGameLevel*, MiniFunction<void()>);

    void keyBackClicked() override;
    void registerWithTouchDispatcher() override;
    void close();

    ~BSHoverNode() override;
};
