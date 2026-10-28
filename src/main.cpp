#include <Geode/modify/EditorWindowPopup.hpp>
#include <nwo5.silly-api/include/include.hpp>
#include "settings.hpp"

using namespace geode::prelude;
using namespace nwo5::prelude;

class $modify(EditorWindowPopupHook, EditorWindowPopup) {
    struct Fields {
        CCNode* topContainer = nullptr;
        NineSlice* topCenter = nullptr;
        NineSlice* leftCorner = nullptr;
        NineSlice* rightCorner = nullptr;
        NineSlice* scrollbar = nullptr;
        NineSlice* background = nullptr;
        CCSprite* closeButton = nullptr;

        bool mouseDown = false;
        float lastWindowScale = 0.0f;
        CCPoint lastScrollPos = CCPointZero;
    };

    static constexpr float CLOSE_OFFSET = 5.0f;
    static constexpr float CORNER_OFFSET = 5.0f;
    static constexpr float BUTTON_SCALE_FACTOR = 1.26f;
    static constexpr float SCROLLBAR_OFFSET = 2.5f;

    static void onModify(auto& self) {
        (void)self.setHookPriority("EditorWindowPopup::ccTouchBegan", Priority::Replace);
    }

    void updateCustom(float) {
        auto fields = m_fields.self();

        m_background->setVisible(false);

        const auto scaleFactor = 1.0f / (settings::lockScale ? m_mainLayer->getScale() : 1.0f);
        
        Setup(fields->background)
            .scaleMultiplier(scaleFactor)
            .pos(m_background)
            .size(m_background);

        for (auto nineSlice : fields->topContainer->getChildrenExt<NineSlice>()) {
            Setup(nineSlice)
                .scaleMultiplier(scaleFactor)
                .height(std::max(10.0f, (settings::specialZoneSize / 2)) * scaleFactor);
        }

        const auto width = (ui::w(m_background) / fields->topContainer->getScale()) - (ui::w(fields->topCenter) * 2);
        fields->topCenter->setScaleX(std::max(0.0f, width / ui::w(fields->topCenter)));

        Setup(fields->topContainer)
            .pos(ui::top(m_background))
            .updateLayout();

        if (fields->scrollbar) {
            // the scroll bar can b deleted and shit so this is safer
            auto scrollbarLayer = m_scrollContentLayer->getChildByType<CCLayerColor>();

            if (scrollbarLayer) {
                const auto last = fields->lastScrollPos;
                fields->lastScrollPos = ui::pos(scrollbarLayer);
                
                Setup(fields->scrollbar)
                    .pos(ui::x(scrollbarLayer) - SCROLLBAR_OFFSET, ui::y(scrollbarLayer))
                    .size(4.0f, ui::h(scrollbarLayer))
                    .opacity((last.x == fields->lastScrollPos.x && last.y != fields->lastScrollPos.y) ? 200 : 125)
                    .visible(true);

                scrollbarLayer->setVisible(false);
            }
            else {
                fields->scrollbar->setVisible(false);
            }
        }

        auto toggleAction = [] (bool condition, CCNode* node, geode::Function<CCAction*()> off, geode::Function<CCAction*()> on, bool elseCondition = true) {
            if (condition) {
                if (!node->getUserFlag("selected"_spr)) {
                    node->stopAllActions();
                    node->setUserFlag("selected"_spr, true);
                    node->runAction(on());
                }
            }
            else if (elseCondition) {
                if (node->getUserFlag("selected"_spr)) {
                    node->stopAllActions();
                    node->setUserFlag("selected"_spr, false);
                    node->runAction(off());
                }
            }
        };

        fields->closeButton->setPosition({
            ui::x(fields->topContainer) - ui::w(fields->topContainer) / 2 + CLOSE_OFFSET * scaleFactor,
            ui::y(fields->topContainer) + ui::h(fields->topContainer) - CLOSE_OFFSET * scaleFactor
        });

        if (m_closeSprite) {
            Setup(m_closeSprite)
                .pos(fields->closeButton)
                .scaleToFit(ui::sw(fields->closeButton))
                .hide();
        }

        if (fields->lastWindowScale != m_windowScale) {
            fields->lastWindowScale = m_windowScale;

            fields->closeButton->setScale(settings::closeButtonScale * scaleFactor);
        }

        toggleAction(
            m_closeSprite->getOpacity() != 255, fields->closeButton,
            [&] { return CCEaseBounceOut::create(CCScaleTo::create(0.4f, settings::closeButtonScale * scaleFactor)); },
            [&] { return CCEaseBounceOut::create(CCScaleTo::create(0.4f, settings::closeButtonScale * scaleFactor * BUTTON_SCALE_FACTOR)); }
        );

        Setup(fields->leftCorner)
            .pos(ui::bottomLeft(m_background) - CORNER_OFFSET * scaleFactor)
            .scale(scaleFactor);
        
        Setup(fields->rightCorner)
            .pos(ui::right(m_background).x + CORNER_OFFSET * scaleFactor, ui::bottom(m_background).y - CORNER_OFFSET * scaleFactor)
            .scale(scaleFactor);

        if (settings::alwaysShowResizeCorners) {
            Setup(fields->leftCorner)
                .userFlag("selected"_spr, true)
                .opacity(settings::cornerOpacity.get());

            Setup(fields->rightCorner)
                .userFlag("selected"_spr, true)
                .opacity(settings::cornerOpacity.get());
        }
        else {
            const auto mousePos = this->m_mainLayer->convertToNodeSpace(cocos::getMousePos());

            toggleAction(
                mousePos.fuzzyEquals(ui::bottomLeft(m_background), settings::specialZoneSize / 2) && !fields->mouseDown, fields->leftCorner,
                [] { return CCFadeTo::create(0.25f, 0); }, [] { return CCFadeTo::create(0.25f, settings::cornerOpacity); }, !fields->mouseDown
            );
            toggleAction(
                mousePos.fuzzyEquals({ui::bottomRight(m_background)}, settings::specialZoneSize / 2) && !fields->mouseDown, fields->rightCorner,
                [] { return CCFadeTo::create(0.25f, 0); }, [] { return CCFadeTo::create(0.25f, settings::cornerOpacity); }, !fields->mouseDown
            );
        }
    }

