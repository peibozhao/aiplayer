
#include "operator.h"

void IOperator::Operator(PlayOperation operation) {
  switch (operation.type) {
    case PlayOperationType::SCREEN_CLICK:
      Click(operation.click.x, operation.click.y);
      break;
    default:
      break;
  }
}
