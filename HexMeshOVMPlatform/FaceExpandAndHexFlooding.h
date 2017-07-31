#ifndef _HEXMESH_FACEEXPANDANDHEXFLOODING
#define _HEXMESH_FACEEXPANDANDHEXFLOODING
#include"ComHead.h"
#include "SingularObject.h"

//ָ����xf��xb�ȵȷ���
const static unsigned char con_orientations[6] = { 0,1,2,3,4,5 };
#define INVALID_ORIENTATION 6

struct compare_OVM
{
	bool operator()(const OpenVolumeMesh::OpenVolumeMeshHandle &a, const OpenVolumeMesh::OpenVolumeMeshHandle &b) const
	{
		return a.idx()<b.idx();
	}
};




/*
BaseComplex������֧�ֵ��Ż�����
*/
class BaseComplex {
private:
	//�㼯 (�߽�)
	std::set<VertexHandle, compare_OVM> complex_ves;
	//�㼯 ���ڲ���
	std::set<VertexHandle, compare_OVM> complex_ves_inner; 
	//�Ա߽�������
	std::set<HalfFaceHandle, compare_OVM> complex_hfs;
	//�߽�߼�
	std::set<EdgeHandle, compare_OVM> complex_seds;
	//��Ԫ��
	std::set<CellHandle, compare_OVM> complex_hexcells;

	//�ϲ���ʼ��Ԫ
	CellHandle origination_ve = HexMeshV3f::InvalidCellHandle;
	//��ʼ�ϲ���Ԫλ��
	unsigned char origination_index = 8;
	int idx = 0;
public:
	//���ƺ���
	//����=
	BaseComplex& operator=(const BaseComplex&base_complex) {
		this->complex_hexcells.swap(std::set<CellHandle, compare_OVM>());
		this->complex_hfs.swap(std::set<HalfFaceHandle, compare_OVM>());
		this->complex_seds.swap(std::set<EdgeHandle, compare_OVM>());
		this->complex_ves.swap(std::set<VertexHandle, compare_OVM>());
		this->complex_ves_inner.swap(std::set<VertexHandle, compare_OVM>());
		
		idx = base_complex.idx;
		origination_ve = base_complex.origination_ve;
		origination_index = base_complex.origination_index;

		this->complex_hexcells.insert(base_complex.complex_hexcells.begin(), base_complex.complex_hexcells.end());
		this->complex_hfs.insert(base_complex.complex_hfs.begin(), base_complex.complex_hfs.end());
		this->complex_seds.insert(base_complex.complex_seds.begin(), base_complex.complex_seds.end());
		this->complex_ves.insert(base_complex.complex_ves.begin(), base_complex.complex_ves.end());
		this->complex_ves_inner.insert(base_complex.complex_ves_inner.begin(), base_complex.complex_ves_inner.end());
		return *this;
	}

	BaseComplex(const BaseComplex& base_complex) {
		this->complex_hexcells.swap(std::set<CellHandle, compare_OVM>());
		this->complex_hfs.swap(std::set<HalfFaceHandle, compare_OVM>());
		this->complex_seds.swap(std::set<EdgeHandle, compare_OVM>());
		this->complex_ves.swap(std::set<VertexHandle, compare_OVM>());
		this->complex_ves_inner.swap(std::set<VertexHandle, compare_OVM>());

		idx = base_complex.idx;
		origination_ve = base_complex.origination_ve;
		origination_index = base_complex.origination_index;


		this->complex_hexcells.insert(base_complex.complex_hexcells.begin(), base_complex.complex_hexcells.end());
		this->complex_hfs.insert(base_complex.complex_hfs.begin(), base_complex.complex_hfs.end());
		this->complex_seds.insert(base_complex.complex_seds.begin(), base_complex.complex_seds.end());
		this->complex_ves.insert(base_complex.complex_ves.begin(), base_complex.complex_ves.end());
		this->complex_ves_inner.insert(base_complex.complex_ves_inner.begin(), base_complex.complex_ves_inner.end());
	}
	BaseComplex();
	BaseComplex(std::set<VertexHandle, compare_OVM> complex_ves,
		std::set<HalfFaceHandle, compare_OVM> complex_hfs,
		std::set<EdgeHandle, compare_OVM> complex_seds,
		std::set<CellHandle, compare_OVM> complex_hexcells);

