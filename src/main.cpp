#include <Geode/modify/EditorWindowPopup.hpp>
#include <nwo5.silly-api/include/include.hpp>
#include "settings.hpp"

using namespace geode::prelude;
using namespace nwo5::prelude;

class $modify(EditorWindowPopupHook, EditorWindowPopup) {
    struct Fields {
        CCNode* topContainer = nullptr;
        CCSprite* topCenter = nullptr;
        NineSlice* leftCorner = nullptr;
        NineSlice* rightCorner = nullptr;
        NineSlice* scrollbar = nullptr;
        CCSprite* closeButton = nullptr;

        bool mouseDown = false;
        CCPoint lastScrollPos = CCPointZero;
    };

    static constexpr float CLOSE_OFFSET = 5.0f;
    static constexpr float CORNER_OFFSET = 2.5f;
    static constexpr float BUTTON_SCALE_FACTOR = 1.26f;
    static constexpr float SCROLLBAR_OFFSET = 2.5f;

    void updateCustom(float) {
        auto fields = m_fields.self();

        const auto width = (ui::w(m_background) / fields->topContainer->getScale()) - (ui::w(fields->topCenter) * 2);
        fields->topCenter->setScaleX(std::max(0.0f, width / ui::w(fields->topCenter)));

        Setup(fields->topCenter)
            .pos(ui::top(m_background))
            .updateLayout();

        if (fields->scrollbar) {
            // the scroll bar can b deleted and shit so this is safer
            auto scrollbarLayer = m_scrollLayer->getChildByType<CCLayerColor>();

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
            ui::x(fields->topContainer) - ui::w(fields->topContainer) / 2 + CLOSE_OFFSET,
            ui::x(fields->topContainer) + ui::h(fields->topContainer) - CLOSE_OFFSET
        });

        if (m_closeSprite) {
            Setup(m_closeSprite)
                .pos(fields->closeButton)
                .scaleToFit(ui::sw(fields->closeButton))
                .hide();
        }

        toggleAction(
            m_closeSprite->getOpacity() != 255, fields->closeButton,
            [] { return CCEaseBounceOut::create(CCScaleTo::create(0.4f, settings::closeButtonScale)); },
            [] { return CCEaseBounceOut::create(CCScaleTo::create(0.4f, settings::closeButtonScale * BUTTON_SCALE_FACTOR)); }
        );

        const auto mousePos = this->m_mainLayer->convertToNodeSpace(cocos::getMousePos());

        fields->leftCorner->setPosition(ui::bottomLeft(m_background) - CORNER_OFFSET);
        toggleAction(
            mousePos.fuzzyEquals(ui::bottomLeft(m_background), 12.0f / 2) && !fields->mouseDown, fields->leftCorner,
            [] { return CCFadeTo::create(0.25f, 0); }, [] { return CCFadeTo::create(0.25f, 150); }, !fields->mouseDown
        );

        fields->rightCorner->setPosition(ui::right(m_background).x + CORNER_OFFSET, ui::bottom(m_background).y - CORNER_OFFSET);
        toggleAction(
            mousePos.fuzzyEquals({ui::bottomRight(m_background)}, 12.0f / 2) && !fields->mouseDown, fields->rightCorner,
            [] { return CCFadeTo::create(0.25f, 0); }, [] { return CCFadeTo::create(0.25f, 150); }, !fields->mouseDown
        );
    }

    bool initWindow(EditorUI* editorUI, float width, float height, int windowID, int unused) {
        if (!EditorWindowPopup::initWindow(editorUI, width, height, windowID, unused)) {
            return false;
        }

        auto fields = m_fields.self();

        m_background = this->m_mainLayer->getChildByType<CCScale9Sprite>();

        const auto size = m_background->getContentSize();
        auto texture = CCTextureCache::get()->addImage(settings::noTopSeparator ? "background-no-line.png"_spr : "background.png"_spr, false);
        m_background->setSpriteFrame(
            CCSpriteFrame::createWithTexture(texture, {CCPointZero, texture->getContentSize()})
        );
        Setup(m_background)
            .color(settings::backgroundColor)
            .opacity(settings::opacity.get())
            .size(size);

        fields->closeButton = ui::frame(resources::frame::CLOSE_BUTTON)
            .id("custom-close-button"_spr)
            .order(5)
            .scale(settings::closeButtonScale)
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
        fields->topCenter = ui::sprite("top-bar-center.png"_spr)
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
                ui::sprite("top-bar-left.png"_spr)
                    .color(settings::topBarColor)
                    .opacity(settings::opacity.get()),
                fields->topCenter,
                ui::sprite("top-bar-left.png"_spr)
                    .color(settings::topBarColor)
                    .opacity(settings::opacity.get())
            )
            .parent(m_mainLayer);

        this->addOnEnterCallback([this, fields] {
            m_scrollLayer->addChild(fields->scrollbar);
        });

        this->schedule(schedule_selector(EditorWindowPopupHook::updateCustom));

        this->addEventListener(MouseInputEvent(), [this] (const MouseInputData& pData) {
            if (pData.button == MouseInputData::Button::Left) {
                m_fields->mouseDown = pData.action == MouseInputData::Action::Press;
            }
        });
        
        return true;
    }
};