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

template <unsigned int size> class BoardOptimized {
public:
  /*
   * Cell data layout 1 byte
   *
   * 0 0 0    | 0 0 0 0        | 0
   * Not used | neighbor count | state
   */
  using Cell = char;
  using Cells = Cell[size][size];

  // Pad around the boundaries to prevent branches in countLiveNeighbors
  // void updatePaddingEdges(Cells &grid) {
  //   for (unsigned int i{1}; i < size + 1; i++) {
  //     // Top edge
  //     grid[0][i] = grid[size][i];
  //     // Bottom edge
  //     grid[size + 1][i] = grid[1][i];
  //     // Left edge
  //
  //   // Pad around the boundaries to prevent branches in countLiveNeighbors
  //     grid[i][0] = grid[i][size];
  //     // Right edge
  //     grid[i][size + 1] = grid[i][1];
  //   }

  //   grid[0][0] = grid[size][size];
  //   grid[size + 1][size + 1] = grid[1][1];
  //   grid[0][size + 1] = grid[size][1];
  //   grid[size + 1][0] = grid[1][size];
  // }

  template <bool is_add> void updateNeighborCount(int i, int j) {
    int off = -2;
    if constexpr (is_add)
      off = 2;

    constexpr unsigned int mod = size - 1;
    cells[(i + 1) & (mod)][(j + 1) & (mod)] += off;
    cells[(i + 1) & (mod)][(j - 1) & (mod)] += off;
    cells[(i - 1) & (mod)][(j + 1) & (mod)] += off;
    cells[(i - 1) & (mod)][(j - 1) & (mod)] += off;
    cells[(i + 1) & (mod)][j] += off;
    cells[(i - 1) & (mod)][j] += off;
    cells[i][(j + 1) & (mod)] += off;
    cells[i][(j - 1) & (mod)] += off;
  }

  BoardOptimized() {
    std::default_random_engine generator(0);
    std::uniform_real_distribution<double> dist(0.0, 1.0);

    for (unsigned int i{0}; i < (int)size; i++)
      for (unsigned int j{0}; j < (int)size; j++)
        if (dist(generator) >= 0.5) {
          cells[i][j] |= 0x01;
          updateNeighborCount<true>(i, j);
        }

    length = size * size;
    // updatePaddingEdges(cells);
  }
  ~BoardOptimized() = default;

  void run() {
    memcpy(tmp, cells, length);

    Cell *start = &tmp[0][0];
    Cell *cells_ptr = start;
    const Cell *end = start + length;
    long count{0}, diff{0};
    int i{0}, j{0};

    while (cells_ptr < end) {
      for (; cells_ptr <= end - 16; cells_ptr += 16) {
        __m128i v0 = _mm_loadu_si128((const __m128i *)cells_ptr);
        // __m128i v1 = _mm_loadu_si128((const __m128i *)(cells_ptr + 16));
        // __m128i v2 = _mm_loadu_si128((const __m128i *)(cells_ptr + 32));
        // __m128i v3 = _mm_loadu_si128((const __m128i *)(cells_ptr + 48));
        __m128i vcmp0 = _mm_cmpeq_epi32(v0, _mm_setzero_si128());
        // __m128i vcmp1 = _mm_cmpeq_epi32(v1, _mm_setzero_si128());
        // __m128i vcmp2 = _mm_cmpeq_epi32(v2, _mm_setzero_si128());
        // __m128i vcmp3 = _mm_cmpeq_epi32(v3, _mm_setzero_si128());
        int mask0 = _mm_movemask_epi8(vcmp0);
        // int mask1 = _mm_movemask_epi8(vcmp1);
        // int mask2 = _mm_movemask_epi8(vcmp2);
        // int mask3 = _mm_movemask_epi8(vcmp3);

        if (mask0 != 0xffff) {
          break;
        }
      }

      diff = cells_ptr - start;
      j = diff & (size - 1), i = diff / size;

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

      ++cells_ptr;
    }

    // for (int i{0}; i < (int)size; ++i)
    //   for (int j{0}; j < (int)size; ++j) {
    //     if (tmp[i][j] == 0) {
    //       continue;
    //     }

    //     auto count = tmp[i][j] >> 1;
    //     if (tmp[i][j] & 0x01) {
    //       if (count != 2 && count != 3) {
    //         cells[i][j] &= ~0x01;
    //         updateNeighborCount<false>(i, j);
    //       }
    //     } else if (count == 3) {
    //       cells[i][j] |= 0x01;
    //       updateNeighborCount<true>(i, j);
    //     }
    //   }
  }

  // unsigned int countLiveNeighbors(int row, int col) {
  //   return cells[row + 1][col + 1] + cells[row + 1][col - 1] +
  //          cells[row - 1][col + 1] + cells[row - 1][col - 1] +
  //          cells[row + 1][col] + cells[row - 1][col] + cells[row][col + 1] +
  //          cells[row][col - 1];
  // }

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
  size_t length;
};