
#ifndef OPERATOR_OPERATOR_H
#define OPERATOR_OPERATOR_H

#include <string>
#include "player/player.h"

class IOperator {
public:
  virtual bool Init(const std::string &cfg) = 0;
  bool Operator(PlayOperation operation);

protected:
  virtual bool Click(int x, int y) = 0;
  virtual bool Swipe(int delta_x, int delta_y) = 0;
};

#endif // ifndef OPERATOR_OPERATOR_H

