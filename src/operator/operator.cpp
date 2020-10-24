
#include "operator.h"

bool IOperator::Operator(PlayOperation operation) {
  int ret = true;
  switch (operation.type) {
    case PlayOperationType::SCREEN_CLICK:
      ret = Click(operation.click.x, operation.click.y);
      break;
    default:
      break;
  }
  return ret;
}
