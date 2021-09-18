
#pragma once

#include "application.h"
#include <string>
#include <memory>
#include <atomic>
#include "image_source/image_source.h"
#include "ocr_detect/ocr_detect.h"
#include "player/player.h"
#include "device_operation/device_operation.h"

class BlhxApplication : public IApplication {
public:
    BlhxApplication(const std::string &config_fname);

    bool Init() override;

    void Run() override;

    void Stop() override;

private:
    std::string config_fname_;
    std::shared_ptr<IImageSource> source_;
    std::shared_ptr<IOcrDetect> ocr_;
    std::shared_ptr<IPlayer> player_;
    std::shared_ptr<ITouchScreenOperation> operation_;

    std::atomic<bool> running_;
};

