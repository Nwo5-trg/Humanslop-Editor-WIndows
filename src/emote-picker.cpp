#include <nwo5.silly-api/include/include.hpp>
#include "settings.hpp"

using namespace geode::prelude;
using namespace nwo5::prelude;

static constexpr int INIT_EMOTE_PICKER_ADDRESS = 0x7e96b8;
static constexpr int LAYOUT_EMOTES_EMOTE_PICKER_ADDRESS = 0x7ea548;

// so fucking dumb omg
template<typename FieldsClass>
struct FieldsButEvil {
    static auto getFieldsID() {
        static auto str = [] {
            auto [ptr, size] = arc::getTypename<FieldsClass>();
            return ""_spr + string::replace(string::toLower(std::string{ptr, size}), " ", "-") + "-fields";
        }();
        return str;
    };

    static FieldsClass* self(CCNode* self) {
        return &static_cast<ObjWrapper<FieldsClass>*>(self->getUserObject(getFieldsID()))->getValue();
    }

    static void setup(CCNode* self) {
        self->setUserObject(getFieldsID(), ObjWrapper<FieldsClass>::create({}));
    }
};


struct EmotePickerPopupHook {
    struct Fields {
        CCNode* scrollContent = nullptr;

        std::array<Label*, 8> labels;
    };

    // this is so silly just to avoid waiting a couple days for geode to work lol
    static FieldsButEvil<Fields> m_fields;

    static constexpr float LABEL_SPACING = 50.0f;

    static bool init(EmotePickerPopup* self, CCTextInputNode* p0, CCPoint p1, int p2, int p3) {
        m_fields.setup(self);

        if (!reinterpret_cast<bool(*)(EmotePickerPopup*, CCTextInputNode*, CCPoint, int, int)>(base::get() + INIT_EMOTE_PICKER_ADDRESS)(self, p0, p1, p2, p3)) {
            return false;
        }

        auto fields = m_fields.self(self);

        self->addOnEnterCallback([self, fields] {
            fields->scrollContent = self->m_scrollLayer->getChildByType<CCNode>()->getChildByType<CCMenu>();

            auto makeLabel = [fields] (ZStringView text) -> Label* {
                return ui::label(text, Font::Gold)
                    .id("{}"_spr, string::toLower(text))
                    .scale(0.75f)
                    .parent(fields->scrollContent);
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

            layoutEmoteGrid(self);
        });

        return true;
    }

    static void layoutEmoteGrid(EmotePickerPopup* self) {
        reinterpret_cast<void(*)(EmotePickerPopup*)>(base::get() + LAYOUT_EMOTES_EMOTE_PICKER_ADDRESS)(self);

        auto fields = m_fields.self(self);

        if (!fields->scrollContent || !self->m_scrollLayer) {
            return;
        }

        const auto offsetY = fields->scrollContent->getChildrenExt().front()->getPositionY();

        // this runs twice or smth no clue y but thats y i dvide by 2
        for (auto node : fields->scrollContent->getChildrenExt()) {
            Setup(node).moveY(-(LABEL_SPACING + offsetY) / 2);
        }

        const auto firstPos = fields->scrollContent->getChildrenExt().front()->getPosition();
        // const auto labelX = firstPos.x - 10.0f;
        const auto labelX = ui::w(self->m_scrollLayer) / 2;

        // i give up recents positioning is fucked deal with it
        fields->labels.front()->setPosition(labelX, firstPos.y / 2 + ui::sh(fields->labels.front()) / 2);
        int labelIndex = 1;
        float lastY = firstPos.y;
        for (int i = 0; i < fields->scrollContent->getChildrenCount(); i++) {
            auto emoteButton = fields->scrollContent->getChildByIndex(i);

            if (emoteButton->getTag() <= 0) {
                continue;
            }

            const auto y = ui::y(emoteButton);

            // i still need the loop to run even after labels are positioned so i can get the lowest node
            if (labelIndex < fields->labels.size()) {
                if (std::abs(y - lastY) >= 38.0f) {
                    for (int j = i; j < fields->scrollContent->getChildrenCount(); j++) {
                        auto node = fields->scrollContent->getChildByIndex(j);

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

        const auto height = std::max(self->m_scrollLayer->getContentHeight(), 5.0f - (lastY - 38.0f));
        self->m_scrollLayer->setContentHeight(height);
    }
};

$on_mod(Loaded) {
    (void)Mod::get()->hook(
        reinterpret_cast<void*>(geode::base::get() + INIT_EMOTE_PICKER_ADDRESS),
        &EmotePickerPopupHook::init,
        "EmotePickerPopup::init"
    );

    (void)Mod::get()->hook(
        reinterpret_cast<void*>(geode::base::get() + LAYOUT_EMOTES_EMOTE_PICKER_ADDRESS),
        &EmotePickerPopupHook::layoutEmoteGrid,
        "EmotePickerPopup::layoutEmoteGrid"
    );
}