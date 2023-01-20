#include "main.hpp"
#include "config.hpp"

// #include "HMUI/Touchable.hpp"
// #include "UnityEngine/GameObject.hpp"

void DidActivate(HMUI::ViewController* self, bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling) {
    if(!firstActivation)
        return;

    // self->get_gameObject()->AddComponent<HMUI::Touchable*>();
    auto vertical = QuestUI::BeatSaberUI::CreateVerticalLayoutGroup(self);
    vertical->set_childControlHeight(false);
    vertical->set_childForceExpandHeight(false);
    vertical->set_spacing(1);

    AddConfigValueToggle(vertical, getConfig().Remove);
    QuestUI::BeatSaberUI::CreateSliderSetting(vertical, getConfig().Width.GetName(), 0.01, getConfig().Width.GetValue(), 0.01, 1, [](float value) {
        getConfig().Width.SetValue(value);
    });
    AddConfigValueIncrementFloat(vertical, getConfig().Opacity, 1, 0.1, 0.1, 1);
    AddConfigValueIncrementFloat(vertical, getConfig().Length, 1, 0.2, 0.2, 2);
    AddConfigValueToggle(vertical, getConfig().Rainbow);
    AddConfigValueToggle(vertical, getConfig().Colors);
    AddConfigValueColorPicker(vertical, getConfig().LeftColor);
    AddConfigValueColorPicker(vertical, getConfig().RightColor);
}

using UnityEngine::Color;

inline float RainbowMod(float val) {
    float mod = defaultTrailLength;
    while(val >= mod)
        val -= mod;
    return val;
}

static constexpr std::array<Color, 6> rainbow = {{
    {1, 0, 0, 1},
    {1, 1, 0, 1},
    {0, 1, 0, 1},
    {0, 1, 1, 1},
    {0, 0, 1, 1},
    {1, 0, 1, 1}
}};

Color RainbowManager::ColorAtTime(float time) {
    time = RainbowMod(time + timeOffset) / defaultTrailLength;
    int min = time * rainbow.size();
    float lerp = (time * rainbow.size()) - min;
    Color first = rainbow[min % rainbow.size()];
    Color second = rainbow[(min + 1) % rainbow.size()];
    return Color::Lerp(first, second, lerp);
}

void RainbowManager::AddTime(float time) {
    timeOffset = RainbowMod(timeOffset + time);
}
