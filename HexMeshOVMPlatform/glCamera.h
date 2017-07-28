#ifndef _HEXMESH_GLCAMERA_H_

#define _HEXMESH_GLCAMERA_H_

//һ������
#include"ComHead.h"

//��������eigen��opengl������
#include <GL/glut.h>
#include <GL/GLU.h>
#include "eigen.h"

using namespace Eigen;

class GLCamera
{
public:
	GLCamera();
	GLCamera(const Vector3d & pos, const Vector3d & target, const Vector3d & up);
	void setModelViewMatrix();			// ���ص�ǰMV����
	void setShape(float viewAngle, float aspect, float Near, float Far);		// ������������ӽ�
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
