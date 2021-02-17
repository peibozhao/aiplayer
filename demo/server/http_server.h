
#pragma once

#include "httplib.h"
#include "object_detect/object_detect.h"
#include "ocr_detect/ocr_detect.h"
#include "blhx_player/blhx_player.h"

class BlhxHttpServer {
public:
    bool Init(const std::string &config_str);

    void Start();

    void Stop();

private:
    bool InitHttplibServer();

    bool InitAlgorithm(const std::string &detect_fname, const std::string &ocr_fname, const std::string &player_fname);

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
