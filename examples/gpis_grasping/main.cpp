//---------------------------------------------------------------------------------------------------------------------
//  GRASPING_TOOLS
//---------------------------------------------------------------------------------------------------------------------
//  Copyright 2018 Pablo Ramon Soria (a.k.a. Bardo91) pabramsor@gmail.com
//---------------------------------------------------------------------------------------------------------------------
//  Permission is hereby granted, free of charge, to any person obtaining a copy of this software
//  and associated documentation files (the "Software"), to deal in the Software without restriction,
//  including without limitation the rights to use, copy, modify, merge, publish, distribute,
//  sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in all copies or substantial
//  portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
//  BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
//  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES
//  OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
//  CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//---------------------------------------------------------------------------------------------------------------------

#include <algorithm>
#include <iostream>
#include <thread>
#include <chrono>

#include <pcl/visualization/pcl_visualizer.h>
#include <pcl/io/io.h>
#include <pcl/io/pcd_io.h>
#include <pcl/io/vtk_lib_io.h>
#include <pcl/io/ply_io.h>
#include <pcl/PolygonMesh.h>
#include <pcl/point_types.h>
#include <pcl/common/common.h>
#include <pcl/common/transforms.h>
#include <pcl/filters/voxel_grid.h>

#include <grasping_tools/grasping_tools.h>
#include <grasping_tools/mathTools.h>

#include <gpis/sampler/prior/PriorFactory.h>
#include <gpis/utils/SurfaceGpis.h>

bool rotateGrasp = false;
bool newGrasp = false;
void keycallback(const pcl::visualization::KeyboardEvent &_event, void *_data) {
    if (_event.keyDown() && _event.getKeySym() == "n") {
        newGrasp = true;
    }else if (_event.keyDown() && _event.getKeySym() == "r") {
        rotateGrasp = true;
    }
};

pcl::PolygonMesh gMeshBase;
pcl::PolygonMesh gLeftGripper;
pcl::PolygonMesh gRightGripper;
pcl::PolygonMesh generateHandModel(const grasping_tools::Grasp & _grasp, double _rotation);

int main(int _argc, char** _argv){

    if(_argc != 5){
        std::cout << "Bad input arguments. Usage: ./example_mesh_grasping [resolution] [path to stl mesh file] [path to gripper model] [rot_res]" << std::endl;
        return -1;
    }

    std::string modelFolder = _argv[3];
    
    if (pcl::io::loadPolygonFileSTL(modelFolder + "/base.stl", gMeshBase) == 0) {
        PCL_ERROR("Failed to load STL file\n");
        return -1;
    }
    if (pcl::io::loadPolygonFileSTL(modelFolder + "/leftGripper.stl", gLeftGripper) == 0) {
        PCL_ERROR("Failed to load STL file\n");
        return -1;
    }
    if (pcl::io::loadPolygonFileSTL(modelFolder + "/rightGripper.stl", gRightGripper) == 0) {
        PCL_ERROR("Failed to load STL file\n");
        return -1;
    }

    float resolution = atof(_argv[1]);
    float rotRes = atof(_argv[4]);

    std::cout << "Creating object" << std::endl;
    
    auto prior = gpis::PriorFactory::create(gpis::PriorFactory::eShapeTypes::Constant, 
                                            {1.0}, 
                                            gpis::PriorFactory::eKernelTypes::SquaredExponential, 
                                            {0.1, 700.0, 0.01, 0.05});
	auto gpisMean = prior->mean();
    auto gpisKernel = prior->kernel();
    arma::vec gpisMeanParams = {1.0};
    pcl::PointCloud<pcl::PointNormal> inCloud;
    if (pcl::io::loadPLYFile(_argv[2], inCloud) != 0) {
        std::cerr << "Error loading point cloud of object" << std::endl; 
        return -1;
    }
    pcl::PointCloud<pcl::PointNormal> inCloudFiltered;
    pcl::VoxelGrid<pcl::PointNormal> sor;
    sor.setInputCloud (inCloud.makeShared());
    sor.setLeafSize (0.01f, 0.01f, 0.01f);
    sor.filter (inCloudFiltered);
    std::cout << "Filtered cloud has " << inCloudFiltered.size() << " points." << std::endl;
    arma::mat inData;
    for(auto &p: inCloudFiltered){
        arma::vec6 colDataAux = {
            p.x,
            p.y,
            p.z,
            p.normal_x,
            p.normal_y,
            p.normal_z,
        };
        inData.insert_cols(inData.n_cols, colDataAux); 
    }
    grasping_tools::ObjectGpis object(inData, gpisMean, gpisKernel, gpisMeanParams);
    gpis::SurfaceGpis surface(gpisKernel, gpisMean);
    surface.addData(inData);
    surface.compute(gpisMeanParams, inData.col(0).rows(0,2), 0.003, 10000, 5.0);
    pcl::PolygonMesh mesh;
    surface.mesh(mesh);

    std::cout << "Creating Gripper" << std::endl;
    grasping_tools::GripperHandTwoFingers gripper(1.0);
    boost::shared_ptr<pcl::visualization::PCLVisualizer> viewer (new pcl::visualization::PCLVisualizer ("3D Viewer"));
    viewer->registerKeyboardCallback(keycallback, nullptr);
    viewer->setBackgroundColor(0.5,0.5,0.5);
    viewer->addPointCloud<pcl::PointNormal>(inCloudFiltered.makeShared(), "object");
    viewer->addCoordinateSystem(0.05);
    viewer->addPolygonMesh(mesh, "surface");

    while(true){
        std::cout << "Computing grasp" << std::endl;
        auto grasp = gripper.generate(object);
        
        auto cps = grasp.contactPoints();

        int counter = 0;
        for(auto &cp: cps){
            plotContactPoint(cp, *viewer, 0.1, "cone"+std::to_string(counter++));
        }

        for(float rot = 0; rot < M_PI*2; rot+=rotRes){
            auto handMesh = generateHandModel(grasp, rot);
            viewer->addPolygonMesh(handMesh, "hand");
            while(!rotateGrasp)
                viewer->spinOnce(30,true);

            viewer->removeShape("hand");
            rotateGrasp = false;
        }
    }
    
    viewer->spin();

}

