#include "OccupancyGrid/OccupancyGrid.h"
#include "OccupancyGrid/MCMC.h"
#include "OccupancyGrid/visualiser.h"
#include "OccupancyGrid/loadData.h"

using namespace std;
using namespace gtsam;
Visualiser global_vis_;

int main(int argc, char *argv[]){

	if(argc != 4){
		printf("ERROR [USAGE]: executable width height resolution");
		exit(1);
	}
	double width 		=	atof(argv[1]); 		//meters
	double height 		= 	atof(argv[2]); 		//meters
	double resolution 	= 	atof(argv[3]); 	//meters
	OccupancyGrid occupancyGrid(width, height, resolution); //default center to middle

  global_vis_.init(occupancyGrid.height(), occupancyGrid.width());
  vector<Pose2> allposes;
  vector<double> allranges;
  double max_dist;
  // loadDataFromTxt(
  //     "Data/SICK_Odometry.txt",
  //     "Data/SICK_Snapshot.txt",
  //     allposes, allranges,
  //     max_dist);
  loadPlayerSim(
      "Data/player_sim/laser_pose_all.bin",
      "Data/player_sim/laser_range_all.bin",
      "Data/player_sim/scan_angles_all.bin",
      allposes, allranges, max_dist);
  for (size_t i = 0; i < allranges.size(); i++) {
    const Pose2& pose = allposes[i];
    const double range = allranges[i];
    // this is where factors are added into the factor graph
    occupancyGrid.addLaser(pose, range); //add laser to grid
  }

  occupancyGrid.saveLaser("Data/lasers.lsr");
  occupancyGrid.saveHeatMap("Data/HeatMap.ppm");

	//run metropolis
	OccupancyGrid::Marginals occupancyMarginals = runDDMCMC(occupancyGrid, 20000);

	char marginalsOutput[1000];
			sprintf(marginalsOutput, "Data/DDMCMC_Marginals.txt");
	FILE* fptr = fopen(marginalsOutput, "w");
	fprintf(fptr, "%lu %lu\n", occupancyGrid.width(), occupancyGrid.height());

	for(size_t i = 0; i < occupancyMarginals.size(); i++){
		fprintf(fptr, "%lf ", occupancyMarginals[i]);
	}

	fclose(fptr);
}


