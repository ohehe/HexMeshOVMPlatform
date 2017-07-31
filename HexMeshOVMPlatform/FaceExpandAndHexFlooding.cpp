#include"FaceExpandAndHexFlooding.h"



BaseComplex::BaseComplex() {
	origination_ve = HexMeshV3f::InvalidCellHandle;
}
BaseComplex::BaseComplex(std::set<VertexHandle, compare_OVM> complex_ves,
	std::set<HalfFaceHandle, compare_OVM> complex_hfs,
	std::set<EdgeHandle, compare_OVM> complex_seds,
	std::set<CellHandle, compare_OVM> complex_hexcells) {
	this->complex_ves = complex_ves; 
	this->complex_hexcells = complex_hexcells;     
	this->complex_hfs = complex_hfs;
	this->complex_seds = complex_seds;
	
}

bool BaseComplex::insertVertex(VertexHandle ve,bool is_inner) {
	if (is_inner) {
		return this->complex_ves_inner.insert(ve).second;
	}
	else {
		return this->complex_ves.insert(ve).second;
	}
	
}
bool BaseComplex::insertHalfFace(HalfFaceHandle hf) {
	return this->complex_hfs.insert(hf).second;
}
bool BaseComplex::insertEdge(EdgeHandle ed) {
	return this->complex_seds.insert(ed).second;
}
bool BaseComplex::insertCellHandle(CellHandle ce) {
	return this->complex_hexcells.insert(ce).second;
}
void BaseComplex::setIdx(const int& x) {
	this->idx = x; 
}
const int& BaseComplex::Idx() const{
	return this->idx; 
	
}

