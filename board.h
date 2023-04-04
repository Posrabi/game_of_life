#pragma once
#include <array>
#include <chrono>
#include <cstring>
#include <functional>
#include <iostream>
#include <random>
#include <vector>

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

using State = char;
static constexpr State LIVE = 1;
static constexpr State DEAD = 0;
static constexpr std::array<std::pair<int, int>, 8> NEIGHBOR_OFFSETS{
    {{0, 1}, {0, -1}, {1, 0}, {-1, 0}, {1, 1}, {1, -1}, {-1, 1}, {-1, -1}}};

class Board {
public:
  Board(size_t n) {
    cells.resize(n);
    for (auto &row : cells)
      row.resize(n);

    std::default_random_engine generator(0);
    std::uniform_real_distribution<double> dist(0.0, 1.0);

    for (auto &row : cells)
      for (auto &cell : row)
        if (dist(generator) >= 0.5)
          cell = LIVE;
  }
  ~Board() = default;

  using Cell = char;
  using Cells = std::vector<std::vector<Cell>>;
  using GodFunction =
      std::function<State(unsigned int neighbors, State current_state)>;

  void run(GodFunction god_fn) {
    Cells tmp(cells.size());
    for (auto &row : tmp)
      row.resize(cells.size());

    // Store new state into tmp board
    for (int i{0}; i < cells.size(); ++i)
      for (int j{0}; j < cells.size(); ++j)
        tmp[i][j] = god_fn(countLiveNeighbors(i, j), cells[i][j]);

    cells = tmp;
  }

  unsigned int countLiveNeighbors(int row, int col) {
    unsigned int ret{0};
    for (auto const &[row_off, col_off] : NEIGHBOR_OFFSETS) {
      auto neighbor_r = row + row_off;
      if (neighbor_r >= cells.size())
        neighbor_r = 0;
      else if (neighbor_r < 0)
        neighbor_r = cells.size() - 1;

      auto neighbor_c = col + col_off;
      if (neighbor_c >= cells.size())
        neighbor_c = 0;
      else if (neighbor_c < 0)
        neighbor_c = cells.size() - 1;

      if (cells[neighbor_r][neighbor_c] == LIVE)
        ++ret;
    }

    return ret;
  }

  int print() {
    cv::Mat plot(cells.size(), cells.size(), CV_8U, 255);
    for (unsigned int i{0}; i < cells.size(); i++)
      for (unsigned int j{0}; j < cells.size(); j++)
        if (cells[i][j] == LIVE)
          plot.at<char>(i, j) = 0;

    cv::Mat out;
    cv::resize(plot, out, cv::Size(), 2, 2); // 100x100 is a little too small
    cv::imshow("Board State", out);
    return cv::waitKey(200);
  }

  void destroyWindow() { cv::destroyAllWindows(); }
  bool match(std::function<char(int, int)> get_other_at) {
    for (unsigned int i{0}; i < cells.size(); i++)
      for (unsigned int j{0}; j < cells.size(); j++)
        if (cells[i][j] != get_other_at(i, j))
          return false;

    return true;
  }

private:
  Cells cells;
};
