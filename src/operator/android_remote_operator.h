
#ifndef OPERATOR_ANDROID_REMOTE_OPERATOR
#define OPERATOR_ANDROID_REMOTE_OPERATOR

#include "operator.h"

class AndroidRemoteOperator : public IOperator {
public:
  bool Init(const std::string &cfg) override;

private:
  void Click(int x, int y) override;

private:
  std::string click_cmd_;
};

#endif // ifndef OPERATOR_ANDROID_REMOTE_OPERATOR
