#pragma once

#include "player/common_player.h"
#include "yaml-cpp/yaml.h"
#include <string>
#include <tuple>

// ip, port
typedef std::tuple<std::string, uint16_t> ServerInfo;

ServerInfo GetServerInfo(const YAML::Node &config_yaml);

/// @brief 解析player的page识别配置
PageConfig GetPageConfig(const YAML::Node &yaml_node);

PageKeyElement GetKeyElementConfig(const YAML::Node &yaml_node);

/// @brief 解析player的mode配置
ModeConfig GetModeConfig(const YAML::Node &yaml_node);

ActionConfig GetActionConfig(const YAML::Node &action_yaml);