BaseComplexSetFiller::BaseComplexSetFiller(HexMeshV3f* hexMesh) {
	this->mesh = hexMesh;
}
//贪心搜索所有分割面并返回个数
//是否支持半边集合 (暂时未写)
int BaseComplexSetFiller::SFaceSetSeeking(bool is_support_halfface) {
	if (!is_support_halfface) {
		return 0;
	}
	//获取填充了的属性
	OpenVolumeMesh::EdgePropertyT<int> intEProp_sigular = this->mesh->request_edge_property<int>("SingularEdgeIndex");
	
	int count_sface = 0;
	//添加半边和面遍历的标记属性
	OpenVolumeMesh::HalfEdgePropertyT<bool> boolHEProp_Checked = this->mesh->request_halfedge_property<bool>("IsChecked_HE");
	OpenVolumeMesh::HalfFacePropertyT<bool> boolHFProp_Checked = this->mesh->request_halfface_property<bool>("IsChecked_HF");

	for (OpenVolumeMesh::EdgeIter ed_it = mesh->edges_begin(); ed_it != mesh->edges_end(); ++ed_it) {
		if (boolHEProp_Checked[mesh->halfedge_handle(*ed_it, 0)] && boolHEProp_Checked[mesh->halfedge_handle(*ed_it, 1)]) {
			continue;
		}
		if (intEProp_sigular[*ed_it] != 0) {//是奇异边
			if (intEProp_sigular[*ed_it] > 0) {//opened型的奇异边
				//对相同序号的遍历
				for (OpenVolumeMesh::EdgeIter ed_iter = mesh->edges_begin(); ed_iter != mesh->edges_end(); ++ed_iter) {
					if (intEProp_sigular[*ed_it] != intEProp_sigular[*ed_iter]) {
						continue;
					}
					//对半边进行两次处理 
					//测了 不用 邻接面的共享半边
					/*
					待调试····是不是按照半边找邻接面的 还是说面迭代器绑在边上的
					*/
					if (!boolHEProp_Checked[mesh->halfedge_handle(*ed_it, 0)]) {//未遍历过
						boolHEProp_Checked[mesh->halfedge_handle(*ed_it, 0)] = true;
						for (OpenVolumeMesh::HalfEdgeHalfFaceIter he_hf_iter = mesh->halfedge_halffaces(mesh->halfedge_handle(*ed_it, 0)).first;
							he_hf_iter != mesh->halfedge_halffaces(mesh->halfedge_handle(*ed_it, 0)).second; ++he_hf_iter) {
							count_sface += this->GetSFaceExtendedOfOrientation_Opened(mesh->halfedge_handle(*ed_it, 0),
								*he_hf_iter, boolHEProp_Checked, boolHFProp_Checked, intEProp_sigular);
						}
					}
					
				}
				
				
			}
			else {//closed型的奇异边
				for (OpenVolumeMesh::EdgeIter ed_iter = mesh->edges_begin(); ed_iter != mesh->edges_end(); ++ed_iter) {
					if (intEProp_sigular[*ed_it] != intEProp_sigular[*ed_iter]) {
						continue;
					}
					//对半边进行两次处理 
					//可以分一下 
					
					if (!boolHEProp_Checked[mesh->halfedge_handle(*ed_it, 0)]) {//未遍历过
						boolHEProp_Checked[mesh->halfedge_handle(*ed_it, 0)] = true;
						for (OpenVolumeMesh::HalfEdgeHalfFaceIter he_hf_iter = mesh->halfedge_halffaces(mesh->halfedge_handle(*ed_it, 0)).first;
							he_hf_iter != mesh->halfedge_halffaces(mesh->halfedge_handle(*ed_it, 0)).second; ++he_hf_iter) {
							count_sface += this->GetSFaceExtendedOfOrientation_Closed(mesh->halfedge_handle(*ed_it, 0),
								*he_hf_iter, boolHEProp_Checked, boolHFProp_Checked, intEProp_sigular);
						}
					}
					
				}
			}
			
		}
		else {//分开是有意义的  提前置true会使下层的遍历出错
			boolHEProp_Checked[mesh->halfedge_handle(*ed_it, 0)] = true; 
			boolHEProp_Checked[mesh->halfedge_handle(*ed_it, 1)] = true;
		}

	}
	return count_sface;
}
//******************************************
//1.对所有奇异边的邻接面延伸。
//	(1)保存能延伸到另一条奇异边的分割面部分,保存该奇异边集合。
//  (2)保存能直接延伸到边界且不与当前分割面集合相交的分割面部分，保存该奇异边集合。
//	(3)处理剩下的延伸面，到达当前分割面集合中单元停止，保存入分割面集合。
int BaseComplexSetFiller::SFaceSetSeeking_Complex(bool is_support_halfface,const std::vector<SingularEdge>& vector_se) {
	uint64_t count_step_direct = 0u;
	uint64_t count_step_to_bdy = 0u;
	uint64_t count_step_to_existing_sf = 0u;

	//获取标记
	OpenVolumeMesh::EdgePropertyT<int> intEProp_sigular = this->mesh->request_edge_property<int>("SingularEdgeIndex");

	//TODO:step-1
	for (OpenVolumeMesh::EdgePropertyT<int>::iterator e_p_iter = intEProp_sigular.begin(); e_p_iter != intEProp_sigular.end(); ++e_p_iter) {
		if (!(*e_p_iter)) {//非奇异边跳过
			continue;
		}
		

	}
	//TODO:step-2

	//TODO:step-3
	return 0;
}
int BaseComplexSetFiller::SFaceSetSeeking_Simple(bool is_support_halfface)
{
	
	uint64_t count_n = 0;
	if (!is_support_halfface) { return 0; }

	//获取填充了的属性
	OpenVolumeMesh::EdgePropertyT<int> intEProp_sigular = this->mesh->request_edge_property<int>("SingularEdgeIndex");

	for (OpenVolumeMesh::HalfEdgeIter hf_iter = mesh->halfedges_begin(); hf_iter != mesh->halfedges_end(); ++hf_iter) {
		if (intEProp_sigular[mesh->edge_handle(*hf_iter)]==0) continue;
		std::cout << (*hf_iter).idx()<<"---";
		std::pair<OpenVolumeMesh::HalfEdgeHalfFaceIter, OpenVolumeMesh::HalfEdgeHalfFaceIter> pair = mesh->halfedge_halffaces(*hf_iter);
		for (OpenVolumeMesh::HalfEdgeHalfFaceIter he_hf_iter = pair.first; he_hf_iter != pair.second; ++he_hf_iter) {
			count_n += GetSFaceExtend_Simple(*hf_iter, *he_hf_iter, intEProp_sigular);
		}
		
	}
	return count_n;
}

void ShowNowTemp(const HexMeshV3f& m, HalfEdgeHandle he, HalfFaceHandle hf) {
	OpenVolumeMesh::HalfFaceVertexIter a = m.halfface_vertices(hf).first;
	OpenVolumeMesh::HalfEdgeHalfFaceIter it = m.halfedge_halffaces(he).first;
	while (it++!= m.halfedge_halffaces(he).second) {
		std::cout << 1; 
	}
	
	std::string b = "";
	do
	{
		char str[8];
		
		b += " ";
		b += _itoa((*a).idx(), str,10);
		b += ":";
		b += m.is_boundary(*a)?"1":"0";
	} while (++a != m.halfface_vertices(hf).second);

	std::cout << "当前半边序号：" << he.idx() << " 当前半边点 from-to：" <<
		m.halfedge(he).from_vertex().idx() << "-" << m.halfedge(he).to_vertex().idx() <<
		" 当前半面序号:" << hf.idx() << " 半面点集：" << b << std::endl;
}

