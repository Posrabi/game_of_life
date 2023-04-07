#pragma once
#include <array>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <omp.h>
#include <random>
#include <unistd.h>
#include <x86intrin.h>

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#include "board.h"

#define indexToPtr(ptr, i, j, size) *(ptr + j + i * size)
#define CACHE_LINE_SIZE sysconf(_SC_LEVEL1_DCACHE_LINESIZE)

template <int size> class BoardOptimized {
public:
  /*
   * Cell data layout, 1 byte
   *
   * 0 0 0    | 0 0 0 0        | 0
   * Not used | neighbor count | state
   */

  template <bool is_on> void modifyCell(int i, int j) {
    int off = -2;
    if constexpr (is_on) {
      off = 2;
#pragma omp atomic update
      indexToPtr(cells, i, j, size) |= 0x01;
    } else {
#pragma omp atomic update
      indexToPtr(cells, i, j, size) &= ~0x01;
    }

    constexpr int mod = size - 1;
    int right_wrap{(j + 1) & mod}, up_wrap{(i - 1) & mod},
        down_wrap{(i + 1) & mod}, left_wrap{(j - 1) & mod};

    // Add to the neighbor count of the neighboring cell (bit 1 - 4)
#pragma omp atomic update
    indexToPtr(cells, down_wrap, right_wrap, size) += off;
#pragma omp atomic update
    indexToPtr(cells, down_wrap, left_wrap, size) += off;
#pragma omp atomic update
    indexToPtr(cells, up_wrap, right_wrap, size) += off;
#pragma omp atomic update
    indexToPtr(cells, up_wrap, left_wrap, size) += off;
#pragma omp atomic update
    indexToPtr(cells, down_wrap, j, size) += off;
#pragma omp atomic update
    indexToPtr(cells, up_wrap, j, size) += off;
#pragma omp atomic update
    indexToPtr(cells, i, right_wrap, size) += off;
#pragma omp atomic update
    indexToPtr(cells, i, left_wrap, size) += off;
  }

  BoardOptimized() {
    cells = (char *)aligned_alloc(CACHE_LINE_SIZE, length);
    memset(cells, 0, length);
    tmp = (char *)aligned_alloc(CACHE_LINE_SIZE, length);

    std::default_random_engine generator(0);
    std::uniform_real_distribution<double> dist(0.0, 1.0);

    for (int i{0}; i < (int)size; i++)
      for (int j{0}; j < (int)size; j++)
        if (dist(generator) >= 0.5) {
          modifyCell<true>(i, j);
        }
  }
  ~BoardOptimized() {
    free(cells);
    free(tmp);
  }

  void run() {
    memcpy(tmp, cells, length);

    unsigned count{0};
    char current;
#pragma omp parallel for collapse(2) private(count, current)                   \
    schedule(static, 256)
    for (int i = size - 1; i >= 0; --i)
      for (int j = size - 1; j >= 0; --j) {
        current = indexToPtr(tmp, i, j, size);
        if (!current) [[likely]]
          continue;

        count = current >> 1;
        if (current & 0x01) {
          if (count != 2 && count != 3) {
            modifyCell<false>(i, j);
          }
        } else if (count == 3) {
          modifyCell<true>(i, j);
        }
      }
  }

  int print() {
    cv::Mat plot(size, size, CV_8U, 255);
    for (unsigned int i{0}; i < size; i++)
      for (unsigned int j{0}; j < size; j++)
        if (indexToPtr(cells, i, j, size) & 0x01)
          plot.at<char>(i, j) = 0;

    cv::Mat out;
    cv::resize(plot, out, cv::Size(), 2, 2); // 100x100 is a little too small
    cv::imshow("Board State", out);
    return cv::waitKey(0);
  }

  void destroyWindow() { cv::destroyAllWindows(); }
  char get_at(int i, int j) { return indexToPtr(cells, i, j, size) & 0x01; }

private:
  char *cells;
  char *tmp;

  static constexpr size_t length = size * size;
};
