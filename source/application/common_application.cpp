
#include "common_application.h"
#include "common/log.h"
#include "detect/detect.h"
#include "input/image/file_input.h"
#include "input/image/minicap_input.h"
#include "input/image/scrcpy_input.h"
#include "input/request/http_request.h"
#include "ocr/paddle_ocr.h"
#include "output/notify/miao_notify.h"
#include "output/operation/dummy_operation.h"
#include "output/operation/minitouch_operation.h"
#include "output/operation/scrcpy_operation.h"
#include "parser/yaml_parser.h"
#include "player/common_player.h"
#include "utils/util_functions.h"
#include <filesystem>
#include <opencv2/imgcodecs.hpp>
#include <yaml-cpp/yaml.h>

CommonApplication::CommonApplication(const std::string &config_fname) {
  status_ = ApplicationStatus::Stopped;
  config_abs_path_ = std::filesystem::absolute(config_fname).string();
}

bool CommonApplication::Init() {
  YAML::Node config_yaml(YAML::LoadFile(config_abs_path_));
  return InitWithYaml(config_yaml);
}

void CommonApplication::Start() {
  LOG(INFO) << "Application running";
  status_ = ApplicationStatus::Running;

  while (true) {
    // Check application status
    std::unique_lock<std::mutex> lock(mutex_);
    con_.wait(lock, [this] {
      return status_ != ApplicationStatus::Pausing;
    });
    if (status_ == ApplicationStatus::Stopped) {
      LOG(INFO) << "Application stoped";
      break;
    }
    lock.unlock();

    // Get one screenshot
    TimeLog image_time_log("Image source");
    cv::Mat image = source_->GetOneFrame();
    if (image.empty()) {
      LOG(WARNING) << "Image is empty";
      continue;
    }
    image_time_log.Tok();

    // Ocr detect
    TimeLog ocr_time_log("OCR");
    std::vector<TextBox> text_boxes = ocr_->Detect(image);
    if (text_boxes.empty()) {
      DLOG(INFO) << "Text is empty";
      continue;
    }
    ocr_time_log.Tok();

    // Player
    const auto get_page_element_with_ocr_and_detect =
        [width = image.cols,
         height = image.rows](const std::vector<TextBox> &text_boxes,
                              const std::vector<ObjectBox> &object_boxes) {
          std::vector<Element> ret;
          std::transform(
              text_boxes.begin(), text_boxes.end(), std::back_inserter(ret),
              [width, height](const TextBox &text_box) {
                return Element(text_box.text,
                               static_cast<float>(text_box.region.x) / width,
                               static_cast<float>(text_box.region.y) / height);
              });
          std::transform(
              object_boxes.begin(), object_boxes.end(), std::back_inserter(ret),
              [width, height](const ObjectBox &object_box) {
                return Element(object_box.class_name,
                               static_cast<float>(object_box.region.x) / width,
                               static_cast<float>(object_box.region.y) /
                                   height);
              });
          return ret;
        };
    std::vector<PlayOperation> play_operations =
        player_->Play(get_page_element_with_ocr_and_detect(text_boxes, {}));
    TimeLog operation_time_log("Operation");
    const auto get_point_with_scale =
        [width = image.cols,
         height = image.rows](const ClickOperation &operation) {
          return std::make_pair(width * operation.x, height * operation.y);
        };
    for (const PlayOperation &play_operation : play_operations) {
      switch (play_operation.type) {
      case PlayOperationType::SCREEN_CLICK: {
        std::pair<float, float> point =
            get_point_with_scale(play_operation.click);
        operation_->Click(point.first, point.second);
        break;
      }
      case PlayOperationType::SLEEP: {
        std::this_thread::sleep_for(
            std::chrono::milliseconds(play_operation.sleep_ms));
        break;
      }
      case PlayOperationType::OVER: {
        LOG(INFO) << "Application is over";
        status_ = ApplicationStatus::Pausing;
        if (notify_) {
          LOG(INFO) << "Notify mode over: " << player_->GetMode();
          if (!notify_->Notify("Mode " + player_->GetMode() + " is over")) {
            LOG(ERROR) << "Notify failed.";
          }
        }
        break;
      }
      default: {
        LOG(WARNING) << "Unknown operation. "
                     << static_cast<int>(play_operation.type);
        break;
      }
      }
    }
    operation_time_log.Tok();

    if (interval_ms_ > 0) {
      std::this_thread::sleep_for(std::chrono::milliseconds(interval_ms_));
    }
  }
}

void CommonApplication::Pause() {
  LOG(INFO) << "Application pause";
  std::lock_guard<std::mutex> lock(mutex_);
  status_ = ApplicationStatus::Pausing;
}

void CommonApplication::Continue() {
  LOG(INFO) << "Application continue";
  std::lock_guard<std::mutex> lock(mutex_);
  status_ = ApplicationStatus::Running;
  con_.notify_one();
}

void CommonApplication::Stop() {
  LOG(INFO) << "Application stop";
  std::lock_guard<std::mutex> lock(mutex_);
  status_ = ApplicationStatus::Stopped;
  con_.notify_one();
}

