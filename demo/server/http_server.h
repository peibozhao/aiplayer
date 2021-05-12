
#pragma once

#include "httplib.h"
#include "object_detect/object_detect.h"
#include "ocr_detect/ocr_detect.h"
#include "player/player.h"

class HttpServer {
public:
    bool Init(const std::string &config_fname);

    void Start();

    void Stop();

private:
    bool InitHttplibServer();

    bool InitAlgorithm(const std::string &detect_config, const std::string &ocr_config, const std::string &player_config);

    static void CreatePlayer(HttpServer *this_, const httplib::Request &request, httplib::Response &response);

    static void Play(HttpServer *this_, const httplib::Request &request, httplib::Response &response);

    static void TestDetect(HttpServer *this_, const httplib::Request &request, httplib::Response &response);

    static void TestOcr(HttpServer *this_, const httplib::Request &request, httplib::Response &response);

private:
    std::string ip_;
    int port_;
    httplib::Server server_;

    std::shared_ptr<IObjectDetect> object_detect_;
    std::shared_ptr<IOcrDetect> ocr_detect_;
    std::shared_ptr<IPlayer> player_;
};
