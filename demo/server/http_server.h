
#pragma once

#include "httplib.h"
#include "object_detect/object_detect.h"
#include "ocr_detect/ocr_detect.h"
#include "blhx_player/blhx_player.h"
#include "utils/util_defines.h"

class BlhxHttpServer {
public:
    InitialDefine

    void Start();

    void Stop();

private:
    bool InitHttplibServer();

    bool InitAlgorithm(const std::string &detect_config, const std::string &ocr_config, const std::string &player_config);

    static void CreatePlayer(BlhxHttpServer *this_, const httplib::Request &request, httplib::Response &response);

    static void Play(BlhxHttpServer *this_, const httplib::Request &request, httplib::Response &response);

    static void TestDetect(BlhxHttpServer *this_, const httplib::Request &request, httplib::Response &response);

    static void TestOcr(BlhxHttpServer *this_, const httplib::Request &request, httplib::Response &response);

private:
    std::string ip_;
    int port_;
    httplib::Server server_;

    std::shared_ptr<IObjectDetect> object_detect_;
    std::shared_ptr<IOcrDetect> ocr_detect_;
    std::shared_ptr<IBLHXPlayer> blhx_player_;
};