bool CommonApplication::SetPlayer(const std::string &player_name) {
  std::lock_guard<std::mutex> lock(mutex_);
  for (const auto &player : players_) {
    if (player->Name() == player_name) {
      player_ = player;
      return true;
    }
  }
  LOG(ERROR) << "Unknown player " << player_name;
  return false;
}

std::string CommonApplication::GetPlayer() {
  std::lock_guard<std::mutex> lock(mutex_);
  return player_->Name();
}

bool CommonApplication::SetMode(const std::string &mode_name) {
  std::lock_guard<std::mutex> lock(mutex_);
  return player_->SetMode(mode_name);
}

std::string CommonApplication::GetMode() {
  std::lock_guard<std::mutex> lock(mutex_);
  return player_->GetMode();
}

bool CommonApplication::InitWithYaml(const YAML::Node &yaml) {
  // Image source
  const YAML::Node &source_yaml(yaml["source"]);
  std::string source_type = source_yaml["type"].as<std::string>();
  if (source_type == "minicap") {
    auto server_info(GetServerInfo(source_yaml));
    source_.reset(
        new MinicapInput(std::get<0>(server_info), std::get<1>(server_info)));
  } else if (source_type == "file") {
    source_.reset(
        new FileImageInput(source_yaml["file_name"].as<std::string>()));
  } else if (source_type == "scrcpy") {
    auto server_info(GetServerInfo(source_yaml));
    source_.reset(
        new ScrcpyInput(std::get<0>(server_info), std::get<1>(server_info)));
  }
  if (!source_ || !source_->Init()) {
    LOG(ERROR) << "source init failed";
    return false;
  }
  LOG(INFO) << "Image source init success";

  // Ocr
  const YAML::Node &ocr_yaml(yaml["ocr"]);
  if (ocr_yaml["type"].as<std::string>() == "paddleocr") {
    auto server_info = GetServerInfo(ocr_yaml);
    if (ocr_yaml["recv_timeout"].IsDefined()) {
      ocr_.reset(new PaddleOcr(std::get<0>(server_info),
                               std::get<1>(server_info),
                               ocr_yaml["recv_timeout"].as<int>()));
    } else {
      ocr_.reset(
          new PaddleOcr(std::get<0>(server_info), std::get<1>(server_info)));
    }
  }
  if (!ocr_ || !ocr_->Init()) {
    LOG(ERROR) << "ocr init failed";
    return false;
  }
  LOG(INFO) << "ocr init success";

  // Player
  const YAML::Node &players_yaml(yaml["players"]);
  for (const auto &player_yaml : players_yaml) {
    std::filesystem::path player_config_path =
        std::filesystem::path(config_abs_path_).parent_path();
    player_config_path /= player_yaml["include"].as<std::string>();

    std::shared_ptr<IPlayer> player(CreatePlayer(player_config_path.string()));
    if (!player || !player->Init()) {
      LOG(ERROR) << "Player init failed. " << player->Name();
      continue;
    }
    LOG(INFO) << "Player init success. " << player->Name();
    players_.push_back(player);
  }
  if (!players_.empty()) {
    player_ = players_[0];
  } else {
    LOG(ERROR) << "Players is empty";
  }

  // Operation
  const YAML::Node &operation_yaml = yaml["operation"];
  std::string operation_type = operation_yaml["type"].as<std::string>();
  if (operation_type == "minitouch") {
    cv::Mat image = source_->GetOneFrame();
    if (image.empty()) {
      LOG(ERROR) << "Image get frame failed";
      return false;
    }
    auto server_info = GetServerInfo(operation_yaml);
    int orientation = image.cols > image.rows ? 90 : 0;
    operation_.reset(new MinitouchOperation(
        std::get<0>(server_info), std::get<1>(server_info), image.cols,
        image.rows, orientation));
  } else if (operation_type == "dummy") {
    operation_.reset(new DummyOperation());
  } else if (operation_type == "scrcpy") {
    auto server_info = GetServerInfo(operation_yaml);
    operation_.reset(new ScrcpyOperation(std::get<0>(server_info),
                                         std::get<1>(server_info)));
  }
  if (!operation_ || !operation_->Init()) {
    LOG(ERROR) << "Operation init failed";
    return false;
  }
  LOG(INFO) << "Operation init success";

  // Request
  const YAML::Node &request_yaml = yaml["request"];
  if (request_yaml.IsDefined()) {
    std::string request_type = request_yaml["type"].as<std::string>();
    if (request_type == "http") {
      auto server_info = GetServerInfo(request_yaml);
      request_.reset(
          new HttpRequest(std::get<0>(server_info), std::get<1>(server_info)));
    }
    if (!request_ || !request_->Init()) {
      request_.reset();
      LOG(ERROR) << "Request init failed";
    } else {
      request_->SetCallback(
          "/player/name", RequestOperation::Query,
          std::bind(&CommonApplication::QueryCurrentPlayerCallback, this,
                    std::placeholders::_1, std::placeholders::_2));

      request_->SetCallback(
          "/player/name", RequestOperation::Replace,
          std::bind(&CommonApplication::ReplaceCurrentPlayerCallback, this,
                    std::placeholders::_1, std::placeholders::_2));

      request_->SetCallback(
          "/player/mode/name", RequestOperation::Query,
          std::bind(&CommonApplication::QueryCurrentModeCallback, this,
                    std::placeholders::_1, std::placeholders::_2));

      request_->SetCallback(
          "/player/mode/name", RequestOperation::Replace,
          std::bind(&CommonApplication::ReplaceCurrentModeCallback, this,
                    std::placeholders::_1, std::placeholders::_2));

      request_->SetCallback("/status", RequestOperation::Query,
                            std::bind(&CommonApplication::QueryStatusCallback,
                                      this, std::placeholders::_1,
                                      std::placeholders::_2));

      request_->SetCallback("/status", RequestOperation::Replace,
                            std::bind(&CommonApplication::ReplaceStatusCallback,
                                      this, std::placeholders::_1,
                                      std::placeholders::_2));
    }
  } else {
    LOG(INFO) << "Request is not defined";
  }

  // Notify
  const YAML::Node &notify_yaml = yaml["notify"];
  if (notify_yaml.IsDefined()) {
    std::string notify_type = notify_yaml["type"].as<std::string>();
    if (notify_type == "miao") {
      notify_.reset(new MiaoNotify(notify_yaml["miao_key"].as<std::string>()));
    }
    if (!notify_ || !notify_->Init()) {
      notify_.reset();
      LOG(ERROR) << "Notify init failed";
    }
  } else {
    LOG(INFO) << "Notify is not defined";
  }

  // Application
  const YAML::Node &app_yaml = yaml["application"];
  if (app_yaml.IsDefined()) {
    interval_ms_ =
        app_yaml["interval"].IsDefined() ? app_yaml["interval"].as<int>() : 0;
  }

  return true;
}

