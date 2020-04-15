////////////////////////////////////////////////////////////////////////////////////////////////////
//                               This file is part of CosmoScout VR                               //
//      and may be used under the terms of the MIT license. See the LICENSE file for details.     //
//                        Copyright: (c) 2019 German Aerospace Center (DLR)                       //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Plugin.hpp"

#include "../../../src/cs-core/GraphicsEngine.hpp"
#include "../../../src/cs-core/GuiManager.hpp"
#include "../../../src/cs-core/InputManager.hpp"
#include "../../../src/cs-core/SolarSystem.hpp"
#include "../../../src/cs-core/TimeControl.hpp"
#include "../../../src/cs-scene/CelestialAnchorNode.hpp"
#include "../../../src/cs-utils/convert.hpp"

#include <VistaKernel/GraphicsManager/VistaOpenGLNode.h>
#include <VistaKernel/GraphicsManager/VistaSceneGraph.h>
#include <VistaKernel/VistaSystem.h>
#include <VistaKernelOpenSGExt/VistaOpenSGMaterialTools.h>

#include <memory>

////////////////////////////////////////////////////////////////////////////////////////////////////

EXPORT_FN cs::core::PluginBase* create() {
  return new csp::customwebui::Plugin;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

EXPORT_FN void destroy(cs::core::PluginBase* pluginBase) {
  delete pluginBase;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace csp::customwebui {

////////////////////////////////////////////////////////////////////////////////////////////////////

void from_json(nlohmann::json const& j, Plugin::Settings::SideBarItem& o) {
  cs::core::Settings::deserialize(j, "name", o.mName);
  cs::core::Settings::deserialize(j, "icon", o.mIcon);
  cs::core::Settings::deserialize(j, "html", o.mHTML);
}

void to_json(nlohmann::json& j, Plugin::Settings::SideBarItem const& o) {
  cs::core::Settings::serialize(j, "name", o.mName);
  cs::core::Settings::serialize(j, "icon", o.mIcon);
  cs::core::Settings::serialize(j, "html", o.mHTML);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void from_json(nlohmann::json const& j, Plugin::Settings::SpaceItem& o) {
  cs::core::Settings::deserialize(j, "center", o.mCenter);
  cs::core::Settings::deserialize(j, "icon", o.mFrame);
  cs::core::Settings::deserialize(j, "longitude", o.mLongitude);
  cs::core::Settings::deserialize(j, "latitude", o.mLatitude);
  cs::core::Settings::deserialize(j, "elevation", o.mElevation);
  cs::core::Settings::deserialize(j, "scale", o.mScale);
  cs::core::Settings::deserialize(j, "width", o.mWidth);
  cs::core::Settings::deserialize(j, "height", o.mHeight);
  cs::core::Settings::deserialize(j, "html", o.mHTML);
}

void to_json(nlohmann::json& j, Plugin::Settings::SpaceItem const& o) {
  cs::core::Settings::serialize(j, "center", o.mCenter);
  cs::core::Settings::serialize(j, "icon", o.mFrame);
  cs::core::Settings::serialize(j, "longitude", o.mLongitude);
  cs::core::Settings::serialize(j, "latitude", o.mLatitude);
  cs::core::Settings::serialize(j, "elevation", o.mElevation);
  cs::core::Settings::serialize(j, "scale", o.mScale);
  cs::core::Settings::serialize(j, "width", o.mWidth);
  cs::core::Settings::serialize(j, "height", o.mHeight);
  cs::core::Settings::serialize(j, "html", o.mHTML);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void from_json(nlohmann::json const& j, Plugin::Settings& o) {
  cs::core::Settings::deserialize(j, "sidebar-items", o.mSideBarItems);
  cs::core::Settings::deserialize(j, "space-items", o.mSpaceItems);
}

void to_json(nlohmann::json& j, Plugin::Settings const& o) {
  cs::core::Settings::serialize(j, "sidebar-items", o.mSideBarItems);
  cs::core::Settings::serialize(j, "space-items", o.mSpaceItems);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void Plugin::init() {

  // First parse the settings.
  mPluginSettings = mAllSettings->mPlugins.at("csp-custom-web-ui");

  // Then add all sidebar tabs.
  for (auto const& settings : mPluginSettings.mSideBarItems) {
    mGuiManager->addPluginTabToSideBar(settings.mName, settings.mIcon, settings.mHTML);
  }

  // Then add all space items.
  auto pSG = GetVistaSystem()->GetGraphicsManager()->GetSceneGraph();
  for (auto const& settings : mPluginSettings.mSpaceItems) {
    mSpaceItems.resize(mSpaceItems.size() + 1);
    SpaceItem& item = mSpaceItems.back();

    item.mAnchor = std::make_shared<cs::scene::CelestialAnchorNode>(
        pSG->GetRoot(), pSG->GetNodeBridge(), "", settings.mCenter, settings.mFrame);
    item.mScale = settings.mScale;

    glm::dvec2 lngLat(settings.mLongitude, settings.mLatitude);
    lngLat        = cs::utils::convert::toRadians(lngLat);
    double height = 0.0;
    auto   parent = mSolarSystem->getBody(settings.mCenter);
    if (parent) {
      height = parent->getHeight(lngLat);
    }
    auto radii = mSolarSystem->getRadii(settings.mCenter);
    item.mAnchor->setAnchorPosition(
        cs::utils::convert::toCartesian(lngLat, radii[0], radii[0], height));

    mSolarSystem->registerAnchor(item.mAnchor);

    item.mGuiArea = std::make_unique<cs::gui::WorldSpaceGuiArea>(settings.mWidth, settings.mHeight);
    item.mGuiItem = std::make_unique<cs::gui::GuiItem>(
        "file://../share/resources/gui/custom-web-ui-simple.html");

    item.mTransform = pSG->NewTransformNode(item.mAnchor.get());
    item.mTransform->Scale(
        0.001F * item.mGuiArea->getWidth(), 0.001F * item.mGuiArea->getHeight(), 1.F);
    item.mTransform->Rotate(
        VistaAxisAndAngle(VistaVector3D(0.0, 1.0, 0.0), -glm::pi<float>() / 2.F));
    item.mGuiArea->addItem(item.mGuiItem.get());
    item.mGuiArea->setUseLinearDepthBuffer(true);
    item.mGuiNode = pSG->NewOpenGLNode(item.mTransform, item.mGuiArea.get());

    mInputManager->registerSelectable(item.mGuiNode);

    VistaOpenSGMaterialTools::SetSortKeyOnSubtree(
        item.mGuiNode, static_cast<int>(cs::utils::DrawOrder::eTransparentItems));

    item.mGuiItem->setCursorChangeCallback(
        [](cs::gui::Cursor c) { cs::core::GuiManager::setCursor(c); });
    item.mGuiItem->waitForFinishedLoading();
    item.mGuiItem->callJavascript("set_content", settings.mHTML);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void Plugin::update() {
  double simulationTime(mTimeControl->pSimulationTime.get());

  for (auto& item : mSpaceItems) {
    cs::core::SolarSystem::scaleRelativeToObserver(*item.mAnchor, mSolarSystem->getObserver(),
        simulationTime, item.mScale, mAllSettings->mGraphics.pWidgetScale.get());
    cs::core::SolarSystem::turnToObserver(
        *item.mAnchor, mSolarSystem->getObserver(), simulationTime, false);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void Plugin::deInit() {
}

////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace csp::customwebui
