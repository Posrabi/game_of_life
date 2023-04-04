#include "board.h"
#include <chrono>
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

  std::cout << "Iteration beginning, count: 100, board size 256x256\n";

  auto board = Board(256);

  auto begin = std::chrono::steady_clock::now();
  for (unsigned int i{0}; i < 100; ++i) {
    board.run(god_fn_0);
  }
  auto end = std::chrono::steady_clock::now();

  std::cout << "Elapsed: "
            << std::chrono::duration_cast<std::chrono::milliseconds>(end -
                                                                     begin)
                   .count()
            << "[ms]"
            << "\n";

  board.print();
  board.destroyWindow();

  std::cout << "Iteration beginning, count: 1000, board size 2048x2048\n";

  board = Board(1000);

  begin = std::chrono::steady_clock::now();
  for (unsigned int i{0}; i < 1000; ++i) {
    board.run(god_fn_0);
  }
  end = std::chrono::steady_clock::now();

  std::cout << "Elapsed: "
            << std::chrono::duration_cast<std::chrono::milliseconds>(end -
                                                                     begin)
                   .count()
            << "[ms]"
            << "\n";

  board.print();
  board.destroyWindow();
}
