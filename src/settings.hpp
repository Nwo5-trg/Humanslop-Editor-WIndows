#pragma once

#include <nwo5.silly-api/include/cache/include.hpp>

using namespace nwo5::cache::prelude;

namespace settings {
    inline SettingCache<cocos2d::ccColor3B> backgroundColor{"background-color"};
    inline SettingCache<cocos2d::ccColor3B> topBarColor{"top-bar-color"};
    inline SettingCache<int> opacity{"opacity"};
    inline SettingCache<bool> noTopSeparator{"no-top-separator"};
    inline SettingCache<float> closeButtonScale{"close-button-scale"};
}