IPlayer *CommonApplication::CreatePlayer(const std::string &config_path) {
  LOG(INFO) << "Load player " << config_path;
  IPlayer *ret = nullptr;
  YAML::Node player_yaml = YAML::LoadFile(config_path);
  std::string player_name = player_yaml["name"].as<std::string>();
  if (player_yaml["type"].as<std::string>() == "common") {
    std::vector<PageConfig> page_configs;
    for (const auto &page_yaml : player_yaml["pages"]) {
      PageConfig page_config;
      page_config.name = page_yaml["name"].as<std::string>();

      for (auto &page_condition_yaml : page_yaml["conditions"]) {
        PageKeyElement key_element = GetKeyElementConfig(page_condition_yaml);
        page_config.key_elements.push_back(key_element);
      }
      page_configs.push_back(page_config);
    }

    std::vector<ModeConfig> mode_configs;
    for (const auto &mode_yaml : player_yaml["modes"]) {
      ModeConfig mode_config = GetModeConfig(mode_yaml, mode_configs);
      mode_configs.push_back(mode_config);
    }
    ret = new CommonPlayer(player_name, page_configs, mode_configs);
  }
  return ret;
}

bool CommonApplication::QueryCurrentPlayerCallback(
    const std::string &request_str, std::string &response_str) {
  response_str = player_->Name();
  LOG(INFO) << "Get current player " << response_str;
  return true;
}

bool CommonApplication::ReplaceCurrentPlayerCallback(
    const std::string &request_str, std::string &response_str) {
  LOG(INFO) << "Application set player " << request_str;
  if (!SetPlayer(request_str)) {
    LOG(ERROR) << "Set player failed";
    return false;
  }
  return true;
}

bool CommonApplication::QueryCurrentModeCallback(const std::string &request_str,
                                                 std::string &response_str) {
  response_str = player_->GetMode();
  LOG(INFO) << "Get current mode " << response_str;
  return true;
}

bool CommonApplication::ReplaceCurrentModeCallback(
    const std::string &request_str, std::string &response_str) {
  LOG(INFO) << "Application set mode " << request_str;
  if (!player_->SetMode(request_str)) {
    LOG(ERROR) << "Set mode failed";
    return false;
  }
  return true;
}

bool CommonApplication::QueryStatusCallback(const std::string &request_str,
                                            std::string &response_str) {
  switch (status_) {
  case ApplicationStatus::Stopped:
    response_str = "stop";
    break;
  case ApplicationStatus::Pausing:
    response_str = "pause";
    break;
  case CommonApplication::Running:
    response_str = "running";
    break;
  default:
    LOG(ERROR) << "Status unkown status";
    response_str = "";
    break;
  }
  LOG(INFO) << "Get status " << response_str;
  return !response_str.empty();
}

bool CommonApplication::ReplaceStatusCallback(const std::string &request_str,
                                              std::string &response_str) {
  LOG(INFO) << "Replace status " << request_str;
  if (request_str == "running") {
    Continue();
  } else if (request_str == "pause") {
    Pause();
  } else if (request_str == "stop") {
    Stop();
  } else {
    LOG(ERROR) << "Unkown status changed. " << request_str;
    return false;
  }
  return true;
}
