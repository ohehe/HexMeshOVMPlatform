#ifndef _HEXMESH_SINGULAROBJECT_H_
#define _HEXMESH_SINGULAROBJECT_H_


#include "ComHead.h"
const unsigned char U_RELOCATED_CELL_OUT_INDEX_[8][3] = {
	{ 0,3,4 },
	{ 0,2,4 },
	{ 0,2,5 },
	{ 0,3,5 },
	{ 1,3,4 },
	{ 1,3,5 },
	{ 1,2,5 },
	{ 1,2,4 }
};
class SingularEdge;
struct compare_OVM_For_Singular
{
	bool operator()(const OpenVolumeMesh::OpenVolumeMeshHandle &a, const OpenVolumeMesh::OpenVolumeMeshHandle &b) const
	{
		return a.idx()<b.idx();
	}
};


//template<class MeshT>
class SingularObject {                                                                                                                                                                                                                           
private :
	HexMeshV3f *mesh;
	//提取的奇异点、边列表
	std::vector<VertexHandle> vector_singular_vertex_handles;
	std::vector<EdgeHandle> vector_singular_edge_handles;
	std::vector<SingularEdge> vector_singular_edges_complex;
	bool B_READY = false;
public :
	
	SingularObject();
	SingularObject(HexMeshV3f* in_mesh);

	~SingularObject();

	std::vector<EdgeHandle> getEdgeVector() {
		return vector_singular_edge_handles;
	}

	std::vector<SingularEdge> getEdgeComplexVector() {
		return vector_singular_edges_complex;
	}
	void FindSingularObject_simple();
	void FindSingularObject(bool b_need_ver, bool b_need_edge);
	void FindSingularObjectForBC(std::vector<SingularEdge>& se_vector, bool b_need_ver, bool b_need_edge);

	void SetColorAttr(int d_color_s_ver, int d_color_o_ver, int d_color_s_edge, int d_color_i_edge);
protected :
	
	//初始化网格
	bool InitMesh();
	 
};
//奇异边操作类
class SingularEdge{
private:
	VertexHandle endPoint_1;
	VertexHandle endPoint_2;
	
	std::set<HalfFaceHandle,compare_OVM_For_Singular> faces_aroud_set_p1;
	std::set<HalfFaceHandle, compare_OVM_For_Singular> faces_aroud_set_p2;

	std::set<CellHandle, compare_OVM_For_Singular> cells_around_set_p1;
	std::set<CellHandle, compare_OVM_For_Singular> cells_around_set_p2;

	bool is_filled_complish = false;
	bool is_entirely_in_bdy = false;
	bool is_loopped = false;
public:
	//按边顺序插入 
	std::vector<VertexHandle> ver_vector;
	std::vector<EdgeHandle> edge_vector; 
	HexMeshV3f* mesh;

	//复制函数
	SingularEdge(const SingularEdge& se) {
		this->endPoint_1 = se.endPoint_1; 
		this->endPoint_2 = se.endPoint_2; 
		
		this->is_entirely_in_bdy = se.is_entirely_in_bdy; 
		this->is_filled_complish = se.is_filled_complish;
		this->mesh = se.mesh;

		//对集合等等的复制

		faces_aroud_set_p1.insert(se.faces_aroud_set_p1.begin(), se.faces_aroud_set_p1.end());
		faces_aroud_set_p2.insert(se.faces_aroud_set_p2.begin(), se.faces_aroud_set_p2.end());

		cells_around_set_p1.insert(se.cells_around_set_p1.begin(), se.cells_around_set_p1.end()); 
		cells_around_set_p2.insert(se.cells_around_set_p2.begin(), se.cells_around_set_p2.end());

		ver_vector.assign(se.ver_vector.begin() , se.ver_vector.end());
		edge_vector.assign(se.edge_vector.begin(), se.edge_vector.end());
		is_filled_complish = true;
		is_loopped = se.is_loopped;
	}

