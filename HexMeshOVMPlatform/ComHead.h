#ifndef _HEXOVM_COMHEAD_H_
#define _HEXOVM_COMHEAD_H_
#include "OVMFileContast.h"
/*
包含一般的引用
*/
#include<iostream>
#include<sstream>
#include<string>
#include<fstream>

/*
包含一般数据结构相关引用
*/
#include<algorithm>
#include<cctype>
#include<vector>
#include<array>
#include<set>
#include<iterator>
#include<map>
#include<cmath>

/*
对第三方库 eigen的引用和opengl的引用
*/

/*
对该OVM库的引用
*/
#include<OpenVolumeMesh\Geometry\VectorT.hh>
#include<OpenVolumeMesh\FileManager\FileManager.hh>
#include<OpenVolumeMesh\Mesh\HexahedralMesh.hh>
#include<OpenVolumeMesh\Mesh\PolyhedralMesh.hh>
#include<OpenVolumeMesh\Mesh\HexahedralMeshTopologyKernel.hh>


//typedef

// Make some typedefs to facilitate your life
typedef OpenVolumeMesh::Geometry::Vec3f         Vec3f;
typedef OpenVolumeMesh::GeometryKernel<Vec3f>   PolyhedralMeshV3f;
typedef OpenVolumeMesh::GeometricHexahedralMeshV3f HexMeshV3f;

typedef OpenVolumeMesh::VertexHandle VertexHandle;
typedef OpenVolumeMesh::HalfEdgeHandle HalfEdgeHandle;
typedef OpenVolumeMesh::EdgeHandle EdgeHandle;
typedef OpenVolumeMesh::HalfFaceHandle HalfFaceHandle;
typedef OpenVolumeMesh::FaceHandle FaceHandle;
typedef OpenVolumeMesh::CellHandle CellHandle;

typedef OpenVolumeMesh::OpenVolumeMeshEdge Edge;

#endif // !_HEXOVM_COMHEAD_H_
