
#pragma once

#include "application.h"
#include "input/image/image_input.h"
#include "input/request/request.h"
#include "ocr/ocr.h"
#include "output/notify/event_notify.h"
#include "output/operation/device_operation.h"
#include "player/player.h"
#include "yaml-cpp/yaml.h"
#include <chrono>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <string>

class CommonApplication : public IApplication {
private:
  enum ApplicationStatus { Stopped, Running, Pausing };

public:
  CommonApplication(const std::string &config_fname);

  bool Init() override;

  void Start() override;

  void Pause() override;

  void Continue() override;

  void Stop() override;

  bool SetPlayer(const std::string &player_name) override;

  std::string GetPlayer() override;

  bool SetMode(const std::string &mode_name) override;

  std::string GetMode() override;

private:
  bool InitWithYaml(const YAML::Node &yaml);

  IPlayer *CreatePlayer(const std::string &config_path);

  bool QueryCurrentPlayerCallback(const std::string &request_str,
                                  std::string &response_str);

  bool ReplaceCurrentPlayerCallback(const std::string &request_str,
                                    std::string &response_str);

  bool QueryCurrentModeCallback(const std::string &request_str,
                                std::string &response_str);

  bool ReplaceCurrentModeCallback(const std::string &request_str,
                                  std::string &response_str);

  bool QueryStatusCallback(const std::string &request_str,
                           std::string &response_str);

  bool ReplaceStatusCallback(const std::string &request_str,
                             std::string &response_str);

private:
  std::string config_abs_path_;
  std::shared_ptr<IImageInput> source_;
  std::shared_ptr<IOcrDetect> ocr_;
  std::vector<std::shared_ptr<IPlayer>> players_;
  std::shared_ptr<ITouchScreenOperation> operation_;
  std::shared_ptr<IRequest> request_;
  std::shared_ptr<IEventNotify> notify_;

  ApplicationStatus status_;
  std::mutex mutex_;
  std::condition_variable con_;

  std::shared_ptr<IPlayer> player_;

  int interval_ms_;
};