int BaseComplexSetFiller::GetSFaceExtend_Simple(const HalfEdgeHandle & he, const HalfFaceHandle & hf, OpenVolumeMesh::EdgePropertyT<int>& intEProp_sigular)
{
	HalfEdgeHandle now_he_handle = he; 
	HalfEdgeHandle last_he_handle = he;
	HalfFaceHandle now_hf_handle = hf;
	EdgeHandle now_edge_handle;
	EdgeHandle last_edge_handle;
	HalfFaceHandle hf_h_temp[4];
	int count = 0;
	do
	{
		//ShowNowTemp(*mesh, now_he_handle, now_hf_handle);
		last_he_handle = now_he_handle; 
		//面内对边
		now_he_handle = mesh->next_halfedge_in_halfface(now_he_handle, now_hf_handle);
		now_he_handle = mesh->next_halfedge_in_halfface(now_he_handle, now_hf_handle);

		now_edge_handle = mesh->edge_handle(now_he_handle);
		last_edge_handle = mesh->edge_handle(last_he_handle);

		//ShowNowTemp(*mesh, now_he_handle, now_hf_handle);
		//检查是否到边界或者奇异边
		if (mesh->is_boundary(last_edge_handle)) { //起始边在边界上
			
			if (mesh->is_boundary(now_edge_handle)) { //对边在边界上 
				return count;
			}
			else if(!intEProp_sigular[now_edge_handle]){//对边不在边界上 且不是奇异边
				hf_all_set.insert(now_hf_handle); 
				f_all_set.insert(mesh->face_handle(now_hf_handle));
				//跳到下个邻接半面
				int f_count = 0, index_finded = 0;
				for (OpenVolumeMesh::HalfEdgeHalfFaceIter hehf_iter = mesh->halfedge_halffaces(now_he_handle).first;
					hehf_iter != mesh->halfedge_halffaces(now_he_handle).second; ++hehf_iter, ++f_count) {
					hf_h_temp[f_count] = *hehf_iter;
					if (*hehf_iter == now_hf_handle) {
						index_finded = f_count;
					}
				}
				now_hf_handle = hf_h_temp[(index_finded + 2) % 4];
				++count; 
			}
			else {//对边不在边界上 是奇异边
				hf_all_set.insert(now_hf_handle);
				f_all_set.insert(mesh->face_handle(now_hf_handle));
				++count; 
				return count;
			}

		}
		else { //起始边不在边界上
			if (mesh->is_boundary(now_edge_handle)|| intEProp_sigular[now_edge_handle]) { //对边在边界上或是奇异边
				hf_all_set.insert(now_hf_handle);
				f_all_set.insert(mesh->face_handle(now_hf_handle));
				++count;
				return count;
			}
			else {//对边不在边界上 也不是奇异边
				hf_all_set.insert(now_hf_handle);
				f_all_set.insert(mesh->face_handle(now_hf_handle));
				//跳到下个邻接半面
				int f_count = 0, index_finded = 0;
				for (OpenVolumeMesh::HalfEdgeHalfFaceIter hehf_iter = mesh->halfedge_halffaces(now_he_handle).first;
					hehf_iter != mesh->halfedge_halffaces(now_he_handle).second; ++hehf_iter, ++f_count) {
					hf_h_temp[f_count] = *hehf_iter;
					if (*hehf_iter == now_hf_handle) {
						index_finded = f_count;
					}
				}
				now_hf_handle = hf_h_temp[(index_finded + 2) % 4];
				++count;
			}
		}

		
	} while (!mesh->is_boundary(mesh->face_handle(now_hf_handle)));
	return count;
}

