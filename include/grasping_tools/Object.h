///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef GPISGRASPING_OBJECT_H_
#define GPISGRASPING_OBJECT_H_

#include <armadillo>
#include <Eigen/Eigen>

namespace grasping_tools {
	/// Parent class from which all kind of objects inherit, such as ObjectMesh, ObjectGpis and ObjectPointCloud
	///
	/// Class defined in `#include <grasping_tools/Object.h>`
	class Object {
	public:
		/// Return the centroid of the object.
		virtual arma::colvec3 center() = 0;

		/// Get the min and max point that encloses boundaries of the object.
		virtual void minMax(arma::colvec3 &_min, arma::colvec&_max) {std::cout << "Not implemented" << std::endl;};

		/// Set a pose to an object.
		void move(arma::mat44 &_pose){
			mPose = _pose*mPose;
			for(auto i = 0; i < 4; i++){
				for(auto j=0; j <4; j++){
					mPoseEigen(i,j) = mPose(i,j);
				}
			}
			moveObject();
		}

		/// Set a pose to an object.
		void move(Eigen::Matrix4f &_pose){
			mPoseEigen = _pose*mPoseEigen;
			for(auto i = 0; i < 4; i++){
				for(auto j=0; j <4; j++){
					mPose(i,j) = mPoseEigen(i,j);
				}
			}
			moveObject();
		}

		/// Get object pose
		void pose(arma::mat44 &_pose){
			_pose = mPose;
		}

		/// Get object pose
		void pose(Eigen::Matrix4f &_pose){
			_pose = mPoseEigen;
		}

	protected:
		virtual void moveObject() = 0;

	protected:
		arma::mat44 	mPose = arma::eye(4,4);
		Eigen::Matrix4f mPoseEigen = Eigen::Matrix4f::Identity();		// Computed once 
	};
}	//	gpisGrasping

#endif	//	GPISGRASPING_OBJECT_H_