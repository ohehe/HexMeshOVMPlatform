#ifndef _HEXMESH_GLCAMERA_H_

#define _HEXMESH_GLCAMERA_H_

//一般引用
#include"ComHead.h"

//第三方库eigen和opengl的引用
#include <GL/glut.h>
#include <GL/GLU.h>
#include "eigen.h"

using namespace Eigen;

class GLCamera
{
public:
	GLCamera();
	GLCamera(const Vector3d & pos, const Vector3d & target, const Vector3d & up);
	void setModelViewMatrix();			// 加载当前MV矩阵
	void setShape(float viewAngle, float aspect, float Near, float Far);		// 设置摄像机的视角
	void slide(float du, float dv, float dn);
	void roll(float angle);
	void yaw(float angle);
	void pitch(float angle);
	float getDist();

private:
	Vector3d m_pos;
	Vector3d m_target;
	Vector3d m_up;
	Vector3d u, v, n;
};

#endif // !_HEXMESH_GLCAMERA_H_
