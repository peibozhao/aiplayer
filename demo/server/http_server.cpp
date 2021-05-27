
#include "http_server.h"
#include "glog/logging.h"
#include "nlohmann/json.hpp"
#include "object_detect/yolov5_detect.h"
#include "ocr_detect/chineselite_ocr.h"
#include "opencv2/opencv.hpp"
#include "player/blhx_player.h"
#include "utils/util_functions.h"
#include "yaml-cpp/yaml.h"
// #include <linux/uuid.h>

static nlohmann::json PlayOperation2Json(const PlayOperation &operation) {
    nlohmann::json root;
    switch (operation.type) {
    case PlayOperationType::SCREEN_CLICK:
        root["type"] = "click";
        root["x"] = operation.click.x;
        root["y"] = operation.click.y;
        break;
    case PlayOperationType::SCREEN_SWIPE:
        root["type"] = "swipe";
        root["delta_x"] = operation.swipe.delta_x;
        root["delta_y"] = operation.swipe.delta_y;
        break;
    case PlayOperationType::LIMITS:
        root["type"] = "limit";
        break;
    default:
        break;
    }
    return root;
}

bool HttpServer::Init(const std::string &config_fname) {
    YAML::Node detect_config, ocr_config, player_config;
    try {
        YAML::Node config = YAML::LoadFile(config_fname);
        ip_ = config["server"]["ip"].as<std::string>();
        port_ = config["server"]["port"].as<int>();
        detect_config = config["detect"];
        ocr_config = config["ocr"];
        player_config = config["player"];
    } catch (std::exception &e) {
        LOG(ERROR) << "Catch error: " << e.what();
        return false;
    }
    object_detect_.reset(new Yolov5Detect());
    ocr_detect_.reset(new ChineseOcr());
    player_.reset(new BLHXPlayer());
    if (!object_detect_->Init(YAML::Dump(detect_config))) {
        return false;
    }
    if (!ocr_detect_->Init(YAML::Dump(ocr_config))) {
        return false;
    }
    if (!player_->Init(YAML::Dump(player_config))) {
        return false;
    }
    if (!InitHttplibServer()) {
        return false;
    }
    return true;
}

void HttpServer::Start() { server_.listen(ip_.c_str(), port_); }

void HttpServer::Stop() {}

bool HttpServer::InitHttplibServer() {
    server_.Post("/blhx/players", [this](const httplib::Request &request,
                                         httplib::Response &response) {
        // Create blhx player
        CreatePlayer(this, request, response);
    });

    server_.Patch("/blhx/players/.*", [this](const httplib::Request &request,
                                             httplib::Response &response) {
        // Update blhx player
        LOG(INFO) << "Request " << request.path;
        Play(this, request, response);
    });

    server_.Post("/test/detect", [this](const httplib::Request &request,
                                        httplib::Response &response) {
        TestDetect(this, request, response);
    });

    server_.Post("/test/ocr", [this](const httplib::Request &request,
                                     httplib::Response &response) {
        TestOcr(this, request, response);
    });
    return true;
}

void HttpServer::CreatePlayer(HttpServer *this_,
                              const httplib::Request &request,
                              httplib::Response &response) {
    LOG(INFO) << "Create player";
}

void HttpServer::Play(HttpServer *this_, const httplib::Request &request,
                      httplib::Response &response) {
    std::string request_body = request.body;
    std::vector<uint8_t> data;
    try {
        nlohmann::json root = nlohmann::json::parse(request_body);
        auto image_node = root["image"];
        std::string image_base64 = image_node["data"].get<std::string>();
        data = Base64Decode(image_base64);
    } catch (std::exception &e) {
        response.status = 400;
    }

    // Ocr
    cv::Mat image = cv::imdecode(data, cv::ImreadModes::IMREAD_COLOR);
    std::vector<TextBox> text_boxes = this_->ocr_detect_->Detect(image);
    for (const auto &box : text_boxes) {
        LOG(INFO) << "Ocr " << box.text << ": " << box.x << " " << box.y << " "
                  << box.width << " " << box.height;
    }

    // Detect
    std::vector<ObjectBox> object_boxes;
    // cv::Mat image_rgb;
    // cv::cvtColor(image, image_rgb, cv::COLOR_BGR2RGB);
    // std::vector<ObjectBox> object_boxes =
    //     this_->object_detect_->Detect(image_rgb);
    // for (const auto &box : object_boxes) {
    //     LOG(INFO) << "Detect " << box.name << ": " << box.x << " " << box.y
    //               << " " << box.width << " " << box.height;
    // }

    // Player
    std::vector<PlayOperation> operations =
        this_->player_->Play(object_boxes, text_boxes);
    for (const auto &opt : operations) {
        LOG(INFO) << "Play " << int(opt.type);
    }
    nlohmann::json res_json;
    for (const PlayOperation &operation : operations) {
        nlohmann::json operation_json = PlayOperation2Json(operation);
        if (operation_json.empty()) {
            continue;
        }
        res_json["operations"].push_back(operation_json);
    }
    response.body = res_json.empty() ? "{}" : res_json.dump();
    LOG(INFO) << "Response: " << response.body;
}

void HttpServer::TestDetect(HttpServer *this_, const httplib::Request &request,
                            httplib::Response &response) {
    std::string request_body = request.body;
    std::vector<uint8_t> data;
    try {
        nlohmann::json root = nlohmann::json::parse(request_body);
        auto image_node = root["image"];
        std::string image_base64 = image_node["data"].get<std::string>();
        data = Base64Decode(image_base64);
    } catch (std::exception &e) {
        response.status = 400;
    }

    nlohmann::json res_json;
    cv::Mat image = cv::imdecode(data, cv::ImreadModes::IMREAD_COLOR);
    std::vector<ObjectBox> object_boxes = this_->object_detect_->Detect(image);
    for (const auto &box : object_boxes) {
        nlohmann::json box_json;
        box_json["x"] = box.x;
        box_json["y"] = box.y;
        box_json["width"] = box.width;
        box_json["height"] = box.height;
        box_json["name"] = box.name;
        res_json["object"].push_back(box_json);
    }

    response.body = res_json.dump();
}

void HttpServer::TestOcr(HttpServer *this_, const httplib::Request &request,
                         httplib::Response &response) {
    std::string request_body = request.body;
    std::vector<uint8_t> data;
    try {
        nlohmann::json root = nlohmann::json::parse(request_body);
        auto image_node = root["image"];
        std::string image_base64 = image_node["data"].get<std::string>();
        data = Base64Decode(image_base64);
    } catch (std::exception &e) {
        response.status = 400;
    }

    nlohmann::json res_json;
    cv::Mat image = cv::imdecode(data, cv::ImreadModes::IMREAD_COLOR);
    std::vector<TextBox> text_boxes = this_->ocr_detect_->Detect(image);
    for (const auto &box : text_boxes) {
        nlohmann::json box_json;
        box_json["x"] = box.x;
        box_json["y"] = box.x;
        box_json["width"] = box.x;
        box_json["height"] = box.x;
        box_json["text"] = box.text;
        res_json["ocr"].push_back(box_json);
    }

    response.body = res_json.dump();
}
