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

void from_json(const nlohmann::json& j, Plugin::Settings::SideBarItem& o) {
  o.mName = cs::core::parseProperty<std::string>("name", j);
  o.mIcon = cs::core::parseProperty<std::string>("icon", j);
  o.mHTML = cs::core::parseProperty<std::string>("html", j);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void from_json(const nlohmann::json& j, Plugin::Settings::SpaceItem& o) {
  o.mCenter    = cs::core::parseProperty<std::string>("center", j);
  o.mFrame     = cs::core::parseProperty<std::string>("icon", j);
  o.mLongitude = cs::core::parseProperty<double>("longitude", j);
  o.mLatitude  = cs::core::parseProperty<double>("latitude", j);
  o.mElevation = cs::core::parseProperty<double>("elevation", j);
  o.mScale     = cs::core::parseProperty<double>("scale", j);
  o.mWidth     = cs::core::parseProperty<uint32_t>("width", j);
  o.mHeight    = cs::core::parseProperty<uint32_t>("height", j);
  o.mHTML      = cs::core::parseProperty<std::string>("html", j);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void from_json(const nlohmann::json& j, Plugin::Settings& o) {
  cs::core::parseSection("csp-custom-web-ui", [&] {
    auto iter = j.find("sidebar-items");
    if (iter != j.end()) {
      o.mSideBarItems = cs::core::parseVector<Plugin::Settings::SideBarItem>("sidebar-items", j);
    }

    iter = j.find("space-items");
    if (iter != j.end()) {
      o.mSpaceItems = cs::core::parseVector<Plugin::Settings::SpaceItem>("space-items", j);
    }
  });
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

    item.mGuiArea.reset(new cs::gui::WorldSpaceGuiArea(settings.mWidth, settings.mHeight));
    item.mGuiItem.reset(
        new cs::gui::GuiItem("file://../share/resources/gui/custom-web-ui-simple.html"));

    item.mTransform = pSG->NewTransformNode(item.mAnchor.get());
    item.mTransform->Scale(
        0.001f * item.mGuiArea->getWidth(), 0.001f * item.mGuiArea->getHeight(), 1.f);
    item.mTransform->Rotate(
        VistaAxisAndAngle(VistaVector3D(0.0, 1.0, 0.0), -glm::pi<float>() / 2.f));
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
        simulationTime, item.mScale, mGraphicsEngine->pWidgetScale.get());
    cs::core::SolarSystem::turnToObserver(
        *item.mAnchor, mSolarSystem->getObserver(), simulationTime, false);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void Plugin::deInit() {
}

////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace csp::customwebui
