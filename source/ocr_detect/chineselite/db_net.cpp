
#include "db_net.h"
#include "MNN/ImageProcess.hpp"

bool DbNet::InitModel(const std::string &model_fname) {
    mnn_net_.reset(MNN::Interpreter::createFromFile(model_fname.c_str()));
    if (mnn_net_ == nullptr) {
        return false;
    }
    MNN::ScheduleConfig mnn_config;
    mnn_session_ = mnn_net_->createSession(mnn_config);
    return mnn_session_ != nullptr;
}

std::vector<ChineseliteTextBox> DbNet::Detect(const cv::Mat &image) {
    MNN::Tensor *mnn_input = mnn_net_->getSessionInput(mnn_session_, nullptr);
    std::vector<int> input_shape{1, 3, image.rows, image.cols};
    std::cout << "-------------------------------------------" << std::endl;
    mnn_net_->resizeTensor(mnn_input, input_shape);
    mnn_net_->resizeSession(mnn_session_);
    std::cout << "-------------------------------------------" << std::endl;
    MNN::Tensor *mnn_output = mnn_net_->getSessionOutput(mnn_session_, nullptr);

    std::shared_ptr<MNN::Tensor> mnn_input_temp(MNN::Tensor::create(input_shape, image.data, MNN::Tensor::DimensionType::TENSORFLOW));

    MNN::CV::ImageProcess::Config image_cfg;
    image_cfg.sourceFormat = MNN::CV::ImageFormat::RGB;
    image_cfg.destFormat = MNN::CV::ImageFormat::RGB;
    memcpy(image_cfg.mean, mean_, sizeof(mean_));
    memcpy(image_cfg.normal, norm_, sizeof(norm_));
    std::shared_ptr<MNN::CV::ImageProcess> image_process(
        MNN::CV::ImageProcess::create(MNN::CV::RGB, MNN::CV::RGB, mean_, 3, norm_, 3));
    image_process->convert((const uint8_t *)image.data, image.cols, image.rows, image.cols,
                           mnn_input);

    std::cout << mnn_input->getDimensionType() << std::endl;
    std::cout << mnn_input->host<float>()[0] << std::endl;
    std::cout << mnn_input->host<float>()[100] << std::endl;
    // for (int i = 0; i < 1000; ++i) {
    //     std::cout << mnn_input->host<float>()[i] << std::endl;
    // }
    MNN::ErrorCode predict_ret = mnn_net_->runSession(mnn_session_);
    if (predict_ret != MNN::NO_ERROR) {
        std::cout << "Predict Error" << std::endl;
        return {};
    }

    cv::Mat output_image(image.rows, image.cols, CV_32FC1);
    memcpy(output_image.data, mnn_output->host<float>(), image.cols * image.rows * sizeof(float));

    cv::Mat norm_image = output_image >= thresh_;

    cv::imwrite("line.png", norm_image);

    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(norm_image, contours, cv::RETR_LIST, cv::CHAIN_APPROX_SIMPLE);
    std::vector<ChineseliteTextBox> boxes_ret;
    for (const std::vector<cv::Point> &contour : contours) {
        cv::Rect rect = cv::boundingRect(contour);
        ChineseliteTextBox box;
        box.points.push_back(std::make_pair(rect.x, rect.y));
        box.points.push_back(std::make_pair(rect.x + rect.width, rect.y + rect.height));
        boxes_ret.push_back(box);
    }

    for (const auto &box : boxes_ret) {
        // std::cout << box.points[0].first << " " << box.points[0].second << std::endl;
        cv::Rect rect(box.points[0].first, box.points[0].second,
                      box.points[1].first - box.points[0].first,
                      box.points[1].second - box.points[0].second);
        cv::rectangle(image, rect, cv::Scalar(255, 0, 0), 5);
    }
    cv::imwrite("output.png", image);
    return boxes_ret;
}