//对于开放型奇异边的面延展
int BaseComplexSetFiller::GetSFaceExtendedOfOrientation_Opened(const HalfEdgeHandle& he, const HalfFaceHandle& hf,
	OpenVolumeMesh::HalfEdgePropertyT<bool> &boolHEProp_Checked, OpenVolumeMesh::HalfFacePropertyT<bool> &boolHFProp_Checked,
	OpenVolumeMesh::EdgePropertyT<int> &intEProp_sigular) {

	HalfEdgeHandle he_now = he; 

	HalfFaceHandle hf_now = hf; 

	int temp_count = 0; 
	
	boolHEProp_Checked[he] = true; 
	boolHFProp_Checked[hf] = true; 
	while ((!mesh->is_boundary(he_now)) && (!mesh->is_boundary(hf_now)))
	{
		/*
		-----------					-----------
		|										|
		|				----》					|
		|										|
		-----------					------------
		*/
		ShowNowTemp(*mesh, he_now, hf_now);
		he_now = mesh->next_halfedge_in_halfface(he_now, hf_now);
		he_now = mesh->next_halfedge_in_halfface(he_now, hf_now);
		this->hf_all_set.insert(hf_now);
		temp_count++;
		this->hf_all_set.insert(mesh->opposite_halfface_handle(hf_now));
		temp_count++;
		

		//蠢啊····  一想很简单 
		//1、测试 半边的邻接面数量是不是4  -----测了 半边跟边的邻接数相同
		//2、这就很简单了

		//错的
		//hf_now = mesh->adjacent_halfface_on_sheet(hf_now,he_now); 
		

		boolHFProp_Checked[hf_now] = true;

		//TODO  添加对该面对边是否在边界是奇异边的判断  是则置标志 跳出
		if (mesh->is_boundary(mesh->edge_handle(he_now))) {//该面一边已到达边界
			//boolHEProp_Checked[he_now] = true; 
			
			/*this->hf_all_set.insert(hf_now); 
			temp_count++;
			this->hf_all_set.insert(mesh->opposite_halfface_handle(hf_now));
			temp_count++;*/
			break;
		}

		if (intEProp_sigular[mesh->edge_handle(he_now)] != 0) {//该边在奇异边上
			//boolHEProp_Checked[he_now] = true;
			
			/*this->hf_all_set.insert(hf_now); 
			temp_count++;
			this->hf_all_set.insert(mesh->opposite_halfface_handle(hf_now));
			temp_count++;*/
			break; 
		}

		HalfFaceHandle hf_h_temp[4];
		int count=0,index_finded = 0; 
		for (OpenVolumeMesh::HalfEdgeHalfFaceIter hehf_iter = mesh->halfedge_halffaces(he_now).first;
			hehf_iter != mesh->halfedge_halffaces(he_now).second; ++hehf_iter,++count) {
			hf_h_temp[count] = *hehf_iter; 
			if (*hehf_iter == hf_now) {
				index_finded = count; 
			}
		}
		hf_now = hf_h_temp[(index_finded + 2) % 4];

		//he_now = mesh->opposite_halfedge_handle(he_now); 
		
		/*
		if (hf_now == mesh->InvalidHalfFaceHandle) {//邻接半面可能到达边界了
			//TODO 等会补全
			//该方向没有找下去的必要了 不是内部分割面
			//标志重置
			boolHEProp_Checked[he_now] = true; 
			he_now = mesh->next_halfedge_in_halfface(mesh->opposite_halfedge_handle(he_now), hf_now);
			he_now = mesh->next_halfedge_in_halfface(he_now, hf_now);
			break; 
		}
		else {//邻接半面未到边界
			//找平行半边
			//TODO 不知道对应数据结构上 该半边是否属于两个块
			if (!boolHFProp_Checked[hf_now]) {
				he_now = mesh->next_halfedge_in_halfface(mesh->opposite_halfedge_handle(he_now), hf_now);
				he_now = mesh->next_halfedge_in_halfface(he_now, hf_now);
				
				boolHFProp_Checked[hf_now] = true; 
				//不用手动检查去重
				this->hf_all_set.insert(hf_now); 
				++temp_count; 
			}
			
		}*/
	};

	return temp_count; 
}

