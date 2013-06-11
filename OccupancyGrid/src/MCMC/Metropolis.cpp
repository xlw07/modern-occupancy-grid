/**
 *      @file slowMetropolis.cpp
 *      @date May 23, 2012
 *      @author Brian Peasley
 *      @author Frank Dellaert
 */

#include "../../include/OccupancyGrid.h"
#include "../../include/visualiser.h"
#include <boost/random.hpp>
#include <boost/random/normal_distribution.hpp>

using namespace std;
using namespace gtsam;

/**
 * @brief Run a metropolis sampler.
 * @param iterations defines the number of iterations to run.
 * @return  vector of marginal probabilities.
 */
OccupancyGrid::Marginals runSlowMetropolis(const OccupancyGrid &occupancyGrid,
    size_t iterations) {

  // Create a data structure to hold the estimated marginal occupancy probabilities
  // and initialize to zero.
  size_t size = occupancyGrid.cellCount();
  OccupancyGrid::Marginals marginals(size);
  for (size_t it = 0; it < marginals.size(); it++)
    marginals[it] = 0;

  // Initialize randum number generator
  size_t nrows = occupancyGrid.height();
  size_t ncols = occupancyGrid.width();
  boost::mt19937 rng;
  boost::uniform_int < Index > random_cell(0, size - 1);
  double sigma = 0.05 * nrows ; // next sample point will be within 2*5 cells (95% of the times)

  boost::normal_distribution< double > normal_dist(0, sigma);
  boost::variate_generator<boost::mt19937&,
    boost::normal_distribution< double > > var_nor(rng, normal_dist);

  double dsize   = static_cast<double>(size);
  double dheight = floor(sqrt(dsize));
  size_t height  = static_cast<size_t>(dheight);
  size_t width   = static_cast<size_t>(floor(dsize/dheight));

  // Create empty occupancy as initial state and
  // compute initial neg log-probability of occupancy grid, - log P(x_t)
  LaserFactor::Occupancy occupancy = occupancyGrid.emptyOccupancy();

  double Ex = occupancyGrid(occupancy);
  // global_vis_.init(occupancyGrid.height(), occupancyGrid.width());
  // global_vis_.enable_show();

  // for logging
  vector<double> energy;

  // Choose a random cell
  Index x = random_cell(rng);

  // run Metropolis for the requested number of operations
  for (size_t it = 0; it < iterations; it++) {

    // Log and print
    energy.push_back(Ex);
    if (it % 100 == 0) {
      printf("%lf\n", (double) it / (double) iterations);

      // global_vis_.reset();
      // global_vis_.setOccupancy(occupancy);
      // global_vis_.show();
    }

    // Sample a point close to the previous point with gaussian probability
    // This is a heuristic strategy that high occupancy regions (high
    // probability) are going to be few but close together in space. This is
    // not same as the choosing the proposal distribution for metropolis
    // algorithm as the space we are working is a 100x100 dimensional space
    // rather than a 2D space. But some of the properties of this 2D space can
    // be made use of. This idea is similar to that of a heat map.
    double col = x % ncols;
    double row = x / ncols;
    row += var_nor();
    col += var_nor();
    Index row_lu = (row < 0) ? 0
      : (row >= nrows) ? nrows
      : static_cast<Index>(row);
    Index col_lu = (col < 0) ? 0
      : (col >= ncols) ? ncols
      : static_cast<Index>(col);
    Index x_prime = row_lu * occupancyGrid.width() + col_lu;

    // Flip the state of a random cell, x_prime
    occupancy[x_prime] = 1 - occupancy[x_prime];

    // Compute neg log-probability of new occupancy grid, -log P(x')
    // by summing over all LaserFactor::operator()
    double Ex_prime = occupancyGrid(occupancy);

    // Calculate acceptance ratio, a
    // See e.g. MacKay 96 "Intro to Monte Carlo Methods"
    // a = P(x')/P(x) = exp {-E(x')} / exp {-E(x)} = exp {E(x)-E(x')}
    double a = exp(Ex - Ex_prime);

    // If a <= 1 otherwise accept with probability a
    double rn = static_cast<double>(std::rand()) / (RAND_MAX);
    bool accept = (a>=1) ? true // definitely accept
      : (a >= rn) ?  true       // accept with probability a
      : false;

    //printf("%lu : %lu; accepted: %d\n", x_prime, occupancy.at(x_prime), accept);
    if (accept) {
      Ex = Ex_prime;
      x = x_prime;
    } else {
      // we don't accept: flip it back !
      occupancy[x_prime] = 1 - occupancy[x_prime];
      x = random_cell(rng);
    }

    //increment the number of iterations each cell has been on
    for (size_t i = 0; i < size; i++) {
      if (occupancy[i] == 1)
        marginals[i]++;
    }
  }

  FILE *fptr = fopen("Data/Metropolis_Energy.txt", "w");
  for (int i = 0; i < iterations; i++)
    fprintf(fptr, "%lf ", energy[i]);

  //compute the marginals
  for (size_t it = 0; it < size; it++)
    marginals[it] /= iterations;

  return marginals;
}
