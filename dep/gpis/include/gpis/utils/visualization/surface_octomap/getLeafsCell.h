///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef GPIS_UTILS_VISUALIZATION_SURFACEOCTOMAP_GETLEAFSCELL_H_
#define GPIS_UTILS_VISUALIZATION_SURFACEOCTOMAP_GETLEAFSCELL_H_

#include <gpis/utils/visualization/surface_octomap/GpisCell.h>

#include <armadillo>

#include <pcl/point_cloud.h>
#include <pcl/point_types.h>

namespace gpis {
	void getLeafsCell(const GpisCell &_cell, arma::mat &_cloud);

	template<typename PointType_>
	void getLeafsCell(const GpisCell &_cell, pcl::PointCloud<PointType_> &_cloud);

}

#include <gpis/utils/visualization/surface_octomap/getLeafsCell.inl>

#endif	//	GPIS_UTILS_VISUALIZATION_SURFACEOCTOMAP_GETLEAFSCELL_H_


