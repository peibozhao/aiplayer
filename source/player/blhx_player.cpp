
#include "blhx_player.h"
#include "common/log.h"

namespace {
std::string GetCommonPageName(int chapter_num) {
  return "章节选择-普通-" + std::to_string(chapter_num) + "章";
}

std::string GetHardPageName(int chapter_num) {
  return "章节选择-困难-" + std::to_string(chapter_num) + "章";
}
} // namespace

BlhxPlayer::BlhxPlayer(const std::string &name,
                       const std::vector<PageConfig> &page_configs,
                       const std::vector<ModeConfig> &mode_configs)
    : CommonPlayer(name, page_configs, mode_configs) {}

void BlhxPlayer::RegisterSpecialPages() {
  static const int chapter_num = 14;
  static const int section_num = 4;
  static const float prev_chapter_x = 0.05;
  static const float prev_chapter_y = 0.5;
  static const float next_chapter_x = 0.95;
  static const float next_chapter_y = 0.5;

  auto click_point_func = [](const std::vector<Element> &elements, float x,
                             float y) {
    PlayOperation click_operation;
    click_operation.type = PlayOperationType::SCREEN_CLICK;
    click_operation.click.x = x;
    click_operation.click.y = y;
    return std::vector<PlayOperation>({click_operation});
  };

  auto click_section_func = [](const std::vector<Element> &elements,
                               const std::string &chapter_section_name) {
    PlayOperation click_operation;
    click_operation.type = PlayOperationType::SCREEN_CLICK;
    for (const Element element : elements) {
      if (element.name.substr(0, chapter_section_name.size()) ==
          chapter_section_name) {
        click_operation.click.x = element.x;
        click_operation.click.y = element.y;
        return std::vector<PlayOperation>({click_operation});
      }
    }
    LOG(WARNING) << "Not find section: " << chapter_section_name;
    return std::vector<PlayOperation>({click_operation});
  };

  for (int chapter_idx = 1; chapter_idx < chapter_num; ++chapter_idx) {
    for (int section_idx = 1; section_idx < section_num; ++section_idx) {
      std::string common_mode_name =
          std::to_string(chapter_idx) + '-' + std::to_string(section_idx);
      std::string hard_mode_name = "hard-" + common_mode_name;
      std::string chapter_section_name =
          std::to_string(chapter_idx) + '-' + std::to_string(section_idx);

      RegisterSpecialPage(common_mode_name, GetCommonPageName(chapter_idx),
                          std::bind(click_section_func, std::placeholders::_1,
                                    chapter_section_name));
      RegisterSpecialPage(hard_mode_name, GetHardPageName(chapter_idx),
                          std::bind(click_section_func, std::placeholders::_1,
                                    chapter_section_name));

      for (int before_chapter_idx = 1; before_chapter_idx < chapter_idx;
           ++before_chapter_idx) {
        RegisterSpecialPage(common_mode_name,
                            GetCommonPageName(before_chapter_idx),
                            std::bind(click_point_func, std::placeholders::_1,
                                      next_chapter_x, next_chapter_y));
        RegisterSpecialPage(hard_mode_name, GetHardPageName(before_chapter_idx),
                            std::bind(click_point_func, std::placeholders::_1,
                                      next_chapter_x, next_chapter_y));
      }

      for (int behind_chapter_idx = chapter_idx + 1;
           behind_chapter_idx < chapter_num; ++behind_chapter_idx) {
        RegisterSpecialPage(common_mode_name,
                            GetCommonPageName(behind_chapter_idx),
                            std::bind(click_point_func, std::placeholders::_1,
                                      prev_chapter_x, prev_chapter_y));
        RegisterSpecialPage(hard_mode_name, GetHardPageName(behind_chapter_idx),
                            std::bind(click_point_func, std::placeholders::_1,
                                      prev_chapter_x, prev_chapter_y));
      }
    }
  }
}