//对闭合型的遍历
int BaseComplexSetFiller::GetSFaceExtendedOfOrientation_Closed(const HalfEdgeHandle& he, const HalfFaceHandle& hf,
	OpenVolumeMesh::HalfEdgePropertyT<bool> &boolHEProp_Checked, OpenVolumeMesh::HalfFacePropertyT<bool> &boolHFProp_Checked,
	OpenVolumeMesh::EdgePropertyT<int> &intEProp_sigular) {

	int s_line_index = intEProp_sigular[mesh->edge_handle(he)]; 
	int temp_count_sface = 0; 
	HalfEdgeHandle he_now = he;
	HalfFaceHandle hf_now = hf;
	boolHEProp_Checked[he_now] = true; 
	boolHFProp_Checked[hf_now] = true;

	hf_all_set.insert(hf_now);
	//hf_all_set.insert(mesh->opposite_halfface_handle(hf_now)); 
	//甄别下是不是在边界上
	if (mesh->is_boundary(mesh->face_handle(hf_now))) {


		if (mesh->is_boundary(mesh->opposite_halfface_handle(hf_now))) {
			hf_all_set.insert(mesh->opposite_halfface_handle(hf_now));
			boolHFProp_Checked[mesh->opposite_halfface_handle(hf_now)] = true;
		}
		//else//理论上不需要····  加上试试是否影响块分割
		//{
		//	hf_all_set.insert(mesh->opposite_halfface_handle(hf_now));
		//}
		

		do
		{
			//hf_now = mesh->adjacent_halfface_on_sheet(hf_now, he_now);
			ShowNowTemp(*mesh, he_now, hf_now);
			he_now = mesh->next_halfedge_in_halfface(he_now, hf_now);
			//TODO
			if (he_now == mesh->InvalidHalfEdgeHandle) {
				std::cout << "遍历到异常边";
			}
			he_now = mesh->next_halfedge_in_halfface(he_now, hf_now);

			//boolHEProp_Checked[he_now] = true;

			
			//触碰到奇异边
			if (intEProp_sigular[mesh->edge_handle(he_now)] !=0) {
				break;
			}

			//对闭合的情况而言很可能闭合的分割面全在表面  度数的确定按3来  可以分两种方案来搞
			if (mesh->is_boundary(mesh->face_handle(hf_now))) {
				//hf_now = mesh->neighboring_outside_halfface();

				//hf_now = mesh->adjacent_halfface_on_surface(hf_now, he_now);

				HalfFaceHandle hf_h_temp[4];
				int count = 0, index_finded = 0;
				for (OpenVolumeMesh::HalfEdgeHalfFaceIter hehf_iter = mesh->halfedge_halffaces(he_now).first;
					hehf_iter != mesh->halfedge_halffaces(he_now).second; ++hehf_iter, ++count) {
					hf_h_temp[count] = *hehf_iter;
					if (*hehf_iter == hf_now) {
						index_finded = count;
					}
				}
				hf_now = mesh->is_boundary(mesh->face_handle( hf_h_temp[(index_finded + 1) % 3]))?
					hf_h_temp[(index_finded + 1) % 3]: hf_h_temp[(index_finded + 2) % 3];

				boolHFProp_Checked[hf_now] = true;

				//对边界的封闭型其实需要插入两个面
				hf_all_set.insert(hf_now);
				
				//hf_all_set.insert(mesh->opposite_halfface_handle(hf_now));
				//++temp_count_sface;
				
				++temp_count_sface;

			}
			else {//非表面则在内部
				HalfFaceHandle hf_h_temp[4];
				int count = 0, index_finded = 0;
				for (OpenVolumeMesh::HalfEdgeHalfFaceIter hehf_iter = mesh->halfedge_halffaces(he_now).first;
					hehf_iter != mesh->halfedge_halffaces(he_now).second; ++hehf_iter, ++count) {
					hf_h_temp[count] = *hehf_iter;
					if (*hehf_iter == hf_now) {
						index_finded = count;
					}
				}
				hf_now = hf_h_temp[(index_finded + 2) % 4];
				boolHFProp_Checked[hf_now] = true;


				hf_all_set.insert(hf_now);
				if (mesh->is_boundary(mesh->opposite_halfface_handle(hf_now))) {
					hf_all_set.insert(mesh->opposite_halfface_handle(hf_now));
					++temp_count_sface;
				}
				else {//理论上不加应该不影响啊  但是分割错了
					hf_all_set.insert(mesh->opposite_halfface_handle(hf_now));
					++temp_count_sface;
				}
				++temp_count_sface;
			}
			
			
		} while (intEProp_sigular[mesh->edge_handle(he_now)] != s_line_index);
	}
	else {
		do
		{
			he_now = mesh->next_halfedge_in_halfface(he_now, hf_now);
			he_now = mesh->next_halfedge_in_halfface(he_now, hf_now);

			boolHEProp_Checked[he_now] = true;

			if (mesh->is_boundary(he_now)) {
				break;
			}

			//对闭合的情况而言很可能闭合的分割面全在表面  度数的确定按3来  可以分两种方案来搞
			
			HalfFaceHandle hf_h_temp[4];
			int count = 0, index_finded = 0;
			for (OpenVolumeMesh::HalfEdgeHalfFaceIter hehf_iter = mesh->halfedge_halffaces(he_now).first;
				hehf_iter != mesh->halfedge_halffaces(he_now).second; ++hehf_iter, ++count) {
				hf_h_temp[count] = *hehf_iter;
				if (*hehf_iter == hf_now) {
					index_finded = count;
				}
			}
			hf_now = hf_h_temp[(index_finded + 2) % 4];
			boolHFProp_Checked[hf_now] = true;
			
			hf_all_set.insert(hf_now);
			++temp_count_sface;
			hf_all_set.insert(mesh->opposite_halfface_handle(hf_now)); 
			++temp_count_sface;
			if (intEProp_sigular[mesh->edge_handle(he_now)] == s_line_index) {
				break;
			}


		} while (!mesh->is_boundary(he_now)); 

	}
	

	return temp_count_sface; 
}


