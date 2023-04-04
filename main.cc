#include "board.h"
#include <opencv2/highgui.hpp>
#include <unistd.h>

int main() {
  Board::GodFunction god_fn_0 = [](unsigned int neighbors,
                                   State current_state) {
    if (current_state == LIVE) {
      if (neighbors >= 2 && neighbors <= 3)
        return LIVE;

      return DEAD;             // greater than 3 die
    } else if (neighbors == 3) // dead but has exactly 3 neighbors live
      return LIVE;

    return DEAD;
  };

  Board::GodFunction god_fn_1 = [](unsigned int neighbors,
                                   State current_state) {
    if (current_state == LIVE) {
      if (neighbors >= 1 && neighbors <= 2)
        return LIVE;

      return DEAD;
    } else if (neighbors == 2)
      return LIVE;

    return DEAD;
  };

  Board::GodFunction god_fn_2 = [](unsigned int neighbors,
                                   State current_state) {
    if (current_state == LIVE) {
      if (neighbors >= 1)
        return LIVE;

      return DEAD;
    } else if (neighbors == 1)
      return LIVE;

    return DEAD; // should never take this path
  };

  std::vector<std::pair<int, int>> start_positions = {
      {17, 11}, {20, 11}, {18, 10}, {21, 10}, {13, 9}, {14, 9},
      {16, 9},  {17, 9},  {18, 9},  {19, 9},  {20, 9}, {21, 9},
      {13, 8},  {14, 8},  {10, 7},  {11, 7},  {10, 6}, {11, 6},
      {11, 5},  {12, 5},  {11, 4},  {12, 4}};

  std::cout << "Iteration beginning, press 'q' on the pop-up screen to stop, "
               "max iteration: 500\n";

  auto board = Board(256, start_positions);
  for (unsigned int i{0}; i < 500; ++i) {
    if (board.print() == 'q')
      break;

    std::cout << "Iteration: " << i << "\n";
    board.run(god_fn_2);
  }

  board.print();
}
