/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////
////
////
////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef GPISGRASPING_OBJECTS_OBJECTMESH_H_
#define GPISGRASPING_OBJECTS_OBJECTMESH_H_

#include <grasping_tools/Object.h>

#include <pcl/PolygonMesh.h>
#include <pcl/point_cloud.h>
#include <pcl/point_types.h>
#include <pcl/kdtree/kdtree_flann.h>

#include <vector>
//#include <pcl/visualization/pcl_visualizer.h>


namespace grasping_tools {
	/// Class that inherit from Object parent class. This kind of objects uses a mesh as model for object's surface
	///
	/// Class defined in `#include <grasping_tools/objects/ObjectMesh.h>`
	class ObjectMesh: public Object {
	public:
		/// Construct object from a file.
		/// \param _filename: path to file.
		ObjectMesh(std::string _filename);

		/// Construct object from vertices and faces indices.
        ObjectMesh(pcl::PointCloud<pcl::PointNormal> &_vertices, std::vector<pcl::Vertices> &_faces);

		/// Return centroid 3D center of object.
		arma::colvec3 center();

		/// Get the min and max point that encloses boundaries of the object.
		virtual void minMax(arma::colvec3 &_min, arma::colvec&_max);

		/// Intersect a directed ray with the mesh.
		/// \param _initpoint: point that defines the ray position.
		/// \param _dir: vector that defines the direction of the ray.
		arma::colvec3 intersect(arma::colvec3 _initPoint, arma::colvec3 _dir);

		/// Intersect a directed ray with the mesh.
		/// \param _initpoint: point that defines the ray position.
		/// \param _dir: vector that defines the direction of the ray.
		arma::mat intersectRay(arma::colvec3 _p1, arma::colvec3 _p2);

		/// Get pointcloud containing the centroid of the faces for any purpose.
		/// \return: centroid of faces.
		arma::mat centroidFaces();

		/// Get closest point on the mesh surface from a given point.
		/// \param _p: given point.
		/// \return: closest point to given point.
        pcl::PointNormal closestVertex(const pcl::PointNormal &_p);

		/// Get closest point on the mesh surface from a given point.
		/// \param _p: given point.
		/// \return: id of closest point to given point.
        int closestVertexId(const pcl::PointNormal &_p);

		/// Get vertex point by id
		/// \param id
		/// \return point
        pcl::PointNormal vertex(int _id);

        /// Get mesh info
        void mesh(pcl::PointCloud<pcl::PointNormal> &_vertices, std::vector<pcl::Vertices> &_faces);

		/// Get mesh info
		template<typename PointType_>
        void vertices(pcl::PointCloud<PointType_> &_vertices){
			for(auto &p:mVertices){
				PointType_ point;
				point.x = p.x;
				point.y = p.y;
				point.z = p.z;
				_vertices.push_back(point);
			}
		}

        /// Get mesh info
        void mesh(pcl::PolygonMesh &_mesh);

	protected:
		virtual void moveObject();

	private:
        pcl::PointCloud<pcl::PointNormal> mVertices;
		std::vector<pcl::Vertices> mFaces;
        pcl::PolygonMesh mMesh;
		arma::mat mCentroidFaces;
		arma::colvec3 mCentroid;
		bool mIsKdtreeInit = false;
        pcl::KdTreeFLANN<pcl::PointNormal> mKdTree;

		pcl::PointNormal mMinPoint, mMaxPoint;

        //pcl::visualization::PCLVisualizer viewer;
        //bool mStop = false;
        //void callbackKeyboard3dViewer(const pcl::visualization::KeyboardEvent & _event, void * _ptrViewer) ;

	};
}	//	gpisGrasping

#endif	//	GPISGRASPING_OBJECT_H_
