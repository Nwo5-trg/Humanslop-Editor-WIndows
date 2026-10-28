#pragma once

#include <nwo5.silly-api/include/cache/include.hpp>

using namespace nwo5::cache::prelude;

namespace settings {
    inline SettingCache<bool> enabled{"enabled"};
    inline SettingCache<cocos2d::ccColor3B> backgroundColor{"background-color"};
    inline SettingCache<cocos2d::ccColor3B> topBarColor{"top-bar-color"};
    inline SettingCache<int> opacity{"opacity"};
    inline SettingCache<bool> alwaysShowResizeCorners{"always-show-resize-corners"};
    inline SettingCache<int> cornerOpacity{"corner-opacity"};
    inline SettingCache<bool> noTopSeparator{"no-top-separator"};
    inline SettingCache<bool> lockScale{"lock-scale"};
    inline SettingCache<float> closeButtonScale{"close-button-scale"};
    inline SettingCache<float> specialZoneSize{"special-zone-size"};
}