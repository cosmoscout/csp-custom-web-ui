////////////////////////////////////////////////////////////////////////////////////////////////////
//                               This file is part of CosmoScout VR                               //
//      and may be used under the terms of the MIT license. See the LICENSE file for details.     //
//                        Copyright: (c) 2019 German Aerospace Center (DLR)                       //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef CSP_CUSTOM_WEB_UI_PLUGIN_HPP
#define CSP_CUSTOM_WEB_UI_PLUGIN_HPP

#include "../../../src/cs-core/PluginBase.hpp"

#include <string>
#include <vector>

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

      /// Tha actual HTML code to add. You can use an <iframe> for example.
      std::string mHTML;
    };

    std::vector<SideBarItem> mSideBarItems;
  };

  void init() override;
  void deInit() override;

 private:
  Settings mPluginSettings;
};

} // namespace csp::customwebui

#endif // CSP_CUSTOM_WEB_UI_PLUGIN_HPP