//优化优化 50万以上数据溢出 1500单元栈溢出 已释放解决
//填充种子区域  务必使用初始单元
void BaseComplexSetFiller::HexCellFlooding(const unsigned char from_oriention , OpenVolumeMesh::CellPropertyT<bool>& boolCellChecked,const CellHandle& initHandle) {

	CellHandle now_cell_handle;
	HalfFaceHandle now_halfface_handle, tmp_halfface_handle;
	//初始的遍历方向
	unsigned char now_oriention = mesh->XF;
	//unsigned char now_oriention = from_oriention; 
	now_cell_handle = initHandle;

	int cycle_orientions = 0;
	if (!initHandle) {
		//初始种子为空的情况
		//该处使用有危险！！！！！
		//会重复遍历同一块分割部分！！！
		now_cell_handle = *mesh->cells_begin();
	}

	//提前过滤
	if (boolCellChecked[now_cell_handle]) {
		return;
	}

	cell_temp_set.insert(now_cell_handle);
	boolCellChecked[now_cell_handle] = true;
	//使用初始种子开始填充
	do
	{

		if (now_oriention == con_orientations[0]) {
			tmp_halfface_handle = mesh->xfront_halfface(now_cell_handle);
		}else if (now_oriention == con_orientations[1]) {
			tmp_halfface_handle = mesh->xback_halfface(now_cell_handle);
		}
		else if (now_oriention == con_orientations[2]) {
			tmp_halfface_handle = mesh->yfront_halfface(now_cell_handle);
		}
		else if (now_oriention == con_orientations[3]) {
			tmp_halfface_handle = mesh->yback_halfface(now_cell_handle);
		}
		else if (now_oriention == con_orientations[4]) {
			tmp_halfface_handle = mesh->zfront_halfface(now_cell_handle);
		}
		else if (now_oriention == con_orientations[5]) {
			tmp_halfface_handle = mesh->zback_halfface(now_cell_handle);
		}
		
		if (!mesh->is_boundary(mesh->face_handle(tmp_halfface_handle))&&
			//(tmp_halfface_handle != *hf_all_set.end())&& 
			(tmp_halfface_handle != mesh->InvalidHalfFaceHandle)&&
			hf_all_set.end() == hf_all_set.find(tmp_halfface_handle)&&
			hf_all_set.end() == hf_all_set.find(mesh->opposite_halfface_handle(tmp_halfface_handle))) {//未达到边界或者分割面
			//当前加入集合
			CellHandle temp_cell = mesh->incident_cell(mesh->opposite_halfface_handle(tmp_halfface_handle));
			HexCellFlooding(now_oriention%2?now_oriention-1:now_oriention+1,boolCellChecked, temp_cell);
		}


		//下个方向
		++now_oriention; 

	} while (now_oriention<=5);
}

