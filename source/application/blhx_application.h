
#pragma once

#include "application.h"
#include <string>
#include <memory>
#include <chrono>
#include <mutex>
#include <condition_variable>
#include "source/image/source.h"
#include "ocr_detect/ocr_detect.h"
#include "player/player.h"
#include "sink/device_operation.h"
#include "source/request/request.h"

class BlhxApplication : public IApplication {
private:
    enum ApplicationStatus {
        Stopped,
        Running,
        Pausing,
        Over
    };

public:
    BlhxApplication(const std::string &config_fname);

    bool Init() override;

    void Run() override;

    void Pause() override;

    void Continue() override;

    void Stop() override;

    bool SetParam(const std::string &key, const std::string &value) override;

    std::string GetParam(const std::string &key) override;

private:
    bool QueryCurrentModeCallback(const std::string &request_str, std::string &response_str);

    bool ReplaceCurrentModeCallback(const std::string &request_str, std::string &response_str);

    bool QueryStatusCallback(const std::string &request_str, std::string &response_str);

    bool ReplaceStatusCallback(const std::string &request_str, std::string &response_str);

private:
    std::string config_fname_;
    std::shared_ptr<ISource> source_;
    std::shared_ptr<IOcrDetect> ocr_;
    std::shared_ptr<IPlayer> player_;
    std::shared_ptr<ITouchScreenOperation> operation_;
    std::shared_ptr<IRequest> request_;

    ApplicationStatus status_;
    std::mutex status_mutex_;
    std::condition_variable status_con_;
};