	SingularEdge & operator=(SingularEdge &se) {
		se.endPoint_1 = this->endPoint_1;
		se.endPoint_2 = this->endPoint_2;
		
		se.is_entirely_in_bdy = this->is_entirely_in_bdy;
		se.is_filled_complish = this->is_filled_complish;
		se.mesh = this->mesh;

		//对集合等等的复制

		se.faces_aroud_set_p1.insert(faces_aroud_set_p1.begin(), faces_aroud_set_p1.end());
		se.faces_aroud_set_p2.insert(faces_aroud_set_p2.begin(), faces_aroud_set_p2.end());

		se.cells_around_set_p1.insert(cells_around_set_p1.begin(), cells_around_set_p1.end());
		se.cells_around_set_p2.insert(cells_around_set_p2.begin(), cells_around_set_p2.end());

		se.ver_vector.assign(ver_vector.begin(), ver_vector.end());
		se.edge_vector.assign(edge_vector.begin(), edge_vector.end());
		is_filled_complish = true; 
		is_loopped = se.is_loopped;
		return *this;
	}

	SingularEdge() {
		this->endPoint_1 = HexMeshV3f::InvalidVertexHandle;
		this->endPoint_2 = HexMeshV3f::InvalidVertexHandle;
		mesh = NULL;
		is_filled_complish = false;

	}

	SingularEdge(HexMeshV3f *mesh, std::vector<VertexHandle> ver_vector, std::vector<EdgeHandle> edge_vector,VertexHandle endv1 , VertexHandle endv2,bool is_loop) {
		if (mesh == NULL) {
			this->is_filled_complish = false;
			
			return;
		}
		this->mesh = mesh;
		this->endPoint_1 = endv1; 
		this->endPoint_2 = endv2; 
		if ((ver_vector.size() == 0)||(edge_vector.size()==0)) {
			this->is_filled_complish = false;
			this->endPoint_1 = HexMeshV3f::InvalidVertexHandle;
			this->endPoint_2 = HexMeshV3f::InvalidVertexHandle;
			return;
		}
		
		this->ver_vector.swap(ver_vector);
		this->edge_vector.swap(edge_vector);
		is_loopped = is_loop; 
		//填充边集合
		is_filled_complish = true; 
		//校验
		Get_is_filled_complish(true);
	}

	bool Get_is_loopped() {
		return is_loopped;
	}

	void Set_is_loopped(bool is_loop) {
		is_loopped = is_loop;
	}

	VertexHandle Get_end_point(bool is_v1) {
		return is_v1 ? endPoint_1 : endPoint_2; 
	}

	std::set<HalfFaceHandle, compare_OVM_For_Singular> Get_faces_aroud_set(bool is_v1) {
		return is_v1 ? faces_aroud_set_p1 : faces_aroud_set_p2;
	}
	std::set<CellHandle, compare_OVM_For_Singular> Get_cell_aroud_set(bool is_v1) {
		return is_v1 ? cells_around_set_p1 : cells_around_set_p2;
	}
	std::set<HalfFaceHandle, compare_OVM_For_Singular> FindSeparatedFaceHandles();

	std::set<CellHandle, compare_OVM_For_Singular> FindCellAroundSE();

	unsigned char JudgeVer2CellIndex(CellHandle cell,VertexHandle point);

	void Set_is_filled_complish(bool is) {
		is_entirely_in_bdy = is;
	}

	bool Get_is_filled_complish(bool need_checked) {
		if (need_checked) {
			for (std::vector<EdgeHandle>::iterator edge_iter = edge_vector.begin(); edge_iter != edge_vector.end(); ++edge_iter) {
				//校验一次
				OpenVolumeMesh::OpenVolumeMeshEdge edge = mesh->edge(*edge_iter);
				
				if (!mesh->is_boundary(edge.from_vertex()) || !mesh->is_boundary(edge.to_vertex())) {
					is_entirely_in_bdy = false; 
					return false;
				}
			}
			is_entirely_in_bdy = true;
			return true;
		}
		return is_entirely_in_bdy;
	}
	/*
	bool is_filled_complish = false;
	bool is_entirely_in_bdy = false;
	getter setter
	*/
	bool Get_is_entireely_in_bdy() {
		return this->is_entirely_in_bdy;
	}
};
#endif // !_HEXMESH_SINGULAROBJECT_H_
