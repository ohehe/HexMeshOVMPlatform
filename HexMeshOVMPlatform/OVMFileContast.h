#ifndef _HEXOVM_OVMFILECONTAST_H_
#define _HEXOVM_OVMFILECONTAST_H_

//*******************************************************************
/*����ovm�ļ���ǵĺ�
*/
#define OVM_FILE_HEADER OVM
#define OVM_FILE_CODE_TYPE ASCII
#define OVM_FIlE_MARK_VERTICES Vertices
#define OVM_FILE_MARK_VERTEX_PROPERTY VProp
#define OVM_FILE_MARK_EDGES Edges
#define OVM_FILE_MARK_EDGE_PROPERTY EProp
#define OVM_FILE_MARK_FACES Faces
#define OVM_FILE_MARK_FACE_PROPERTY FProp
#define OVM_FILE_MARK_HALFFACE_PROPERTY HFProp 
#define OVM_FILE_MARK_POLYHEDRA Polyhedra

//*******************************************************************
/*����ovm�ļ����������͵ĺ�
*/
#define OVM_CONSTANT_TYPE_DOUBLE double
#define OVM_CONSTANT_TYPE_BOOL bool
#define OVM_CONSTANT_TYPE_FLOAT float
#define OVM_CONSTANT_TYPE_INT int




//*******************************************************************
/*
��׼����
*/
#define VERTICE_INNER_REGULAR_COUNT 6
#define EDGE_INNER_REGULAR_COUNT 4
#define VERTICE_BDY_REGULAR_COUNT 5
#define EDGE_BDY_REGULAR_COUNT 3


//*************************************************
//����ʹ���еĶ�Ӧ��ϵ
static const unsigned char sc_diagonal_index[8] = {6,5,4,7,2,1,0,3};

#endif // !_HEXOVM_OVMFILECONTAST_H_
