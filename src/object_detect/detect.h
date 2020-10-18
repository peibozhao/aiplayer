/**
 * @file detect.h
 * @brief 目标检测接口基类
 * @author peibozhao
 * @version 1.0.0
 * @date 2020-10-11
 */

#ifndef OBJECT_DETECT_DETECT_H
#define OBJECT_DETECT_DETECT_H

#include <string>
#include <vector>

struct DetectBox {
  int xmin, xmax, ymin, ymax;
  float conf;
  std::string class_name;
};

class IDetect {
public:
  virtual ~IDetect() {}

  /**
   * @brief 初始化
   *
   * @param 配置字符串
   *
   * @return 是否成功
   */
  virtual bool Init(const std::string &cfg) = 0;

  /**
   * @brief 设置参数. 比如图像大小等
   *
   * @param key
   * @param value
   *
   * @return 
   */
  virtual bool SetParam(const std::string &key, const std::string &value) = 0;

  /**
   * @brief 目标框检测
   *
   * @param data 输入的图像数据
   *
   * @return 检测框
   */
  virtual std::vector<DetectBox> Detect(const std::vector<uint8_t> &data) = 0;
};

#endif  // ifndef OBJECT_DETECT_DETECT_H