pcl::PolygonMesh generateHandModel(const grasping_tools::Grasp & _grasp, double _rotation) {
    auto cps = _grasp.contactPoints();
    pcl::PointCloud<pcl::PointXYZ> finalMeshCloud;
    pcl::PolygonMesh finalMesh;

    // Compute aperture
    arma::vec vAperture = cps[1].position() - cps[0].position();
    double separation = arma::norm(vAperture);
    pcl::PointCloud<pcl::PointXYZ> cloudGripper1;
    pcl::fromPCLPointCloud2(gLeftGripper.cloud, cloudGripper1);

    pcl::PointCloud<pcl::PointXYZ> cloudGripper2;
    pcl::fromPCLPointCloud2(gRightGripper.cloud, cloudGripper2);

    pcl::PointCloud<pcl::PointXYZ> cloudGripper3;
    pcl::fromPCLPointCloud2(gMeshBase.cloud, cloudGripper3);

    Eigen::Matrix4f t = Eigen::Matrix4f::Identity();
    t(1, 3) = -0.05;
    pcl::transformPointCloud(cloudGripper3, cloudGripper3, t);

    t = Eigen::Matrix4f::Identity();
    t(0, 3) = -separation / 2;
    pcl::transformPointCloud(cloudGripper1, cloudGripper1, t);
    t(0, 3) = separation / 2;
    pcl::transformPointCloud(cloudGripper2, cloudGripper2, t);

    finalMeshCloud.points.insert(finalMeshCloud.end(), cloudGripper1.begin(), cloudGripper1.end());
    finalMesh.polygons.insert(finalMesh.polygons.end(), gLeftGripper.polygons.begin(), gLeftGripper.polygons.end());
    finalMeshCloud.points.insert(finalMeshCloud.end(), cloudGripper2.begin(), cloudGripper2.end());
    finalMesh.polygons.insert(finalMesh.polygons.end(), gRightGripper.polygons.begin(), gRightGripper.polygons.end());
    for (unsigned i = 0; i < gRightGripper.polygons.size(); i++) {
        finalMesh.polygons[i + gLeftGripper.polygons.size()].vertices[0] += gLeftGripper.cloud.width*gLeftGripper.cloud.height;
        finalMesh.polygons[i + gLeftGripper.polygons.size()].vertices[1] += gLeftGripper.cloud.width*gLeftGripper.cloud.height;
        finalMesh.polygons[i + gLeftGripper.polygons.size()].vertices[2] += gLeftGripper.cloud.width*gLeftGripper.cloud.height;
    }

    int prevPoints = finalMeshCloud.size();
    int prevPolygons = finalMesh.polygons.size();
    finalMeshCloud.points.insert(finalMeshCloud.end(), cloudGripper3.begin(), cloudGripper3.end());
    finalMesh.polygons.insert(finalMesh.polygons.end(), gMeshBase.polygons.begin(), gMeshBase.polygons.end());

    for (unsigned i = 0; i < gMeshBase.polygons.size(); i++) {
        finalMesh.polygons[i + prevPolygons].vertices[0] += prevPoints;
        finalMesh.polygons[i + prevPolygons].vertices[1] += prevPoints;
        finalMesh.polygons[i + prevPolygons].vertices[2] += prevPoints;
    }


    // Rotate pointcloud of mesh
    t = Eigen::Matrix4f::Identity();
    auto rot = grasping_tools::rotUnitaryVectors({ 1,0,0 }, arma::vec(cps[1].position() - cps[0].position()));
    t.block<3, 3>(0, 0) = Eigen::Matrix3d(&rot(0, 0)).cast<float>();
    pcl::transformPointCloud(finalMeshCloud, finalMeshCloud, t);

    // Set extra rotation
    t = Eigen::Matrix4f::Identity();
    arma::vec cp1 = cps[0].position();
    arma::vec cp2 = cps[1].position();
    Eigen::Vector3f axis = { float(cp2[0] - cp1[0]) ,float(cp2[1] - cp1[1]),float(cp2[2] - cp1[2]) };
    axis /= axis.norm();
    auto rotExtra = Eigen::AngleAxisf(_rotation, axis)*
                Eigen::AngleAxisf(0, Eigen::Vector3f::UnitX());
    t.block<3, 3>(0, 0) = rotExtra.matrix();
    pcl::transformPointCloud(finalMeshCloud, finalMeshCloud, t);

    // Translate pointcloud of mesh
    t = Eigen::Matrix4f::Identity();
    t(0, 3) = cps[0].position()[0] + vAperture[0]/2;
    t(1, 3) = cps[0].position()[1] + vAperture[1]/2;
    t(2, 3) = cps[0].position()[2] + vAperture[2]/2;
    pcl::transformPointCloud(finalMeshCloud, finalMeshCloud, t);


    pcl::toPCLPointCloud2(finalMeshCloud, finalMesh.cloud);

    return finalMesh;
}