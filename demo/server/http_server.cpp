
#include "http_server.h"
#include "blhx_player/battle_player.h"
#include "blhx_player/blhx_player.h"
#include "nlohmann/json.hpp"
#include "object_detect/yolov5_detect.h"
#include "ocr_detect/chineselite_ocr.h"
#include "opencv2/opencv.hpp"
#include "utils/util_functions.h"
#include <linux/uuid.h>

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

bool BlhxHttpServer::Init(const std::string &config_str) {
    ip_ = "0.0.0.0";
    port_ = 8080;
    if (InitHttplibServer() && InitAlgorithm("blhx-detect-config.yaml", "chineseocr.yaml", "blhx-player-battle.yaml")) {
        return true;
    }
    return false;
}

void BlhxHttpServer::Start() { server_.listen(ip_.c_str(), port_); }

void BlhxHttpServer::Stop() {}

bool BlhxHttpServer::InitHttplibServer() {
    server_.Post("/blhx/players",
                 [this](const httplib::Request &request, httplib::Response &response) {
                     // Create blhx player
                     CreatePlayer(this, request, response);
                 });

    server_.Patch("/blhx/players/.*",
                  [this](const httplib::Request &request, httplib::Response &response) {
                      // Update blhx player
                      SPDLOG_DEBUG("Request {}", request.path);
                      Play(this, request, response);
                  });

    server_.Post("/test/detect",
                 [this](const httplib::Request &request, httplib::Response &response) {
                     TestDetect(this, request, response);
                 });

    server_.Post("/test/ocr", [this](const httplib::Request &request, httplib::Response &response) {
        TestOcr(this, request, response);
    });
    return true;
}

bool BlhxHttpServer::InitAlgorithm(const std::string &detect_fname, const std::string &ocr_fname,
                                   const std::string &player_fname) {
    object_detect_.reset(new Yolov5Detect());
    ocr_detect_.reset(new ChineseOcr());
    blhx_player_.reset(new BattlePlayer());

    if (!object_detect_->InitWithFile(detect_fname) ||
        !ocr_detect_->InitWithFile(ocr_fname) ||
        !blhx_player_->InitWithFile(player_fname)) {
        return false;
    }

    return true;
}

void BlhxHttpServer::CreatePlayer(BlhxHttpServer *this_, const httplib::Request &request,
                                  httplib::Response &response) {
    std::cout << "Create player" << std::endl;
}

void BlhxHttpServer::Play(BlhxHttpServer *this_, const httplib::Request &request,
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
        SPDLOG_INFO("Ocr {}: {} {} {} {}", box.text, box.x, box.y, box.width, box.height);
    }
    // Detect
    cv::Mat image_rgb;
    cv::cvtColor(image, image_rgb, cv::COLOR_BGR2RGB);
    std::vector<ObjectBox> object_boxes = this_->object_detect_->Detect(image_rgb);
    for (const auto &box : object_boxes) {
        SPDLOG_INFO("Detect {}: {} {} {} {}", box.name, box.x, box.y, box.width, box.height);
    }
    // Player
    std::vector<PlayOperation> operations = this_->blhx_player_->Play(object_boxes, text_boxes);
    for (const auto &opt : operations) {
        SPDLOG_INFO("Play {}", opt.type);
    }
    nlohmann::json res_json;
    for (const PlayOperation &operation : operations) {
        nlohmann::json operation_json = PlayOperation2Json(operation);
        if (operation_json.empty()) {
            continue;
        }
        res_json["operations"].push_back(operation_json);
    }
    response.body = res_json.empty() ? "" : res_json.dump();
}

void BlhxHttpServer::TestDetect(BlhxHttpServer *this_, const httplib::Request &request,
                                httplib::Response &response) {
    std::string request_body = request.body;
    std::cout << request_body << std::endl;
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
    std::cout << response.body << std::endl;
}

void BlhxHttpServer::TestOcr(BlhxHttpServer *this_, const httplib::Request &request,
                             httplib::Response &response) {
    std::string request_body = request.body;
    std::cout << request_body << std::endl;
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
    std::cout << response.body << std::endl;
}
