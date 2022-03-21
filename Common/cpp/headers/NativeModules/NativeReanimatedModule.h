#pragma once

#include <react/renderer/uimanager/UIManager.h>

#include <unistd.h>
#include <memory>
#include <string>
#include <vector>

#include "AnimatedSensorModule.h"
#include "ErrorHandler.h"
#include "LayoutAnimationsProxy.h"
#include "NativeReanimatedModuleSpec.h"
#include "PlatformDepMethodsHolder.h"
#include "RuntimeDecorator.h"
#include "RuntimeManager.h"
#include "Scheduler.h"

namespace reanimated {

using FrameCallback = std::function<void(double)>;

class ShareableValue;
class MutableValue;
class MapperRegistry;
class EventHandlerRegistry;

class NativeReanimatedModule : public NativeReanimatedModuleSpec,
                               public RuntimeManager {
  friend ShareableValue;
  friend MutableValue;

 public:
  NativeReanimatedModule(
      std::shared_ptr<CallInvoker> jsInvoker,
      std::shared_ptr<Scheduler> scheduler,
      std::shared_ptr<jsi::Runtime> rt,
      std::shared_ptr<ErrorHandler> errorHandler,
      std::function<jsi::Value(jsi::Runtime &, const int, const jsi::String &)>
          propObtainer,
      std::shared_ptr<LayoutAnimationsProxy> layoutAnimationsProxy,
      PlatformDepMethodsHolder platformDepMethodsHolder);

  void installCoreFunctions(jsi::Runtime &rt, const jsi::Value &valueSetter)
      override;

  jsi::Value makeShareable(jsi::Runtime &rt, const jsi::Value &value) override;
  jsi::Value makeMutable(jsi::Runtime &rt, const jsi::Value &value) override;
  jsi::Value makeRemote(jsi::Runtime &rt, const jsi::Value &value) override;

  jsi::Value startMapper(
      jsi::Runtime &rt,
      const jsi::Value &worklet,
      const jsi::Value &inputs,
      const jsi::Value &outputs,
      const jsi::Value &updater,
      const jsi::Value &viewDescriptors) override;
  void stopMapper(jsi::Runtime &rt, const jsi::Value &mapperId) override;

  jsi::Value registerEventHandler(
      jsi::Runtime &rt,
      const jsi::Value &eventHash,
      const jsi::Value &worklet) override;
  void unregisterEventHandler(
      jsi::Runtime &rt,
      const jsi::Value &registrationId) override;

  jsi::Value getViewProp(
      jsi::Runtime &rt,
      const jsi::Value &viewTag,
      const jsi::Value &propName,
      const jsi::Value &callback) override;

  jsi::Value enableLayoutAnimations(jsi::Runtime &rt, const jsi::Value &config)
      override;
  jsi::Value configureProps(
      jsi::Runtime &rt,
      const jsi::Value &uiProps,
      const jsi::Value &nativeProps) override;

  jsi::Value initializeForFabric(jsi::Runtime &rt) override;

  void onRender(double timestampMs);
  void onEvent(std::string eventName, jsi::Value &&eventAsString);
  bool isAnyHandlerWaitingForEvent(std::string eventName);

  void maybeRequestRender();

  void updateProps(
      jsi::Runtime &rt,
      const jsi::Value &shadowNodeValue,
      const jsi::Value &props);
  void performOperations();

  std::shared_ptr<UIManager> getUIManager() const {
    react_native_assert(
        uiManager_ !=
        nullptr); // make sure you have called initializeForFabric() before
    return uiManager_;
  }

  jsi::Value registerSensor(
      jsi::Runtime &rt,
      const jsi::Value &sensorType,
      const jsi::Value &interval,
      const jsi::Value &sensorDataContainer) override;
  void unregisterSensor(jsi::Runtime &rt, const jsi::Value &sensorId) override;

 private:
  std::shared_ptr<MapperRegistry> mapperRegistry;
  std::shared_ptr<EventHandlerRegistry> eventHandlerRegistry;
  std::function<void(FrameCallback &, jsi::Runtime &)> requestRender;
  std::shared_ptr<jsi::Value> dummyEvent;
  std::vector<FrameCallback> frameCallbacks;
  bool renderRequested = false;
  std::function<jsi::Value(jsi::Runtime &, const int, const jsi::String &)>
      propObtainer;
  std::function<void(double)> onRenderCallback;
  SynchronouslyUpdateUIPropsFunction synchronouslyUpdateUIPropsFunction;
  std::shared_ptr<LayoutAnimationsProxy> layoutAnimationsProxy;
  AnimatedSensorModule animatedSensorModule;
  ConfigurePropsFunction configurePropsPlatformFunction;

  std::shared_ptr<UIManager> uiManager_;
  std::vector<std::pair<ShadowNode::Shared, std::unique_ptr<RawProps>>>
      operationsInBatch_; // TODO: refactor std::pair to custom struct

  std::unordered_set<std::string> nativePropNames_; // filled by configureProps
};

} // namespace reanimated
