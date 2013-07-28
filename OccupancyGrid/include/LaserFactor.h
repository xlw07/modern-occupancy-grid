/**
 *      @file LaserFactor.h
 *      @author Brian Peasley
 */

#pragma once

#include <gtsam/discrete/DiscreteFactorGraph.h>
#include <gtsam/geometry/Pose2.h>

#include <CppUnitLite/TestHarness.h>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int.hpp>

/**
 * Laser Factor
 * @brief factor that encodes a laser measurements likelihood.
 */
class LaserFactor : public gtsam::DiscreteFactor{
  friend class OccupancyGrid;

private:
	std::vector<gtsam::Index> cells_;	///cells in which laser passes through

public:

  typedef gtsam::DiscreteFactor::Values Occupancy;

	/// Constructor
	LaserFactor(const std::vector<gtsam::Index> &cells) : cells_(cells) {}

	/**
	 * Find value for given assignment of values to variables
   * returns 1000 if any of the non-last cell is occupied and 1 otherwise
   * returns 900 if the last cell is not occupied
   * returns 1 otherwise
	 */
	virtual double operator()(const Occupancy &vals) const;

	/// Multiply in a DecisionTreeFactor and return the result as DecisionTreeFactor
	virtual gtsam::DecisionTreeFactor operator*(const gtsam::DecisionTreeFactor&) const{
		throw std::runtime_error("operator * not implemented");
	}

	virtual operator gtsam::DecisionTreeFactor() const{
		throw std::runtime_error("operator DecisionTreeFactor not implemented");
	}

  virtual gtsam::DecisionTreeFactor toDecisionTreeFactor() const {
		throw std::runtime_error("toDecisionTreeFactor not implemented");
  }
};
