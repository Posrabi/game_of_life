#pragma once
#include <array>
#include <chrono>
#include <cstring>
#include <functional>
#include <x86intrin.h>

#include <iostream>
#include <random>
#include <vector>

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#include "board.h"

template <int size> class BoardOptimized {
public:
  /*
   * Cell data layout, 1 byte
   *
   * 0 0 0    | 0 0 0 0        | 0
   * Not used | neighbor count | state
   */
  using Cell = char;
  using Cells = Cell[size][size];

  template <bool is_add> void updateNeighborCount(int i, int j) {
    int off = -2;
    if constexpr (is_add)
      off = 2;

    constexpr int mod = size - 1;
    int right_wrap{(j + 1) & mod}, up_wrap{(i - 1) & mod},
        down_wrap{(i + 1) & mod}, left_wrap{(j - 1) & mod};

#pragma omp atomic update
    cells[down_wrap][right_wrap] += off;
#pragma omp atomic update
    cells[down_wrap][left_wrap] += off;
#pragma omp atomic update
    cells[up_wrap][right_wrap] += off;
#pragma omp atomic update
    cells[up_wrap][left_wrap] += off;
#pragma omp atomic update
    cells[down_wrap][j] += off;
#pragma omp atomic update
    cells[up_wrap][j] += off;
#pragma omp atomic update
    cells[i][right_wrap] += off;
#pragma omp atomic update
    cells[i][left_wrap] += off;
  }

  BoardOptimized() {
    std::default_random_engine generator(0);
    std::uniform_real_distribution<double> dist(0.0, 1.0);

    for (int i{0}; i < (int)size; i++)
      for (int j{0}; j < (int)size; j++)
        if (dist(generator) >= 0.5) {
          cells[i][j] |= 0x01;
          updateNeighborCount<true>(i, j);
        }
  }
  ~BoardOptimized() = default;

  void run() {
    memcpy(tmp, cells, length);

    unsigned count{0};
#pragma omp parallel for collapse(2) private(count)
    for (int i = 0; i < size; ++i)
      for (int j = 0; j < size; ++j) {
        if (!tmp[i][j]) [[likely]]
          continue;

        count = tmp[i][j] >> 1;
        if (tmp[i][j] & 0x01) {
          if (count != 2 && count != 3) {
            cells[i][j] &= ~0x01;
            updateNeighborCount<false>(i, j);
          }
        } else if (count == 3) {
          cells[i][j] |= 0x01;
          updateNeighborCount<true>(i, j);
        }
      }
  }

  int print() {
    cv::Mat plot(size, size, CV_8U, 255);
    for (unsigned int i{0}; i < size; i++)
      for (unsigned int j{0}; j < size; j++)
        if (cells[i][j] & 0x01)
          plot.at<char>(i, j) = 0;

    cv::Mat out;
    cv::resize(plot, out, cv::Size(), 2, 2); // 100x100 is a little too small
    cv::imshow("Board State", out);
    return cv::waitKey(200);
  }

  void destroyWindow() { cv::destroyAllWindows(); }
  char get_at(int i, int j) { return cells[i][j] & 0x01; }

private:
  Cells cells;
  Cells tmp;

  static constexpr size_t length = size * size;
};
