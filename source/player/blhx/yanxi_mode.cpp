
#include "yanxi_mode.h"

std::vector<PlayOperation>
YanxiMode::MainPlay(const std::vector<ObjectBox> &objects,
                    const std::vector<TextBox> &texts) {
    return {Click(texts, "出击")};
}

std::vector<PlayOperation>
YanxiMode::WeighAnchorPlay(const std::vector<ObjectBox> &objects,
                           const std::vector<TextBox> &texts) {
    return {Click(texts, "演习")};
}

std::vector<PlayOperation>
YanxiMode::OperationPlay(const std::vector<ObjectBox> &objects,
                         const std::vector<TextBox> &texts) {
    PlayOperation opt;
    opt.type = PlayOperationType::SCREEN_CLICK;
    opt.click.x = 500;
    opt.click.y = 300;
    return {opt};
}

std::vector<PlayOperation>
YanxiMode::StartOperationPlay(const std::vector<ObjectBox> &objects,
                              const std::vector<TextBox> &texts) {
    return {Click(texts, "开演习")};
}
