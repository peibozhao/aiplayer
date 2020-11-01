
#ifndef OPERATOR_ANDROID_REMOTE_OPERATOR
#define OPERATOR_ANDROID_REMOTE_OPERATOR

#include "operator.h"

class AndroidRemoteOperator : public IOperator {
public:
  bool Init(const std::string &cfg) override;

private:
  bool Click(int x, int y) override;
  bool Swipe(int delta_x, int delta_y) override;

private:
  std::string click_cmd_;
  std::string swipe_cmd_;
  int screen_width_, screen_height_;
};

#endif // ifndef OPERATOR_ANDROID_REMOTE_OPERATOR
