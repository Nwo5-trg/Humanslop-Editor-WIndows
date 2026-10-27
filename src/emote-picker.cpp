#include <nwo5.silly-api/include/include.hpp>
#include "settings.hpp"

using namespace geode::prelude;
using namespace nwo5::prelude;

#include <Geode/modify/EmotePickerPopup.hpp>

class $modify(EmotePickerPopup) {
    struct Fields {
        std::array<Label*, 8> labels;
    };

    static constexpr float LABEL_SPACING = 50.0f;

    bool init(CCTextInputNode* p0, CCPoint p1, int p2, int p3) {
        if (!EmotePickerPopup::init(p0, p1, p2, p3)) {
            return false;
        }

        if (!settings::enabled) {
            return true;
        }

        auto fields = m_fields.self();

        this->addOnEnterCallback([this, fields] {
            auto makeLabel = [this] (ZStringView text) -> Label* {
                return ui::label(text, Font::Gold)
                    .id("{}"_spr, string::toLower(text))
                    .scale(0.75f)
                    .parent(this->m_scrollContentMenu);
            };

            fields->labels = {
                makeLabel("Recents"),
                makeLabel("Ratings"),
                makeLabel("Faces"),
                makeLabel("Hands"),
                makeLabel("Bubbles"),
                makeLabel("Misc"),
                makeLabel("Symbols"),
                makeLabel("Boomlings")
            };

            this->layoutEmoteGrid();
        });
        
        return true;
    }
    
    void layoutEmoteGrid() {
        EmotePickerPopup::layoutEmoteGrid();

        auto fields = m_fields.self();

        if (!this->m_scrollContentMenu || !this->m_scrollContentLayer || ! fields->labels.front()) {
            return;
        }

        const auto offsetY = this->m_scrollContentMenu->getChildrenExt().front()->getPositionY();

        // this runs twice or smth no clue y but thats y i dvide by 2
        for (auto node : this->m_scrollContentMenu->getChildrenExt()) {
            Setup(node).moveY((LABEL_SPACING + offsetY) / -2);
        }

        const auto firstPos = this->m_scrollContentMenu->getChildrenExt().front()->getPosition();
        const auto labelX = ui::w(this->m_scrollContentLayer) / 2;

        // i give up recents positioning is fucked deal with it
        fields->labels.front()->setPosition(labelX, firstPos.y / 2 + ui::sh(fields->labels.front()) / 2);
        int labelIndex = 1;
        float lastY = firstPos.y;
        for (int i = 0; i < this->m_scrollContentMenu->getChildrenCount(); i++) {
            auto emoteButton = this->m_scrollContentMenu->getChildByIndex(i);

            if (emoteButton->getTag() <= 0) {
                continue;
            }

            const auto y = ui::y(emoteButton);

            // i still need the loop to run even after labels are positioned so i can get the lowest node
            if (labelIndex < fields->labels.size()) {
                if (std::abs(y - lastY) >= 38.0f) {
                    for (int j = i; j < this->m_scrollContentMenu->getChildrenCount(); j++) {
                        auto node = this->m_scrollContentMenu->getChildByIndex(j);

                        if (node->getTag() <= 0) {
                            continue;
                        }

                        Setup(node).moveY(-LABEL_SPACING + 38.0f);
                    }

                    fields->labels[labelIndex++]->setPosition(labelX, (y + lastY) / 2);
                }
            }

            lastY = y;
        }

        const auto height = std::max(this->m_scrollContentLayer->m_viewSize.height, 5.0f - (lastY - 38.0f));
        this->m_scrollContentLayer->setContentHeight(height);
    }
};