//不存在set clear导致的提前释放空间问题（为什么··）
void BaseComplexSetFiller::OneBaseComplexFilling(std::vector<SingularEdge> se_vector,std::set<BaseComplex,compare_BaseComplex>& baseComplexSet,const CellHandle& initHandle) {
	int count_cell = 0; 
	CellHandle initCell;
	//添加六面体单元属性
	OpenVolumeMesh::CellPropertyT<bool> boolCellChecked = mesh->request_cell_property<bool>("boolCellChecked");

	int i = 0; 
	//提前把边界面上点筛选出来
	std::set<VertexHandle, compare_OVM> surface_vex_set; 
	for (std::set<HalfFaceHandle, compare_OVM>::iterator hf_iter = hf_all_set.begin(); hf_iter != hf_all_set.end(); ++hf_iter) {
		std::vector<HalfEdgeHandle> vector_he = mesh->halfface(*hf_iter).halfedges();
		for (std::vector<HalfEdgeHandle>::iterator he_iter = vector_he.begin(); he_iter != vector_he.end(); ++he_iter) {
			surface_vex_set.insert(mesh->halfedge(*he_iter).from_vertex());
			surface_vex_set.insert(mesh->halfedge(*he_iter).to_vertex());
		}
	}

	

	//开始种子填充
	while (count_cell<mesh->n_cells()) {
		//寻找未遍历的初始种子
		for (OpenVolumeMesh::CellIter c_iter = mesh->cells_begin(); c_iter != mesh->cells_end(); ++c_iter) {
			if (!boolCellChecked[*c_iter]) {
				initCell = *c_iter;
				break;
			}
		}
		

		this->HexCellFlooding(INVALID_ORIENTATION,boolCellChecked,initCell);
		BaseComplex baseComplex;
		baseComplex.setIdx(++i);
		InsertObject2BaseComplex(baseComplex,surface_vex_set );
		baseComplexSet.insert(baseComplex); 
		count_cell += cell_temp_set.size();
		//清空暂存										
		std::cout << " {" << (int)cell_temp_set.size()<<"} "<<"\n";
		cell_temp_set.clear();
	}
	std::cout << " " << count_cell;


	//TODO:带上标记
	bool is_have_point[250] = {false};

	/*1.迭代每条边
	2.选取每条边的终结点之一
	3.查找哪个bc包含该节点，将该bc起始单元确定
	*/
	//给base_complex集合中的元素设置起始单元
	for (std::vector<SingularEdge>::iterator se_iter = se_vector.begin(); se_iter != se_vector.end(); ++se_iter) {

		//TODO:
		//跳过环形边
		if ((*se_iter).Get_is_loopped()) {
			continue;
		}

		//起始点不在这选 在se的内部函数中决定
		//找起始点邻接面
		(*se_iter).FindSeparatedFaceHandles();
		//找邻接cell单元
		(*se_iter).FindCellAroundSE();
		

		int coi = 0;
		for (std::set<BaseComplex, compare_BaseComplex>::iterator bc_iter = baseComplexSet.begin();
			bc_iter != baseComplexSet.end(); ++bc_iter,++coi) {
			//是否存在该cell
			bool cell_existing = false;
			
			const BaseComplex &bc_o = const_cast<BaseComplex&>(*bc_iter);
			BaseComplex  &bc = const_cast<BaseComplex&>(bc_o);

			//看该bc是否已经填充初始单元
			if (bc.get_origination_cell().idx() > -1) {
				continue;
			}
			std::set<VertexHandle, compare_OVM> set_ = bc.getVes();
			
			if ((set_.find((*se_iter).Get_end_point(false)) != set_.end()) || (set_.find((*se_iter).Get_end_point(true)) != set_.end())) {
				is_have_point[coi] = true;
			}

			std::set<VertexHandle, compare_OVM> outter_ve_set = bc.getVes();
			std::set<CellHandle, compare_OVM> bc_cell_set = bc.getCellSet();
		
		
			VertexHandle ve_temp;
			
			//起始单元拓展位置
			unsigned char origination_cell_index = 8;
			//终结点1 2
			VertexHandle v1_v2_arr[2] = { (*se_iter).Get_end_point(true),(*se_iter).Get_end_point(false) };
			
			for (int v_index = 0; v_index < 2; ++v_index) {
				if (outter_ve_set.find(v1_v2_arr[v_index]) != outter_ve_set.end()) {
					ve_temp = v1_v2_arr[v_index];
					
					std::set<CellHandle, compare_OVM_For_Singular> set_cell =(*se_iter).Get_cell_aroud_set(!v_index);
					
					if ((bc.Idx() == 10)||(bc.Idx() == 22)||(bc.Idx() == 28)) {
						std::cout << "";
					}
					//遍历起始点邻接单元 对单元确定位置
					for (std::set<CellHandle, compare_OVM_For_Singular>::iterator orig_cell_iter = set_cell.begin();
						orig_cell_iter != set_cell.end(); ++orig_cell_iter) {
						//找到该单元
						if (bc.find_cell(*orig_cell_iter)) {
							
							//查找对应的8合并起始位置
							bc.set_origination_cell(*orig_cell_iter);

							//寻找该单元相对于起始点的位置
							origination_cell_index = (*se_iter).JudgeVer2CellIndex(*orig_cell_iter,v1_v2_arr[v_index]);
							if ((*orig_cell_iter).idx() == -1) {
								std::cout << "guale";
							}
							bc.set_origination_index(origination_cell_index);
							if (origination_cell_index == 8) {
								std::cout << "guale";
							}
							cell_existing = true;
							break;
						}
						
					}//for end
					//判断是否找到了cell在bc内部
					if (cell_existing) {
						break;
					}
					
				}//if end
			}
			
		
		}
	}

	//检测一下
	int a = 0;
	std::set<BaseComplex, compare_BaseComplex>::iterator iter; 
	for (iter = baseComplexSet.begin(); iter != baseComplexSet.end(); iter++) {
		const BaseComplex &bc_o = const_cast<BaseComplex&>(*iter);
		BaseComplex  &bc = const_cast<BaseComplex&>(bc_o);
		if (bc.get_origination_cell().idx() == -1) {
			a++; continue;
		}
		if (bc.get_origination_index() == 8) {
			a++; continue;

		}
	}
	if (iter != baseComplexSet.end()) {
		std::cout << "";
	}
	int b = 0;
	for (int i = 0; i < baseComplexSet.size(); ++i) {
		if (is_have_point[i]) b++;
	}
	std::cout << "gaodingo";
}


