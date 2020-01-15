////////////////////////////////////////////////////////////////////////////////////////////////////
//                               This file is part of CosmoScout VR                               //
//      and may be used under the terms of the MIT license. See the LICENSE file for details.     //
//                        Copyright: (c) 2019 German Aerospace Center (DLR)                       //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef CSP_PIE_PLUGIN_HPP
#define CSP_PIE_PLUGIN_HPP

#include "../../../src/cs-core/InputManager.hpp"
#include "../../../src/cs-core/PluginBase.hpp"
#include "../../../src/cs-core/Settings.hpp"
#include "../../../src/cs-gui/WorldSpaceGuiArea.hpp"

class VistaOpenGLNode;
class VistaViewportResizeToProjectionAdapter;
class VistaTransformNode;

namespace csp::pie {

/// Pie Menu interactions
class Plugin : public cs::core::PluginBase {
 public:
  struct Settings {
    std::string mDisplay;
    std::string mConfig;
    std::string mStructure;
  };

  void init() override;
  void deInit() override;
  void update() override;

 private:
  VistaTransformNode*         mGuiTransform = nullptr;
  VistaOpenGLNode*            mGuiNode      = nullptr;
  cs::gui::WorldSpaceGuiArea* mGuiArea      = nullptr;
  cs::gui::GuiItem*           mGuiItem      = nullptr;
  Settings                    mPluginSettings;
  bool                        mLoaded = false;

  /// Creates a WorldSpaceGuiArea and inserts the pie menu
  void initGuiGlobal();

  /// Inserts the pie menu into the global cosmoscout gui
  void initGuiLocal();
  void addGuiCallbacks();
};

} // namespace csp::pie

#endif // CSP_PIE_PLUGIN_HPP
