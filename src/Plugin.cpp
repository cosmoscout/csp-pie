////////////////////////////////////////////////////////////////////////////////////////////////////
//                               This file is part of CosmoScout VR                               //
//      and may be used under the terms of the MIT license. See the LICENSE file for details.     //
//                        Copyright: (c) 2019 German Aerospace Center (DLR)                       //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Plugin.hpp"

#include "../../../src/cs-core/GuiManager.hpp"
#include "../../../src/cs-core/MessageBus.hpp"
#include <VistaKernel/DisplayManager/GlutWindowImp/VistaGlutWindowingToolkit.h>
#include <VistaKernel/DisplayManager/VistaDisplayManager.h>
#include <VistaKernel/DisplayManager/VistaDisplaySystem.h>
#include <VistaKernel/DisplayManager/VistaViewport.h>
#include <VistaKernel/DisplayManager/VistaViewportResizeToProjectionAdapter.h>

#include <VistaKernel/DisplayManager/VistaProjection.h>
#include <VistaKernel/DisplayManager/VistaVirtualPlatform.h>
#include <VistaKernel/EventManager/VistaEventManager.h>
#include <VistaKernel/GraphicsManager/VistaGeomNode.h>
#include <VistaKernel/GraphicsManager/VistaGraphicsManager.h>
#include <VistaKernel/GraphicsManager/VistaOpenGLNode.h>
#include <VistaKernel/GraphicsManager/VistaSceneGraph.h>
#include <VistaKernel/GraphicsManager/VistaTransformNode.h>
#include <VistaKernel/InteractionManager/VistaInteractionEvent.h>
#include <VistaKernel/InteractionManager/VistaInteractionManager.h>
#include <VistaKernel/InteractionManager/VistaUserPlatform.h>
#include <VistaKernel/Stuff/VistaFramerateDisplay.h>
#include <VistaKernel/VistaFrameLoop.h>
#include <VistaKernel/VistaSystem.h>
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
    o.mDisplay   = cs::core::parseProperty<std::string>("display", j);
    o.mConfig    = j.find("config")->dump();
    o.mStructure = j.find("structure")->dump();
  });
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void Plugin::init() {
  std::cout << "Loading: CosmoScout VR Plugin Pie Menu" << std::endl;

  mPluginSettings = mAllSettings->mPlugins.at("csp-pie");
  std::cout << mPluginSettings.mDisplay << "\n";

  if (mPluginSettings.mDisplay == "local") {
    initGuiLocal();
  } else {
    initGuiGlobal();
  }

  addGuiCallbacks();

  mGuiItem->callJavascript("CosmoScout.pie.initMenu", mPluginSettings.mConfig);

  mInputManager->pCurrentKey.onChange().connect([this](int keyCode) {
    if (abs(keyCode) != 32) {
      return;
    }

    VistaVector3D       v3Position;
    VistaQuaternion     qOrientation;
    VistaTransformNode* pIntentionNode = dynamic_cast<VistaTransformNode*>(
        GetVistaSystem()->GetGraphicsManager()->GetSceneGraph()->GetNode("SELECTION_NODE"));

    pIntentionNode->GetWorldPosition(v3Position);
    pIntentionNode->GetWorldOrientation(qOrientation);

    VistaViewport* pViewport(GetVistaSystem()->GetDisplayManager()->GetViewports().begin()->second);
    auto           pProbs = pViewport->GetProjection()->GetProjectionProperties();

    VistaVector3D v3Origin, v3Up, v3Normal;
    double        dLeft, dRight, dBottom, dTop;

    pProbs->GetProjPlaneExtents(dLeft, dRight, dBottom, dTop);
    pProbs->GetProjPlaneMidpoint(v3Origin[0], v3Origin[1], v3Origin[2]);
    pProbs->GetProjPlaneUp(v3Up[0], v3Up[1], v3Up[2]);
    pProbs->GetProjPlaneNormal(v3Normal[0], v3Normal[1], v3Normal[2]);

    auto platformTransform =
        GetVistaSystem()->GetDisplayManager()->GetDisplaySystem()->GetReferenceFrame();

    VistaVector3D start = v3Position;
    VistaVector3D end   = (v3Position + qOrientation.GetViewDir());

    start = platformTransform->TransformPositionToFrame(start);
    end   = platformTransform->TransformPositionToFrame(end);

    VistaVector3D direction(end - start);

    VistaRay   ray(start, direction);
    VistaPlane plane(v3Origin, v3Up.Cross(v3Normal), v3Up, v3Normal);

    VistaVector3D guiIntersection;

    int x = (int)((guiIntersection[0] - dLeft) / (dRight - dLeft) * mGuiItem->getWidth());
    int y =
        (int)((1.0 - (guiIntersection[1] - dBottom) / (dTop - dBottom)) * mGuiItem->getHeight());

    int mX;
    int mY;
    mGuiItem->calculateMousePosition(x, y, mX, mY);

    bool pressed = keyCode == 32;

    mGuiItem->callJavascript("CosmoScout.pie.enabled", pressed, mX, mY);
  });

  mMessageBus->onResponse().connect([this](cs::core::MessageBus::Response const& response) {
    if (response.mSender == "csp::atmospheres" &&
        (response.mType == cs::core::MessageBus::Response::Type::eInfo ||
            response.mRequestSender != "csp::pie")) {
      if (response.mName == "atmosphere_quality" || response.mName == "water_level") {
        mGuiItem->callJavascript("CosmoScout.pie.setSliderValue", response.mName, response.mData);
      } else {
        mGuiItem->callJavascript(
            "CosmoScout.pie.setCheckboxSelected", "set_enable_" + response.mName, response.mData);
      }
    }
  });
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void Plugin::update() {
  if (!mLoaded) {
    mGuiItem->callJavascript("CosmoScout.pie.setStructure", mPluginSettings.mStructure);
    mLoaded = true;

    std::string receiver = "csp::atmospheres";
    std::string sender   = "csp::pie";

    mMessageBus->send({cs::core::MessageBus::Request::Type::eGet, receiver, "water", "", sender});
    mMessageBus->send({cs::core::MessageBus::Request::Type::eGet, receiver, "clouds", "", sender});
    mMessageBus->send(
        {cs::core::MessageBus::Request::Type::eGet, receiver, "atmosphere", "", sender});
    mMessageBus->send(
        {cs::core::MessageBus::Request::Type::eGet, receiver, "light_shafts", "", sender});
    mMessageBus->send(
        {cs::core::MessageBus::Request::Type::eGet, receiver, "atmosphere_quality", "", sender});
    mMessageBus->send(
        {cs::core::MessageBus::Request::Type::eGet, receiver, "water_level", "", sender});
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void Plugin::addGuiCallbacks() {
  mGuiItem->registerCallback<std::string, bool>(
      "pie_item_toggled", [this](const std::string& target, const bool& value) {
        std::cout << "[CSP-PIE]::TOGGLE " << target.substr(11) << " - " << std::to_string(value)
                  << "\n";
        mMessageBus->send({cs::core::MessageBus::Request::Type::eSet, "csp::atmospheres",
            target.substr(11), std::to_string(value)});
      });

  mGuiItem->registerCallback<std::string, double>(
      "pie_slider_changed", [this](const std::string& target, const double& value) {
        std::cout << "[CSP-PIE]::SLIDER CHANGE " << target << " - " << std::to_string(value)
                  << "\n";

        mMessageBus->send({cs::core::MessageBus::Request::Type::eSet, "csp::atmospheres", target,
            std::to_string(value)});
      });

  mGuiItem->registerCallback<std::string>("pie_item_action", [this](const std::string& target) {
    std::cout << "[CSP-PIE]::SELECTION " << target << "\n";
  });
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void Plugin::initGuiGlobal() {
  auto pSG = GetVistaSystem()->GetGraphicsManager()->GetSceneGraph();

  auto platform = GetVistaSystem()
                      ->GetPlatformFor(GetVistaSystem()->GetDisplayManager()->GetDisplaySystem())
                      ->GetPlatformNode();

  mGuiArea = new cs::gui::WorldSpaceGuiArea(1500, 1500);
  mGuiArea->setUseLinearDepthBuffer(true);

  mGuiItem = new cs::gui::GuiItem("file://../share/resources/gui/csp-pie.html");

  mGuiArea->addItem(mGuiItem);
  mGuiItem->setSizeX(1500);
  mGuiItem->setSizeY(1500);
  mGuiItem->waitForFinishedLoading();

  mGuiTransform = pSG->NewTransformNode(platform);
  mGuiTransform->Translate(0, 0, -.8f);
  mGuiNode = pSG->NewOpenGLNode(mGuiTransform, mGuiArea);
  VistaOpenSGMaterialTools::SetSortKeyOnSubtree(
      mGuiTransform, static_cast<int>(cs::utils::DrawOrder::eGui));
  mInputManager->registerSelectable(mGuiNode);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void Plugin::initGuiLocal() {
  mGuiItem = mGuiManager->getGui();

  mGuiManager->addScriptToGuiFromJS("../share/resources/gui/third-party/js/lodash.js");
  mGuiManager->addScriptToGuiFromJS("../share/resources/gui/third-party/js/paper-full.js");
  mGuiManager->addScriptToGuiFromJS("../share/resources/gui/third-party/js/tasty.js");
  mGuiManager->addScriptToGuiFromJS("../share/resources/gui/js/csp-pie.js");

  mGuiItem->callJavascript("CosmoScout.registerCss", "css/csp-pie.css");

  mGuiItem->callJavascript("CosmoScout.pie.initMenu", mPluginSettings.mConfig, "#cosmoscout");
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void Plugin::deInit() {
  delete mGuiArea;

  mGuiItem->unregisterCallback("pie_item_toggled");
  mGuiItem->unregisterCallback("pie_slider_changed");
  mGuiItem->unregisterCallback("pie_item_action");

  if (mPluginSettings.mDisplay == "local") {
    mGuiItem->callJavascript("CosmoScout.unregisterCss", "css/csp-pie.css");
  } else {
    delete mGuiItem;
    mInputManager->unregisterSelectable(mGuiNode);
  }
}

} // namespace csp::pie