//填充每个分割块
void BaseComplexSetFiller::InsertObject2BaseComplex(BaseComplex& baseComplex, std::set<VertexHandle, compare_OVM>& surface_vex_set) {
	
	std::set<VertexHandle, compare_OVM> v_all_set;

	for (std::set<CellHandle, compare_OVM>::iterator bc_iter = cell_temp_set.begin();
		bc_iter != cell_temp_set.end();
		++bc_iter) {
		
		baseComplex.insertCellHandle(*bc_iter);
		//插入全点集
		std::pair<OpenVolumeMesh::CellVertexIter, OpenVolumeMesh::CellVertexIter> cell_v_pair = mesh->cell_vertices(*bc_iter);
		for (OpenVolumeMesh::CellVertexIter cell_v_iter = cell_v_pair.first; cell_v_iter != cell_v_pair.second; ++cell_v_iter) {
			v_all_set.insert(*cell_v_iter); 
		}

		std::vector<HalfFaceHandle> hf_ve = mesh->cell(*bc_iter).halffaces();
		for (std::vector<HalfFaceHandle>::iterator hf_iter = hf_ve.begin(); hf_iter != hf_ve.end(); ++hf_iter) {
			//对边界半面的插入
			if (mesh->is_boundary(mesh->face_handle(*hf_iter))|| (hf_all_set.find(*hf_iter) != hf_all_set.end())) {//可找到 即在分割面或边界上
				baseComplex.insertHalfFace(*hf_iter);
			}
			

		}
	
		

	}
	//对整个两种点集的填充
	std::set<HalfFaceHandle, compare_OVM> hf_temp = baseComplex.getHalfFaceSet();
	//填充点集
	for (std::set<HalfFaceHandle, compare_OVM>::iterator hf_iter = hf_temp.begin(); hf_iter != hf_temp.end(); ++hf_iter) {
		std::pair<OpenVolumeMesh::HalfFaceVertexIter, OpenVolumeMesh::HalfFaceVertexIter> hf_v_pair = mesh->halfface_vertices(*hf_iter);
		for (OpenVolumeMesh::HalfFaceVertexIter hf_v_iter = hf_v_pair.first; hf_v_iter != hf_v_pair.second; ++hf_v_iter) {
			baseComplex.insertVertex(*hf_v_iter, false);
		}
	}

	std::set<VertexHandle, compare_OVM> ve_bdy_temp = baseComplex.getVes();
	//遍历筛下内部点集
	for (std::set<VertexHandle, compare_OVM>::iterator expect_v_iter = v_all_set.begin(); expect_v_iter != v_all_set.end(); ++expect_v_iter) {
		//找不到
		if ((surface_vex_set.find(*expect_v_iter) == surface_vex_set.end())&&!mesh->is_boundary(*expect_v_iter)) {
			baseComplex.insertVertex(*expect_v_iter, true);
		}
	}
	std::cout << " {" <<(int)baseComplex.getVesInner().size()<< "} ";
	//清理下空间
	v_all_set.clear(); 
	hf_temp.clear();
	ve_bdy_temp.clear();

	
}
