#include "main.hpp"
#include "config.hpp"

#include "questui/shared/QuestUI.hpp"

using namespace GlobalNamespace;
using namespace UnityEngine;

static ModInfo modInfo;

Logger& getLogger() {
    static Logger* logger = new Logger(modInfo);
    return *logger;
}

#include "GlobalNamespace/SaberTrail.hpp"

static SaberTrail* leftSaber;

static RainbowManager leftRainbow, rightRainbow;

static bool returnRainbowColor = false;
static Color rainbowColor;

#include "GlobalNamespace/SaberModelController.hpp"
#include "GlobalNamespace/Saber.hpp"
#include "GlobalNamespace/SaberType.hpp"

// distinguish between left and right saber trails
MAKE_HOOK_MATCH(SaberModelController_Init, &SaberModelController::Init, void, SaberModelController* self, Transform* parent, Saber* saber) {

    if(saber->get_saberType() == SaberType::SaberA)
        leftSaber = self->saberTrail;

    SaberModelController_Init(self, parent, saber);
}

// set color, opacity, and duration
MAKE_HOOK_MATCH(SaberTrail_Init, &SaberTrail::Init, void, SaberTrail* self) {

    self->trailDuration = getConfig().Length.GetValue() * defaultTrailLength;
    if(getConfig().Rainbow.GetValue())
        self->whiteSectionMaxDuration = self->trailDuration;
    if(getConfig().Colors.GetValue()) {
        if(self == leftSaber)
            self->color = getConfig().LeftColor.GetValue().get_linear();
        else
            self->color = getConfig().RightColor.GetValue().get_linear();
    }
    self->color.a = getConfig().Opacity.GetValue();

    SaberTrail_Init(self);
}

#include "GlobalNamespace/IBladeMovementData.hpp"
#include "GlobalNamespace/BladeMovementDataElement.hpp"

// update rainbow time
MAKE_HOOK_MATCH(SaberTrail_LateUpdate, &SaberTrail::LateUpdate, void, SaberTrail* self) {

    if(getConfig().Rainbow.GetValue()) {
        int samplesProgressed = floor((self->movementData->get_lastAddedData().time - self->lastTrailElementTime) / self->sampleStep);
        if(self == leftSaber)
            leftRainbow.AddTime(self->sampleStep * samplesProgressed);
        else
            rightRainbow.AddTime(self->sampleStep * samplesProgressed);
        returnRainbowColor = true;
    }
    SaberTrail_LateUpdate(self);

    returnRainbowColor = false;
}

#include "GlobalNamespace/TrailElementCollection.hpp"

// update rainbow color
MAKE_HOOK_MATCH(TrailElementCollection_Interpolate, &TrailElementCollection::Interpolate, void, TrailElementCollection* self, float t, ByRef<TrailElementCollection::InterpolationState> lerpState, ByRef<Vector3> position, ByRef<Vector3> normal, ByRef<float> time) {

    if(self == leftSaber->trailElementCollection)
        rainbowColor = leftRainbow.ColorAtTime(time.heldRef);
    else
        rainbowColor = rightRainbow.ColorAtTime(time.heldRef);

    TrailElementCollection_Interpolate(self, t, lerpState, position, normal, time);
}

// point to get the rainbow color into the trail update function
MAKE_HOOK_MATCH(Color_LerpUnclamped, &Color::LerpUnclamped, Color, Color a, Color b, float t) {

    if(returnRainbowColor)
        return rainbowColor;

    return Color_LerpUnclamped(a, b, t);
}

// set trail width
MAKE_HOOK_MATCH(SaberTrail_GetTrailWidth, &SaberTrail::GetTrailWidth, float, SaberTrail* self, BladeMovementDataElement lastAddedData) {

    auto value = SaberTrail_GetTrailWidth(self, lastAddedData);

    if(getConfig().Remove.GetValue())
        return 0;
    return value * getConfig().Width.GetValue();
}

// recenter trail based on width
MAKE_HOOK_MATCH(TrailElementCollection_SetHeadData, &TrailElementCollection::SetHeadData, void, TrailElementCollection* self, Vector3 start, Vector3 end, float time) {

    start = Vector3::Lerp(end, start, getConfig().Width.GetValue());

    TrailElementCollection_SetHeadData(self, start, end, time);
}

extern "C" void setup(ModInfo& info) {
    info.id = MOD_ID;
    info.version = VERSION;
    modInfo = info;

    getConfig().Init(modInfo);

    getLogger().info("Completed setup!");
}

extern "C" void load() {
    il2cpp_functions::Init();

    QuestUI::Init();
    QuestUI::Register::RegisterMainMenuModSettingsViewController(modInfo, "Saber Trailer", DidActivate);

    getLogger().info("Installing hooks...");
    INSTALL_HOOK(getLogger(), SaberModelController_Init);
    INSTALL_HOOK(getLogger(), SaberTrail_Init);
    INSTALL_HOOK(getLogger(), SaberTrail_LateUpdate);
    INSTALL_HOOK(getLogger(), TrailElementCollection_Interpolate);
    INSTALL_HOOK(getLogger(), Color_LerpUnclamped);
    INSTALL_HOOK(getLogger(), TrailElementCollection_SetHeadData);
    INSTALL_HOOK(getLogger(), SaberTrail_GetTrailWidth);
    getLogger().info("Installed all hooks!");
}
