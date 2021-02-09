
#pragma once

#include "httplib.h"
#include "object_detect/object_detect.h"
#include "ocr_detect/ocr_detect.h"
#include "blhx_player/blhx_player.h"

class HttpServer {
public:
    void Init(const std::string &config_str);

    void Start();

    void Stop();

private:
    void InitHttplibServer();

private:
    std::string ip_;
    int port_;
    httplib::Server server_;

    IObjectDetect *object_detect_;
    IOcrDetect *ocr_detect_;
    IBLHXPlayer *blhx_player_;
};
