
#include "common_player.h"
#include "common/log.h"
#include <cassert>

typedef std::pair<uint16_t, uint16_t> Point;
typedef std::array<float, 4> Region;

static bool CenterInRegion(const Region &region, const Point &point) {
  return point.first > region[0] && point.first < region[2] &&
         point.second > region[1] && point.second < region[3];
}

static bool
SatisfyPageCondition(const std::vector<Element> &elements,
                     const std::vector<PageKeyElement> &key_elements) {
  for (const PageKeyElement &key_element : key_elements) {
    for (const Element &element : elements) {
      std::string name = element.name;
      uint16_t x = element.x, y = element.y;
      if (std::regex_match(name, key_element.pattern) &&
          CenterInRegion({key_element.x_min, key_element.y_min,
                          key_element.x_max, key_element.y_max},
                         {x, y})) {
        break;
      } else {
        return false;
      }
    }
  }
  return true;
}

static std::tuple<std::string, int, int>
GetPatternPoint(const std::vector<Element> &elements,
                const std::regex &pattern) {
  for (const auto element : elements) {
    if (std::regex_match(element.name, pattern)) {
      return std::make_tuple(element.name, element.x, element.y);
    }
  }
  return std::make_tuple("", 0, 0);
}

CommonPlayer::CommonPlayer(const std::string &name,
                           const std::vector<PageConfig> &page_configs,
                           const std::vector<ModeConfig> &mode_configs)
    : IPlayer(name), page_configs_(page_configs), mode_configs_(mode_configs) {
  mode_ = nullptr;
}

CommonPlayer::~CommonPlayer() {}

bool CommonPlayer::Init() {
  if (mode_configs_.empty()) {
    LOG(ERROR) << "Mode is empty";
    return false;
  }
  mode_ = &mode_configs_.front();
  for (const ModeConfig &mode_config : mode_configs_) {
    for (const auto &pattern_action : mode_config.page_pattern_actions) {
      bool has_page = false;
      for (const PageConfig &page_config : page_configs_) {
        if (std::regex_match(page_config.name, std::get<0>(pattern_action))) {
          has_page = true;
          break;
        }
      }
      if (!has_page) {
        LOG(ERROR) << "Mode has unmatched page pattern. " << mode_config.name;
        return false;
      }
    }
  }
  return true;
}

std::vector<PlayOperation>
CommonPlayer::Play(const std::vector<Element> &elements) {
  std::lock_guard<std::mutex> lock(mode_mutex_);
  for (const PageConfig &page_config : page_configs_) {
    // Detect page
    if (!SatisfyPageCondition(elements, page_config.key_elements)) {
      continue;
    }
    DLOG(INFO) << "Detect page " << page_config.name;

    // Return page actions
    auto iter = mode_->page_pattern_actions.begin();
    for (; iter != mode_->page_pattern_actions.end(); ++iter) {
      if (std::regex_match(page_config.name, std::get<0>(*iter))) {
        break;
      }
    }
    std::vector<PlayOperation> ret;
    if (iter != mode_->page_pattern_actions.end()) {
      ret = CreatePlayOperation(elements, std::get<1>(*iter));
    } else {
      LOG(INFO) << "Other page operation";
      ret = CreatePlayOperation(elements, mode_->other_page_actions);
    }
    return ret;
  }

  // Page is not defined
  DLOG(INFO) << "Undefined page operation";
  return CreatePlayOperation(elements, mode_->undefined_page_actions);
}

bool CommonPlayer::SetMode(const std::string &mode) {
  std::lock_guard<std::mutex> lock(mode_mutex_);
  for (const auto &mode_config : mode_configs_) {
    if (mode_config.name == mode) {
      mode_ = &mode_config;
      LOG(INFO) << "Player mode " << mode_->name;
      return true;
    }
  }
  LOG(ERROR) << "Unsupport mode " << mode;
  return false;
}

std::string CommonPlayer::GetMode() {
  std::lock_guard<std::mutex> lock(mode_mutex_);
  return mode_->name;
}

std::vector<PlayOperation> CommonPlayer::CreatePlayOperation(
    const std::vector<Element> &elements,
    const std::vector<ActionConfig> &action_configs) {
  std::vector<PlayOperation> ret;
  for (const ActionConfig &action_config : action_configs) {
    if (action_config.type == "click") {
      PlayOperation operation(PlayOperationType::SCREEN_CLICK);
      if (action_config.pattern) {
        auto point_info =
            GetPatternPoint(elements, *action_config.pattern);
        std::string click_text = std::get<0>(point_info);
        if (click_text.empty()) {
          LOG(ERROR) << "Can not find click pattern";
          return {};
        }
        DLOG(INFO) << "Click " << std::get<0>(point_info);
        operation.click.x = std::get<1>(point_info);
        operation.click.y = std::get<2>(point_info);
      } else if (action_config.point) {
        operation.click.x = action_config.point->first;
        operation.click.y = action_config.point->second;
        DLOG(INFO) << "Click point " << operation.click.x << " "
                   << operation.click.y;
      }
      ret.push_back(operation);
    } else if (action_config.type == "sleep") {
      DLOG(INFO) << "Sleep " << *action_config.sleep_time;
      PlayOperation operation(PlayOperationType::SLEEP);
      operation.sleep_ms = *action_config.sleep_time;
      ret.push_back(operation);
    } else if (action_config.type == "over") {
      LOG(INFO) << "Play end";
    }
  }
  return ret;
}
