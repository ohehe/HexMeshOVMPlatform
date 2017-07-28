#ifndef _HEXMESH_OVMVIEWER_H_
#define _HEXMESH_OVMVIEWER_H_

#include "ComHead.h"
#include "glCamera.h"
#include "FaceExpandAndHexFlooding.h"
#include "BaseComplexSimplify.h"

#define M_PI 3.14159265358979323846
#define UNIT 2.00f
#define UNIT_ZOOM 0.5f
//0.3
#define MAXARRAYSUM 100000
typedef HexMeshV3f Mesh;

//*******************************************************************
/*
提供的填色
*/
#define COLOR_BLUE 0x0000FF
#define COLOR_RED  0xFF0000
#define COLOR_AZURE 0xF0FFFF
#define COLOR_GREEN 0x008000
#define COLOR_WHITE 0xFFFFFF

typedef struct V3f
{
public:
	V3f()
	{
		x = 0;
		y = 0;
		z = 0;
	}
	V3f(float x, float y, float z)
	{
		this->x = x;
		this->y = y;
		this->z = z;
	}
	float x, y, z;
}V3f;

//void setupPointersByArray();
void RotateX(float angle);
void RotateY(float angle);
//void drawAllLines();
//void drawAllVertex();
//void drawhex(V3f p0, V3f p1, V3f p2, V3f p3, V3f p4, V3f p5, V3f p6, V3f p7,V3f itemColor, V3f wireColor);
void display();
void display2();
void display3();
void Mouse(int button, int state, int x, int y);
void KeyFunc(unsigned char key, int x, int y);
void onMouseMove(int x, int y);
void setupSingularInit(Mesh *m, bool draw_wire_frame);

//TODO：待会要改的 mesh不需要这样传入
void init_line(Mesh* m, bool draw_wire_frame);
void init_faceset(Mesh& m,std::set<FaceHandle, compare_OVM> f_all_s);
void init_basecomplexset(Mesh &m, std::set<BaseComplex, compare_BaseComplex> &baseComplexSet);
void Init_BC_8_Set(Mesh &showHexMesh, std::vector< std::vector<BC_8> > v_cell_vector);

void reshape(int w, int h);

void setupHalfFaceInit(std::set<BaseComplex, compare_BaseComplex> baseComplex_set);

void setupHalfFaceInit(std::set<HalfFaceHandle, compare_OVM> hf_all_s); 

void setupFaceInit(std::set<FaceHandle, compare_OVM> f_all_s);



void setupBaseComplexSet(std::set<BaseComplex, compare_BaseComplex> &baseComplexSet);
#endif // !_HEXMESH_OVMVIEWER_H_
