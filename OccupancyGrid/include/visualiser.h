/**
 * @file: visualiser.h
 * @data: June 7, 2013
 * @author: Vikas Dhiman
 */
#pragma once
#include <opencv2/opencv.hpp>
#include <boost/format.hpp>

static const cv::Vec3f GREEN(0, 255, 0);
static const cv::Vec3f WHITE(255, 255, 255);
static const cv::Vec3f BLACK(0, 0, 0);

/**
 * @brief: Visualize the occupancy of cells being affected by highlighed laser
 * cells
 */
class Visualiser {
private:
  /// Opencv image to be visualized
  cv::Mat vis_;
  /// Turn on visualization on demand
  bool enable_show_;
  mutable int count_;

public:
  inline Visualiser() : enable_show_(false), count_(0) { }
  Visualiser(size_t nrows, size_t ncols) : vis_(nrows, ncols, CV_8UC3), enable_show_(false), count_(0) { }

  /// For lazy initialization
  inline void init(size_t nrows, size_t ncols) { vis_.create(nrows, ncols, CV_8UC3); }

  /// Convert Index to x,y coordinates for opencv mat
  inline void idx2xy(const gtsam::Index idx, size_t& x, size_t& y) {
    if (vis_.cols == 0) return;
    y = idx / vis_.cols;
    x = idx % vis_.cols;
  }

  /// Set Occupancy for visualization
  inline void setOccupancy(const LaserFactor::Occupancy& occ) {
    for (size_t i = 0; i < vis_.rows; i++)
      for (size_t j = 0; j < vis_.cols; j++)
        vis_.at<cv::Vec3b>(i, j) = (occ.at(i*vis_.cols + j) != 0) ? BLACK : WHITE;
  }

  inline void setMarginals(const std::vector<double>& marg) {
    double max_ele = *std::max_element(marg.begin(), marg.end());
    for (size_t i = 0; i < vis_.rows; i++) {
      for (size_t j = 0; j < vis_.cols; j++) {
        uint8_t val = (max_ele - marg.at(i*vis_.cols + j)) * 255 / max_ele;
        vis_.at<cv::Vec3b>(i, j) = cv::Vec3b(val, val, val);
      }
    }
  }

  /// Highlight given vector of cells
  inline void highlightCells(const std::vector<gtsam::Index> & cells) {
    for (int i = 0; i < cells.size(); i++) {
      gtsam::Index idx = cells[i];
      size_t x,y;
      idx2xy(idx, x, y);
      vis_.at<cv::Vec3b>(y, x) = GREEN;
    }
  }

  /// clear the visualization
  inline void reset() { init(vis_.rows, vis_.cols); }

  /// Enable visualization with show function
  inline void enable_show() { enable_show_ = true; }

  /// update the window
  void show(int t = 33) const {
    if (! enable_show_) return;
    cv::imshow("c", vis_);
    boost::format format("/tmp/occgridvis%d.png");
    cv::imwrite((format % count_++).str(), vis_);
    cv::waitKey(t);
  }
};

extern Visualiser global_vis_;