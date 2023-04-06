#include "board.h"
#include "board_optimized.h"
#include <chrono>
#include <opencv2/highgui.hpp>
#include <unistd.h>

int main() {
  Board::GodFunction god_fn_0 = [](unsigned int neighbors,
                                   State current_state) {
    if (current_state == LIVE) {
      if (neighbors == 2 || neighbors == 3)
        return LIVE;

      return DEAD;             // greater than 3 die
    } else if (neighbors == 3) // dead but has exactly 3 neighbors live
      return LIVE;

    return DEAD;
  };

  /*
   ***************************** Step 1 ********************************
   */

  std::cout << "Iteration beginning, count: 100, board size 256x256\n";

  auto board_256 = Board(256);

  auto begin = std::chrono::steady_clock::now();
  for (unsigned int i{0}; i < 100; ++i) {
    board_256.run(god_fn_0);
  }
  auto end = std::chrono::steady_clock::now();

  std::cout << "Elapsed: "
            << std::chrono::duration_cast<std::chrono::milliseconds>(end -
                                                                     begin)
                   .count()
            << " [ms]"
            << "\n";

  auto board_optimized_256 = BoardOptimized<256>();

  begin = std::chrono::steady_clock::now();
  for (unsigned int i{0}; i < 100; ++i) {
    board_optimized_256.run();
  }
  end = std::chrono::steady_clock::now();

  std::cout << "[OPTIMIZED] Elapsed: "
            << std::chrono::duration_cast<std::chrono::milliseconds>(end -
                                                                     begin)
                   .count()
            << " [ms]"
            << "\n";

  // board_optimized_256.destroyWindow();

  if (board_256.match(
          [&](int i, int j) { return board_optimized_256.get_at(i, j); }))
    std::cout << "States matched!\n";
  else
    std::cout << "States did not match!\n";

  /*
   ***************************** Step 2 ********************************
   */

  std::cout << "Iteration beginning, count: 1000, board size 2048x2048\n";

  // auto board_2048 = Board(2048);

  // begin = std::chrono::steady_clock::now();
  // for (unsigned int i{0}; i < 1000; ++i) {
  //   board_2048.run(god_fn_0);
  // }

  // end = std::chrono::steady_clock::now();

  // std::cout << "Elapsed: "
  //           << std::chrono::duration_cast<std::chrono::milliseconds>(end -
  //                                                                    begin)
  //                  .count()
  //           << " [ms]"
  //           << "\n";

  auto board_optimized_2048 = BoardOptimized<2048>();

  begin = std::chrono::steady_clock::now();
  for (unsigned int i{0}; i < 1000; ++i) {
    board_optimized_2048.run();
  }
  end = std::chrono::steady_clock::now();

  std::cout << "[OPTIMIZED] Elapsed: "
            << std::chrono::duration_cast<std::chrono::milliseconds>(end -
                                                                     begin)
                   .count()
            << " [ms]"
            << "\n";

  // if (board_2048.match(
  //         [&](int i, int j) { return board_optimized_2048.get_at(i, j); }))
  //   std::cout << "States matched!\n";
  // else
  //   std::cout << "States did not match!\n";

  return 0;
}
