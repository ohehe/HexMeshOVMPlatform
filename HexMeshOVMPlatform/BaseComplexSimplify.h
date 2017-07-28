#ifndef _HEXMESH_BASECOMPLEXSIMPLIFY
#define _HEXMESH_BASECOMPLEXSIMPLIFY
#include "ComHead.h"
#include "FaceExpandAndHexFlooding.h"
//�ο�
//static const unsigned char XF = 0;
//static const unsigned char XB = 1;
//static const unsigned char YF = 2;
//static const unsigned char YB = 3;
//static const unsigned char ZF = 4;
//static const unsigned char ZB = 5;
//static const unsigned char INVALID = 6;
//0-7��Ӧ�ⲿ����
static const unsigned char U_RELOCATED_CELL_OUT_INDEX[8][3] = {
	{ 0,3,4 },
	{ 0,2,4 },
	{ 0,2,5 },
	{ 0,3,5 },
	{ 1,3,4 },
	{ 1,3,5 },
	{ 1,2,5 },
	{ 1,2,4 }
};
//0-7��Ӧ�ڲ�����
static const unsigned char U_RELOCATED_CELL_INDEX[8][3] = {
	{1,2,5},
	{1,3,5},
	{1,3,4},
	{1,2,4},
	{0,2,5},
	{0,2,4},
	{0,3,4},
	{0,3,5}
};
//��4���͵�����˳�� ���xf-��xb yf-��yb zf-��zb x->y->z
//��������㿪ʼ˳ʱ�뻷��
static const unsigned char ORI_FACES_VER[6][4] = {
	{3,2,1,0},
	{7,6,5,4},
	{1,2,6,7},
	{4,5,3,0},
	{1,7,4,0},
	{2,3,5,6}
};

//��ʱû���õ�
//��bc4�����ĸ������ϵ�Ԫ�İ���
static const unsigned char ORI_CELLS_FOR_4[6][6*2] = {
	{-1,-1,-1,-1,1,2,0,3,1,0,2,3},
	{-1,-1,-1,-1,7,6,4,5,7,4,6,5},
	{1,2,7,6,-1,-1,-1,-1,1,7,2,6},
	{0,3,4,5,-1,-1,-1,-1,3,5,0,4},
	{ 1,0,7,4,1,7,0,4,-1,-1,-1,-1 },
	{ 2,3,6,5,2,6,3,5,-1,-1,-1,-1 }
};

class BC_8
{
public:
	//���ｲ��������Ҫ��ƴ��λ��� �Ͱ��տ��Ҫ���˳��ṹ����
	CellHandle *bcs = NULL;
	//�ع���İ˸�����λ��
	VertexHandle *vs = NULL;
	//��4���͵� ����  Ҳ��Ӧ��ORI_FACES_VER�з����cell����
	unsigned char orientation = INVALID_ORIENTATION;
private:
	//����
	int pos = 0; 
	bool init = false;
	bool is_type_8 = true;
public:
	BC_8(){
		bcs = new CellHandle[8];
		vs = new VertexHandle[8];
		init = false;
		//�˵�Ԫ �ĵ�Ԫ��ʶ
		is_type_8 = true;
	}
	BC_8(bool is_type_8) {
		bcs = new CellHandle[8];
		vs = new VertexHandle[8];
		init = false;
		this->is_type_8 = is_type_8;
	}
	BC_8(const BC_8& b){
		//���� �����°� ��ֹԭ����������
		//this->bcs = b.bcs;

		
		//���·���ռ�

		this->bc_deleteAll(b.is_type_8);
		
		for (int i = 0; i < 8; ++i) {
			bcs[i] = b.bcs[i];
			vs[i] = b.vs[i]; 
			
		}
		//����������ķ���

		pos = b.pos;
		init = b.init;
		is_type_8 = b.is_type_8;
		if (!is_type_8) {
			this->orientation = b.orientation;
		}
	}
	BC_8& operator=(const BC_8&b) {
		if (bcs != NULL) {
			this->bc_deleteAll(b.is_type_8);
		}
		for (int i = 0; i < 8; ++i) {
			bcs[i] = b.bcs[i];
			vs[i] = b.vs[i];
		}
		pos = b.pos;
		init = b.init;
		is_type_8 = b.is_type_8;
		if (!is_type_8) {
			this->orientation = b.orientation;
		}
	}
	//��Ӧ���б�����ĸ�����
	std::pair<int,int> bc_find2_in_ori(unsigned char ori_4) {
		if ((ori_4%2?ori_4-1:ori_4) != (this->orientation%2?this->orientation-1:this->orientation )) {
			return std::make_pair(ORI_CELLS_FOR_4[orientation][2*ori_4], ORI_CELLS_FOR_4[orientation][1+2 * ori_4]);
			
		}
	}
	void bc_setType(bool is_type_8,bool need_clear) {
		if (this->is_type_8 != is_type_8) {
			if (need_clear) {
				this->clearElem();
				/*pos = 0;
				init = false;*/
			}
		}
		this->is_type_8 = is_type_8;
		if (is_type_8 ? pos == 8 : pos == 4) {
			init = true;
		}
		else {
			init = false;
		}
	}
	const unsigned char getOriFor4() {
		if (!this->is_type_8) {
			return orientation;
		}
		else {
			return INVALID_ORIENTATION;
		}
	}

