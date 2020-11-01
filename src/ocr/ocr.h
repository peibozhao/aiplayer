
#ifndef OCR_OCR_H
#define OCR_OCR_H

#include <string>
#include <vector>

struct DetectWord {
  int xmin, xmax, ymin, ymax;
  float conf;
  std::string word;

  DetectWord() : xmin(0), xmax(0), ymin(0), ymax(0), conf(0.0) {
  }
};

class IOCR {
public:
  virtual ~IOCR() {}

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
  virtual bool SetParam(const std::string &key, const std::string &value) {
    return true;
  }

  /**
   * @brief 检测文字
   *
   * @param data 输入的图像数据
   * @param words [out] 检测结果
   *
   * @return 是否成功
   */
  virtual bool Detect(const std::vector<uint8_t> &data, std::vector<DetectWord> &words) = 0;
};

#endif // ifndef OCR_OCR_H

