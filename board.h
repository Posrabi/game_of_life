#pragma once
#include <array>
#include <chrono>
#include <cstring>
#include <functional>
#include <iostream>
#include <thread>
#include <vector>

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

static constexpr int ROWS = 100;
static constexpr int COLS = 100;

using State = unsigned int;
static constexpr State LIVE = 1;
static constexpr State DEAD = 0;
static constexpr std::array<std::pair<int, int>, 8> NEIGHBOR_OFFSETS{
    {{0, 1}, {0, -1}, {1, 0}, {-1, 0}, {1, 1}, {1, -1}, {-1, 1}, {-1, -1}}};

class Board {
public:
  Board(std::vector<std::pair<int, int>> &live_positions) {
    cells = {};
    for (auto &[x, y] : live_positions)
      cells[x][y] = LIVE;
  }

  using Cell = char;
  using Cells = std::array<std::array<Cell, COLS>, ROWS>;
  using GodFunction =
      std::function<State(unsigned int neighbors, State current_state)>;

  void run(GodFunction god_fn) {
    Cells tmp = {};

    // Store new state into tmp board
    for (int i{0}; i < ROWS; ++i)
      for (int j{0}; j < COLS; ++j)
        tmp[i][j] = god_fn(countLiveNeighbors(i, j), cells[i][j]);

    cells = tmp;
  }

  unsigned int countLiveNeighbors(int row, int col) {
    unsigned int ret{0};
    for (auto const &[row_off, col_off] : NEIGHBOR_OFFSETS) {
      auto neighbor_r = row + row_off;
      if (neighbor_r >= ROWS)
        neighbor_r = 0;
      else if (neighbor_r < 0)
        neighbor_r = 99;

      auto neighbor_c = col + col_off;
      if (neighbor_c >= COLS)
        neighbor_c = 0;
      else if (neighbor_c < 0)
        neighbor_c = 99;

      if (cells[neighbor_r][neighbor_c] == LIVE)
        ++ret;
    }

    return ret;
  }

  int print() {
    cv::Mat plot(ROWS, COLS, CV_8U, 255);
    for (unsigned int i{0}; i < ROWS; i++)
      for (unsigned int j{0}; j < COLS; j++)
        if (cells[i][j] == LIVE)
          plot.at<char>(i, j) = 0;

    cv::Mat out;
    cv::resize(plot, out, cv::Size(), 6, 6); // 100x100 is a little too small
    cv::imshow("Board State", out);
    return cv::waitKey(400);
  }

private:
  Cells cells;
};