    bool initWindow(EditorUI* editorUI, float width, float height, int windowID, int unused) {
        if (!EditorWindowPopup::initWindow(editorUI, width, height, windowID, unused)) {
            return false;
        }

        if (!settings::enabled) {
            return true;
        }

        auto fields = m_fields.self();

        fields->background = ui::nineSlice(settings::noTopSeparator ? "background-no-line.png"_spr : "background.png"_spr)
            .id("custom-background"_spr)
            .size(m_background)
            .pos(m_background)
            .order(m_background)
            .color(settings::backgroundColor)
            .opacity(settings::opacity.get())
            .parent(m_mainLayer);

        fields->closeButton = ui::frame(resources::frame::CLOSE_BUTTON)
            .id("custom-close-button"_spr)
            .order(5)
            .parent(m_mainLayer);

        fields->leftCorner = ui::nineSlice("resize-corner-left.png"_spr)
            .id("left-corner"_spr)
            .userFlag("selected"_spr, false)
            .scaleMultiplier(0.25f)
            .size(20.0f, 20.0f)
            .anchor(Anchor::BottomLeft)
            .opacity(0)
            .parent(m_mainLayer);

        fields->rightCorner = ui::nineSlice("resize-corner-right.png"_spr)
            .id("right-corner"_spr)
            .userFlag("selected"_spr, false)
            .scaleMultiplier(0.25f)
            .size(20.0f, 20.0f)
            .anchor(Anchor::BottomRight)
            .opacity(0)
            .parent(m_mainLayer);

        // for lack of a horizontal/vertical slice
        fields->topCenter = ui::nineSlice("top-bar-center.png"_spr)
            .id("center-slice"_spr)
            .color(settings::topBarColor)
            .opacity(settings::opacity.get());

        fields->topContainer = ui::dummy(ui::row()
            .alignment(AxisAlignment::Center)
            .gap(0.0f)
            .autoScale(false)
            .grow()
        )
            .id("top-bar"_spr)
            .anchor(Anchor::Bottom)
            .children(
                ui::nineSlice("top-bar-left.png"_spr)
                    .id("left-slice"_spr)
                    .color(settings::topBarColor)
                    .opacity(settings::opacity.get()),
                fields->topCenter,
                ui::nineSlice("top-bar-right.png"_spr)
                    .id("right-slice"_spr)
                    .color(settings::topBarColor)
                    .opacity(settings::opacity.get())
            )
            .parent(m_mainLayer);

        fields->scrollbar = ui::nineSlice("scrollbar.png"_spr)
            .anchor(Anchor::TopRight)
            .order(10)
            .opacity(125)
            .scaleMultiplier(0.25f)
            .parent(m_scrollContentLayer);

        this->schedule(schedule_selector(EditorWindowPopupHook::updateCustom));

        this->addEventListener(MouseInputEvent(), [this] (const MouseInputData& pData) {
            if (pData.button == MouseInputData::Button::Left) {
                m_fields->mouseDown = pData.action == MouseInputData::Action::Press;
            }
        });
        
        return true;
    }

