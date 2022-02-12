
#include "yaml_parser.h"
#include "utils/log.h"
#include <arpa/inet.h>
#include <netdb.h>

std::tuple<std::string, unsigned short>
GetServerInfo(const YAML::Node &config_yaml) {
    std::string host_name = config_yaml["host"].as<std::string>();
    uint16_t port = config_yaml["port"].as<unsigned short>();
    struct hostent *host = gethostbyname(host_name.c_str());
    if (host == nullptr) {
        LOG_ERROR("Get ip failed %s", host_name.c_str());
        return std::make_tuple("", port);
    }
    return ServerInfo(inet_ntoa(*(struct in_addr *)host->h_addr_list[0]), port);
}

PageConfig GetPageConfig(const YAML::Node &yaml_node) {
    PageConfig ret;
    ret.name = yaml_node["name"].as<std::string>();

    for (auto &page_condition_yaml : yaml_node["conditions"]) {
        PageKeyElement key_element = GetKeyElementConfig(page_condition_yaml);
        ret.key_elements.push_back(key_element);
    }
    return ret;
}

PageKeyElement GetKeyElementConfig(const YAML::Node &yaml_node) {
    PageKeyElement ret;
    ret.pattern = yaml_node["pattern"].as<std::string>();
    if (yaml_node["x_range"].IsDefined()) {
        ret.x_min = yaml_node["x_range"][0].as<float>();
        ret.x_max = yaml_node["x_range"][1].as<float>();
    } else {
        ret.x_min = 0.f;
        ret.x_max = 1.f;
    }
    if (yaml_node["y_range"].IsDefined()) {
        ret.y_min = yaml_node["y_range"][0].as<float>();
        ret.y_max = yaml_node["y_range"][1].as<float>();
    } else {
        ret.y_min = 0.f;
        ret.y_max = 1.f;
    }
    return ret;
}

ModeConfig GetModeConfig(const YAML::Node &yaml_node,
                         const std::vector<ModeConfig> &mode_configs) {
    ModeConfig ret;
    ret.name = yaml_node["name"].as<std::string>();

    for (const auto &inherit_yaml : yaml_node["inherit"]) {
        std::string inherit_name = inherit_yaml.as<std::string>();
        std::for_each(mode_configs.begin(), mode_configs.end(),
                      [&ret, inherit_name](const ModeConfig &mode_config) {
                          if (mode_config.name == inherit_name) {
                              ret.page_pattern_actions =
                                  mode_config.page_pattern_actions;
                          }
                      });
    }

    for (const auto &defined_page_yaml : yaml_node["page_actions"]) {
        std::string page_pattern = defined_page_yaml["page"].as<std::string>();
        std::vector<ActionConfig> action_configs;
        for (auto &action_yaml : defined_page_yaml["actions"]) {
            ActionConfig action_config = GetActionConfig(action_yaml);
            action_configs.push_back(action_config);
        }
        ret.page_pattern_actions.push_back(
            std::make_tuple(std::regex(page_pattern), action_configs));
    }

    if (yaml_node["other_page_actions"].IsDefined()) {
        std::vector<ActionConfig> action_configs;
        const YAML::Node &other_page_yaml = yaml_node["other_page_actions"];
        for (auto action_yaml : other_page_yaml["actions"]) {
            ActionConfig action_config = GetActionConfig(action_yaml);
            ret.other_page_actions.push_back(action_config);
        }
    }

    if (yaml_node["undefined_page_actions"].IsDefined()) {
        std::vector<ActionConfig> action_configs;
        const YAML::Node &undefined_page_yaml =
            yaml_node["undefined_page_actions"];
        for (auto action_yaml : undefined_page_yaml["actions"]) {
            ActionConfig action_config = GetActionConfig(action_yaml);
            ret.undefined_page_actions.push_back(action_config);
        }
    }
    return ret;
}

ActionConfig GetActionConfig(const YAML::Node &action_yaml) {
    ActionConfig action_config;
    std::string action_type = action_yaml["type"].as<std::string>();
    action_config.type = action_type;
    if (action_type == "click") {
        if (action_yaml["pattern"].IsDefined()) {
            action_config.pattern = action_yaml["pattern"].as<std::string>();
        } else if (action_yaml["point"].IsDefined()) {
            action_config.point =
                std::make_pair(action_yaml["point"][0].as<float>(),
                               action_yaml["point"][1].as<float>());
        }
    } else if (action_type == "sleep") {
        action_config.sleep_time = action_yaml["time"].as<int>();
    }
    return action_config;
}