	void setIdx(const int& x);
	const int& Idx()const;
	bool insertVertex(VertexHandle ve,bool is_inner);
	bool insertHalfFace(HalfFaceHandle hf);
	bool insertEdge(EdgeHandle ed);
	bool insertCellHandle(CellHandle ce);

	std::set<HalfFaceHandle, compare_OVM> getHalfFaceSet() {
		return complex_hfs;
	}
	std::set<CellHandle, compare_OVM> getCellSet() {
		return complex_hexcells;
	}
	std::set<EdgeHandle, compare_OVM> getSedSet() {
		return complex_seds;
	}
	std::set<VertexHandle, compare_OVM> getVes() {
		return complex_ves;
	}
	std::set<VertexHandle, compare_OVM> getVesInner() {
		return complex_ves_inner;
	}
	//get set origination_ve
	void set_origination_cell(CellHandle cell) {
		origination_ve = cell;
	}

	CellHandle get_origination_cell() {
		return origination_ve;
	}

	//get set origination_index
	void set_origination_index(unsigned char origination_index) {
		this->origination_index = origination_index;
	}

	unsigned char get_origination_index() {
		return origination_index;
	}

	bool find_cell(CellHandle cell) {
		if (complex_hexcells.find(cell) == complex_hexcells.end()) {
			return false;
		}
		return true;
	}
};
struct compare_BaseComplex
{
	bool operator()(const BaseComplex &a1, const BaseComplex &b1) const
	{
		return a1.Idx()<b1.Idx();
	}
};


/*
�ò�����Ҫ����
1���Էָ����̰�Ĳ���
2���Էָ���Ϊ�߽��������OVM��HEX֧�� ʵ���ڸ��������ϵķ������
3���������Ĳ��ֺ�����ṹ����Ϊ���ɲ�������Ľṹ����
*/
class BaseComplexSetFiller {
private:
	HexMeshV3f *mesh;
	
	//��ʱ��cell���ϴ洢 
	std::set<CellHandle, compare_OVM> cell_temp_set;

	//�޷������չ�����
	int GetSFaceExtend_Simple(const HalfEdgeHandle& he, const HalfFaceHandle& hf, OpenVolumeMesh::EdgePropertyT<int> &intEProp_sigular);

	//��ȷ�����ȷ������İ�����չ
	int GetSFaceExtendedOfOrientation_Opened(const HalfEdgeHandle& he , const HalfFaceHandle& hf,
		OpenVolumeMesh::HalfEdgePropertyT<bool> &boolHEProp_Checked, OpenVolumeMesh::HalfFacePropertyT<bool> &boolHFProp_Checked,
		OpenVolumeMesh::EdgePropertyT<int> &intEProp_sigular);

	int GetSFaceExtendedOfOrientation_Closed(const HalfEdgeHandle& he, const HalfFaceHandle& hf,
		OpenVolumeMesh::HalfEdgePropertyT<bool> &boolHEProp_Checked, OpenVolumeMesh::HalfFacePropertyT<bool> &boolHFProp_Checked,
		OpenVolumeMesh::EdgePropertyT<int> &intEProp_sigular);
public:
	//��ָ��漯��
	std::set<HalfFaceHandle, compare_OVM> hf_all_set;
	//�ָ��漯��
	std::set<FaceHandle, compare_OVM> f_all_set;
	BaseComplexSetFiller(HexMeshV3f* hexMesh);
	//̰���������зָ��沢���ظ���
	int SFaceSetSeeking(bool is_support_halfface); 

	int SFaceSetSeeking_Simple(bool is_support_halfface); 

	int SFaceSetSeeking_Complex(bool is_support_halfface,const std::vector<SingularEdge>& vector_se);

	//�����������
	void HexCellFlooding(const unsigned char from_oriention, OpenVolumeMesh::CellPropertyT<bool>& boolCellChecked,const CellHandle& initHandle=NULL);

	void OneBaseComplexFilling(std::vector<SingularEdge> se_vector,std::set<BaseComplex,compare_BaseComplex>& baseComplexSet ,const CellHandle& initHandle = NULL);


	//�޷��������������
	//void HexCellFlooding_N_O(OpenVolumeMesh::CellPropertyT<bool>& boolCellChecked, const CellHandle& initHandle);



	void InsertObject2BaseComplex(BaseComplex& baseComplex,std::set<VertexHandle, compare_OVM>& surface_vex_set);
};


 
#endif // !_HEXMESH_FACEEXPANDANDHEXFLOODING