	const int getPos() {
		return pos; 
	}
	bool isType8() {
		return is_type_8;
	}
	//Ŀǰ�ǰ�ֱ�Ӳ����  ������ɺ��ж��Ƿ�ɹ���ʼ�����
	bool bc_insertCell(CellHandle cell) {
		
		if (cell != HexMeshV3f::InvalidCellHandle) {
			if (pos > -1 && pos < (is_type_8 ? 8 : 4)) {
				this->bcs[++pos - 1] = cell;
				if (is_type_8 ? pos == 8 : pos == 4) {
					init = true;
				}
				return true;
			}
		}
		return false;
	}
	//�������
	bool bc_insertCellArr(CellHandle * cell_arr,size_t arr_size) {
		if (is_type_8 ? arr_size != 8 : arr_size != 4) {
			return false;
		}
		for (size_t i = 0; i < arr_size; ++i) {
			if (cell_arr + i != NULL && *(cell_arr + i) != HexMeshV3f::InvalidCellHandle) {
				this->bcs[i] = *(cell_arr + i);
			}
			else {
				init = false;
				bc_deleteAll(is_type_8);
				return false; 
			}
		}
		init = true;
		return true; 
	}

	//�Ժ���ӵİ汾
	bool bc_insertCell(CellHandle cell,const unsigned char index_orientation) {
		//TODO
	}

	//û��չ�ϵҲ����
	void clearElem() {
		if (bcs != NULL&&vs!=NULL) {
			for (uint16_t i = 0; i < 8; ++i) {
				bcs[i] = HexMeshV3f::InvalidCellHandle;
				vs[i] = HexMeshV3f::InvalidVertexHandle;
			}
			init = false;
			pos = 0; 
		}
		
	}

	bool bc_deleteAll(bool is_type_8) {
		if (bcs != NULL) {
			delete[] bcs;
			bcs = NULL;
		}
		if (vs != NULL) {
			delete[] vs;
			vs = NULL;
		}
		bcs = new CellHandle[8];
		vs = new VertexHandle[8];
		init = false;
		pos = 0;
		return true;
	}
	bool bc_is_Init() {
		//������
		if (pos == 8&&is_type_8||pos == 4&&!is_type_8) {
			init = true; 
		}
		else {
			init = false;
		}
		return init; 
	}
	void setConnernVer(unsigned char index,const VertexHandle ve) {
		if(vs)
			this->vs[index] = ve;
	}
	CellHandle getCellHandle(int index) {
		return index > -1 && index < 8 ? *(bcs + index) : HexMeshV3f::InvalidCellHandle;

	}

	bool includeCell(CellHandle cell) {
		for (int i = 0; i < 8; ++i) {
			if (cell.idx() == bcs[i].idx()) {
				return true;
			}
		}
		return false;
	}

	void bc_relocated_8(HexMeshV3f * mesh, VertexHandle middle_vertex);
	void bc_relocated_8(HexMeshV3f * mesh );
	void relocated_complex_real(HexMeshV3f * mesh,unsigned char* index);
	/*
		
	*/
	void bc_relocated_4(HexMeshV3f * mesh, BaseComplex& bc);

	//У���Ƿ����غϵ�
	bool check_relocated() {
		int i, j; 
		for (i = 0; i < 8; ++i) {
			if (-1 == vs[i].idx()) {
				break;
			}
			for (j = i+1; j < 8; ++j) {
				if (vs[i].idx() == vs[j].idx()) {
					goto break2out ;
				}
			}
		}
		//�������ظ�
		for (int i = 0; i < pos; i++) {
			for (int j = i + 1; j < pos; j++) {
				if (bcs[j].idx() == bcs[i].idx()) return false;
			}
		}
		break2out:
		if (i < 7) {
			return false;
		}
		else {
			return true;
		}
	}


	~BC_8()
	{
		
		if (bcs != NULL) {
			delete[] bcs;
			bcs = NULL;
		}
		if (vs != NULL) {
			delete[] vs;
			vs = NULL;
		}
	}
};

class BaseComplexSimplify {

private:
	HexMeshV3f * mesh; 
	
	//��ָ��漯��
	std::set<HalfFaceHandle, compare_OVM> hf_all_set;
	//�ָ��漯��
	std::set<FaceHandle, compare_OVM> f_all_set;

	//�ָ��bc����
	std::set<BaseComplex, compare_BaseComplex> bc_set;
public:
	enum  simplify_type
	{
		re_8_1,
		re_4_1,
		re_2_1
	};

	BaseComplexSimplify(HexMeshV3f * mesh, std::set<HalfFaceHandle, compare_OVM> hf_all , std::set<FaceHandle, compare_OVM> f_all, std::set<BaseComplex, compare_BaseComplex> baseComplexSet);

	//�����������
	std::vector< std::vector<BC_8> > HandleBCSimplify(simplify_type type,simplify_type bdy_type);

private:
	//����basecomplex �� ������־λ ��ͬһ��bc�еĵ�Ԫ���еݹ�ϲ�
	void HandlerBCSimplify_OneBC(BaseComplex & bc, OpenVolumeMesh::VertexPropertyT<bool>& boolVexChecked,OpenVolumeMesh::CellPropertyT<bool>& boolCellChecked,std::vector<BC_8>* vector_bc8, BC_8 next_8_cell, const unsigned char from_oriention);

	
};

#endif // !_HEXMESH_BASECOMPLEXSIMPLIFY
