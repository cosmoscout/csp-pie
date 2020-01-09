////////////////////////////////////////////////////////////////////////////////////////////////////
//                               This file is part of CosmoScout VR                               //
//      and may be used under the terms of the MIT license. See the LICENSE file for details.     //
//                        Copyright: (c) 2019 German Aerospace Center (DLR)                       //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Plugin.hpp"

#include "../../../src/cs-core/GuiManager.hpp"
#include "../../../src/cs-core/SolarSystem.hpp"
#include "../../../src/cs-gui/GuiItem.hpp"

#include <VistaKernel/GraphicsManager/VistaTransformNode.h>
#include <VistaKernelOpenSGExt/VistaOpenSGMaterialTools.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

EXPORT_FN cs::core::PluginBase* create() {
  return new csp::pie::Plugin;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

EXPORT_FN void destroy(cs::core::PluginBase* pluginBase) {
  delete pluginBase;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace csp::pie {

////////////////////////////////////////////////////////////////////////////////////////////////////

void from_json(const nlohmann::json& j, Plugin::Settings& o) {
  cs::core::parseSection("csp-pie", [&] {
    o.mConfig    = j.find("config")->dump();
    o.mStructure = j.find("structure")->dump();
  });
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void Plugin::init() {
  std::cout << "Loading: CosmoScout VR Plugin Pie Menu" << std::endl;

  mPluginSettings = mAllSettings->mPlugins.at("csp-pie");

  mGuiManager->addScriptToGuiFromJS("../share/resources/gui/third-party/js/lodash.js");
  mGuiManager->addScriptToGuiFromJS("../share/resources/gui/third-party/js/paper-full.js");
  mGuiManager->addScriptToGuiFromJS("../share/resources/gui/third-party/js/tasty.js");
  mGuiManager->addScriptToGuiFromJS("../share/resources/gui/js/csp-pie.js");

  mGuiManager->getGui()->callJavascript("CosmoScout.registerCss", "css/csp-pie.css");

  mGuiManager->getGui()->callJavascript(
      "CosmoScout.pie.initMenu", mPluginSettings.mConfig, "#cosmoscout");

  mInputManager->pCurrentKey.onChange().connect([this](int keyCode) {
      if (abs(keyCode) != 32) {
          return;
      }

      bool pressed = keyCode == 32;

      mGuiManager->getGui()->callJavascript("CosmoScout.pie.enabled", pressed, mInputManager->mMousePosition.x, mInputManager->mMousePosition.y);
  });

  mGuiManager->getGui()->registerCallback<std::string>("pie_item_selected", [this](std::string target) {
     std::cout << "[CSP-PIE]::SELECTION " << target << "\n";
  });
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void Plugin::update() {
  if (!mLoaded) {
    mGuiManager->getGui()->callJavascript(
        "CosmoScout.pie.setStructure", mPluginSettings.mStructure);
    mLoaded = true;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void Plugin::deInit() {
  mGuiManager->getGui()->callJavascript("CosmoScout.unregisterCss", "css/csp-pie.css");
}

////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace csp::pie
