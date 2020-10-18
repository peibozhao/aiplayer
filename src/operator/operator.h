
#ifndef OPERATOR_OPERATOR_H
#define OPERATOR_OPERATOR_H

#include <string>
#include "player/player.h"

class IOperator {
public:
  virtual bool Init(const std::string &cfg) = 0;
  void Operator(PlayOperation operation);

protected:
  virtual void Click(int x, int y) = 0;
};

#endif // ifndef OPERATOR_OPERATOR_H

