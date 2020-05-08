////////////////////////////////////////////////////////////////////////////////////////////////////
//                               This file is part of CosmoScout VR                               //
//      and may be used under the terms of the MIT license. See the LICENSE file for details.     //
//                        Copyright: (c) 2019 German Aerospace Center (DLR)                       //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef CSP_CUSTOM_WEB_UI_PLUGIN_HPP
#define CSP_CUSTOM_WEB_UI_PLUGIN_HPP

#include "../../../src/cs-core/PluginBase.hpp"

#include <list>
#include <string>
#include <vector>

class VistaOpenGLNode;
class VistaTransformNode;

namespace cs::scene {
class CelestialAnchorNode;
} // namespace cs::scene

namespace cs::gui {
class WorldSpaceGuiArea;
class GuiItem;
} // namespace cs::gui

namespace csp::customwebui {

/// This plugin allows to add custom HTML content to the sidebar or to any position in space.
class Plugin : public cs::core::PluginBase {
 public:
  struct Settings {

    /// These items will be added to the sidebar.
    struct SideBarItem {

      /// The name of the sidebar tab.
      std::string mName;

      /// Material icon, see https://material.io/resources/icons for options.
      std::string mIcon;

      /// The actual HTML code to add. You can use an <iframe> for example.
      std::string mHTML;

      bool operator==(SideBarItem const& other) const;
    };

    /// These items will be placed somewhere on a celestial body.
    struct SpaceItem {

      /// The SPICE center and frame names.
      std::string mCenter;
      std::string mFrame;

      /// The position of the item, elevation is relative to the surface height.
      double mLongitude;
      double mLatitude;
      double mElevation;

      /// Size of the item. The item will scale based on the observer distance.
      double mScale;

      /// Size of the item in pixels.
      uint32_t mWidth;
      uint32_t mHeight;

      /// The actual HTML code to add. You can use an <iframe> for example.
      std::string mHTML;

      bool operator==(SpaceItem const& other) const;
    };

    std::vector<SideBarItem> mSideBarItems;
    std::vector<SpaceItem>   mSpaceItems;

    bool operator!=(Settings const& other) const;
    bool operator==(Settings const& other) const;
  };

  void init() override;
  void update() override;
  void deInit() override;

 private:
  void onLoad();
  void unload(Settings const& pluginSettings);

  struct SpaceItem {
    std::unique_ptr<cs::gui::WorldSpaceGuiArea>     mGuiArea;
    std::unique_ptr<cs::gui::GuiItem>               mGuiItem;
    std::shared_ptr<cs::scene::CelestialAnchorNode> mAnchor;
    std::unique_ptr<VistaTransformNode>             mTransform;
    std::unique_ptr<VistaOpenGLNode>                mGuiNode;
    double                                          mScale = 1.0;
  };

  Settings mPluginSettings;

  std::list<SpaceItem> mSpaceItems;

  int mOnLoadConnection = -1;
  int mOnSaveConnection = -1;
};

} // namespace csp::customwebui

#endif // CSP_CUSTOM_WEB_UI_PLUGIN_HPP