    // reimpl cctouchbegan so closebuttons gets targeted properly and also to apply specialzonesize
    bool ccTouchBegan(CCTouch* touch, CCEvent* event) {
        if (!settings::enabled) {
            return EditorWindowPopup::ccTouchBegan(touch, event);
        }
        
        if (!this->isVisible()) {
            return false;
        }

        const CCPoint location = touch->getLocation();
        const auto size = CCSize{m_windowWidth, m_windowHeight} * m_windowScale;

        if (m_touchID != -1) {
            return CCRect{
                ccSub(m_windowPosition - size / 2, settings::specialZoneSize / 2),
                ccAdd(size, settings::specialZoneSize)
            }.containsPoint(location);
        }

        if (m_inputNodes) {
            for (auto [_, input] : CCDictionaryExt<int, CCTextInputNode*>(m_inputNodes)) {
                if (input->m_selected) {
                    input->onClickTrackNode(false);
                }

                if (!input->isVisible()) {
                    continue;
                }

                const CCPoint local = input->convertToNodeSpace(location);
                const CCSize size = input->getContentSize();

                if (CCRect{-size / 2, size}.containsPoint(local)) {
                    m_touchID = touch->getID();

                    return true;
                }
            }
        }

        m_closeButtonRelease = false;

        if (m_buttonMenu && m_buttonMenu->ccTouchBegan(touch, event)) {
            m_menuTouch = true;
            m_touchID = touch->getID();
            
            return true;
        }

        const auto inCloseButton = this->isInCloseButton(location);

        if (
            !CCRect{
                ccSub(m_windowPosition - size / 2, settings::specialZoneSize / 2),
                ccAdd(size, settings::specialZoneSize)
            }.containsPoint(location) && !inCloseButton
        ) {
            return false;
        }

        m_touchID = touch->getID();

        if (inCloseButton) {
            m_closeButtonTouch = true;
            m_contentTouch = false;

            if (m_closeSprite) {
                m_closeSprite->setOpacity(150);
            }

            return true;
        }

        if (m_resizeMode != 0) {
            int corner = 0;

            if (
                CCRect{
                    ccSub(m_windowPosition - size / 2, settings::specialZoneSize / 2),
                    CCSize{settings::specialZoneSize, settings::specialZoneSize}
                }.containsPoint(location)
            ) {
                corner = 1;
            }
            else if (
                CCRect{
                    ccSub(CCPoint{m_windowPosition + CCSize{size.width, -size.height} / 2}, settings::specialZoneSize / 2),
                    CCSize{settings::specialZoneSize, settings::specialZoneSize}
                }.containsPoint(location)
            ) {
                corner = 2;
            }
            
            if (corner != 0) {
                m_resizeCorner = corner;

                const auto side = corner == 1 ? -1.0f : 1.0f;
                m_resizeAnchor = CCPoint{m_windowPosition.x - side * size.width / 2, m_windowPosition.y + size.height / 2};
                m_resizeOffset = location - CCPoint{m_windowPosition.x + side * size.width / 2, m_windowPosition.y - size.height / 2};
                m_contentTouch = false;

                return true;
            }
        }

        if (
            CCRect{
                ccSub(m_windowPosition + CCSize{-size.width, size.height} / 2, settings::specialZoneSize / 2),
                CCSize{size.width + settings::specialZoneSize, settings::specialZoneSize}
            }.containsPoint(location)
        ) {
            m_draggingWindow = true;
            m_dragOffset = location - m_windowPosition;
            m_contentTouch = false;

            return true;
        }

        if (m_scrollContentLayer) {
            const CCPoint local = m_scrollContentLayer->convertToNodeSpace(location);

            if (
                CCRect{
                    CCPointZero, CCSize{m_scrollContentLayer->m_viewSize.width, m_scrollContentLayer->m_viewSize.height}
                }.containsPoint(local)
            ) {
                m_scrollContentLayer->handleTouchBegan(touch);

                if (m_scrollContentMenu && m_scrollContentMenu->ccTouchBegan(touch, event)) {
                    m_contentTouch = true;
                    
                    return true;
                }
            }
        }

        m_contentTouch = false;

        if (m_allowDraggingAnywhere) {
            if (m_scrollContentLayer && touch->getID() == m_scrollContentLayer->m_touchID) {
                m_scrollContentLayer->m_touchID = -1;
                m_scrollContentLayer->m_velocity = 0.0f;
                m_scrollContentLayer->m_isDragging = false;
                m_scrollContentLayer->unscheduleUpdate();
            }
            
            m_draggingWindow = true;
            m_dragOffset = location - m_windowPosition;
        }
        return true;
    }
};