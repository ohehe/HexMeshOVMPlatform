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
//̰���������зָ��沢���ظ���
//�Ƿ�֧�ְ�߼��� (��ʱδд)
int BaseComplexSetFiller::SFaceSetSeeking(bool is_support_halfface) {
	if (!is_support_halfface) {
		return 0;
	}
	//��ȡ����˵�����
	OpenVolumeMesh::EdgePropertyT<int> intEProp_sigular = this->mesh->request_edge_property<int>("SingularEdgeIndex");
	
	int count_sface = 0;
	//��Ӱ�ߺ�������ı������
	OpenVolumeMesh::HalfEdgePropertyT<bool> boolHEProp_Checked = this->mesh->request_halfedge_property<bool>("IsChecked_HE");
	OpenVolumeMesh::HalfFacePropertyT<bool> boolHFProp_Checked = this->mesh->request_halfface_property<bool>("IsChecked_HF");

	for (OpenVolumeMesh::EdgeIter ed_it = mesh->edges_begin(); ed_it != mesh->edges_end(); ++ed_it) {
		if (boolHEProp_Checked[mesh->halfedge_handle(*ed_it, 0)] && boolHEProp_Checked[mesh->halfedge_handle(*ed_it, 1)]) {
			continue;
		}
		if (intEProp_sigular[*ed_it] != 0) {//�������
			if (intEProp_sigular[*ed_it] > 0) {//opened�͵������
				//����ͬ��ŵı���
				for (OpenVolumeMesh::EdgeIter ed_iter = mesh->edges_begin(); ed_iter != mesh->edges_end(); ++ed_iter) {
					if (intEProp_sigular[*ed_it] != intEProp_sigular[*ed_iter]) {
						continue;
					}
					//�԰�߽������δ��� 
					//���� ���� �ڽ���Ĺ�����
					/*
					�����ԡ��������ǲ��ǰ��հ�����ڽ���� ����˵����������ڱ��ϵ�
					*/
					if (!boolHEProp_Checked[mesh->halfedge_handle(*ed_it, 0)]) {//δ������
						boolHEProp_Checked[mesh->halfedge_handle(*ed_it, 0)] = true;
						for (OpenVolumeMesh::HalfEdgeHalfFaceIter he_hf_iter = mesh->halfedge_halffaces(mesh->halfedge_handle(*ed_it, 0)).first;
							he_hf_iter != mesh->halfedge_halffaces(mesh->halfedge_handle(*ed_it, 0)).second; ++he_hf_iter) {
							count_sface += this->GetSFaceExtendedOfOrientation_Opened(mesh->halfedge_handle(*ed_it, 0),
								*he_hf_iter, boolHEProp_Checked, boolHFProp_Checked, intEProp_sigular);
						}
					}
					
				}
				
				
			}
			else {//closed�͵������
				for (OpenVolumeMesh::EdgeIter ed_iter = mesh->edges_begin(); ed_iter != mesh->edges_end(); ++ed_iter) {
					if (intEProp_sigular[*ed_it] != intEProp_sigular[*ed_iter]) {
						continue;
					}
					//�԰�߽������δ��� 
					//���Է�һ�� 
					
					if (!boolHEProp_Checked[mesh->halfedge_handle(*ed_it, 0)]) {//δ������
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
		else {//�ֿ����������  ��ǰ��true��ʹ�²�ı�������
			boolHEProp_Checked[mesh->halfedge_handle(*ed_it, 0)] = true; 
			boolHEProp_Checked[mesh->halfedge_handle(*ed_it, 1)] = true;
		}

	}
	return count_sface;
}
//******************************************
//1.����������ߵ��ڽ������졣
//	(1)���������쵽��һ������ߵķָ��沿��,���������߼��ϡ�
//  (2)������ֱ�����쵽�߽��Ҳ��뵱ǰ�ָ��漯���ཻ�ķָ��沿�֣����������߼��ϡ�
//	(3)����ʣ�µ������棬���ﵱǰ�ָ��漯���е�Ԫֹͣ��������ָ��漯�ϡ�
int BaseComplexSetFiller::SFaceSetSeeking_Complex(bool is_support_halfface,const std::vector<SingularEdge>& vector_se) {
	uint64_t count_step_direct = 0u;
	uint64_t count_step_to_bdy = 0u;
	uint64_t count_step_to_existing_sf = 0u;

	//��ȡ���
	OpenVolumeMesh::EdgePropertyT<int> intEProp_sigular = this->mesh->request_edge_property<int>("SingularEdgeIndex");

	//TODO:step-1
	for (OpenVolumeMesh::EdgePropertyT<int>::iterator e_p_iter = intEProp_sigular.begin(); e_p_iter != intEProp_sigular.end(); ++e_p_iter) {
		if (!(*e_p_iter)) {//�����������
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

	//��ȡ����˵�����
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

	std::cout << "��ǰ�����ţ�" << he.idx() << " ��ǰ��ߵ� from-to��" <<
		m.halfedge(he).from_vertex().idx() << "-" << m.halfedge(he).to_vertex().idx() <<
		" ��ǰ�������:" << hf.idx() << " ����㼯��" << b << std::endl;
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
		//���ڶԱ�
		now_he_handle = mesh->next_halfedge_in_halfface(now_he_handle, now_hf_handle);
		now_he_handle = mesh->next_halfedge_in_halfface(now_he_handle, now_hf_handle);

		now_edge_handle = mesh->edge_handle(now_he_handle);
		last_edge_handle = mesh->edge_handle(last_he_handle);

		//ShowNowTemp(*mesh, now_he_handle, now_hf_handle);
		//����Ƿ񵽱߽���������
		if (mesh->is_boundary(last_edge_handle)) { //��ʼ���ڱ߽���
			
			if (mesh->is_boundary(now_edge_handle)) { //�Ա��ڱ߽��� 
				return count;
			}
			else if(!intEProp_sigular[now_edge_handle]){//�Ա߲��ڱ߽��� �Ҳ��������
				hf_all_set.insert(now_hf_handle); 
				f_all_set.insert(mesh->face_handle(now_hf_handle));
				//�����¸��ڽӰ���
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
			else {//�Ա߲��ڱ߽��� �������
				hf_all_set.insert(now_hf_handle);
				f_all_set.insert(mesh->face_handle(now_hf_handle));
				++count; 
				return count;
			}

		}
		else { //��ʼ�߲��ڱ߽���
			if (mesh->is_boundary(now_edge_handle)|| intEProp_sigular[now_edge_handle]) { //�Ա��ڱ߽��ϻ��������
				hf_all_set.insert(now_hf_handle);
				f_all_set.insert(mesh->face_handle(now_hf_handle));
				++count;
				return count;
			}
			else {//�Ա߲��ڱ߽��� Ҳ���������
				hf_all_set.insert(now_hf_handle);
				f_all_set.insert(mesh->face_handle(now_hf_handle));
				//�����¸��ڽӰ���
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

//���ڿ���������ߵ�����չ
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
		|				----��					|
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
		

		//������������  һ��ܼ� 
		//1������ ��ߵ��ڽ��������ǲ���4  -----���� ��߸��ߵ��ڽ�����ͬ
		//2����ͺܼ���

		//���
		//hf_now = mesh->adjacent_halfface_on_sheet(hf_now,he_now); 
		

		boolHFProp_Checked[hf_now] = true;

		//TODO  ��ӶԸ���Ա��Ƿ��ڱ߽�������ߵ��ж�  �����ñ�־ ����
		if (mesh->is_boundary(mesh->edge_handle(he_now))) {//����һ���ѵ���߽�
			//boolHEProp_Checked[he_now] = true; 
			
			/*this->hf_all_set.insert(hf_now); 
			temp_count++;
			this->hf_all_set.insert(mesh->opposite_halfface_handle(hf_now));
			temp_count++;*/
			break;
		}

		if (intEProp_sigular[mesh->edge_handle(he_now)] != 0) {//�ñ����������
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
		if (hf_now == mesh->InvalidHalfFaceHandle) {//�ڽӰ�����ܵ���߽���
			//TODO �ȻᲹȫ
			//�÷���û������ȥ�ı�Ҫ�� �����ڲ��ָ���
			//��־����
			boolHEProp_Checked[he_now] = true; 
			he_now = mesh->next_halfedge_in_halfface(mesh->opposite_halfedge_handle(he_now), hf_now);
			he_now = mesh->next_halfedge_in_halfface(he_now, hf_now);
			break; 
		}
		else {//�ڽӰ���δ���߽�
			//��ƽ�а��
			//TODO ��֪����Ӧ���ݽṹ�� �ð���Ƿ�����������
			if (!boolHFProp_Checked[hf_now]) {
				he_now = mesh->next_halfedge_in_halfface(mesh->opposite_halfedge_handle(he_now), hf_now);
				he_now = mesh->next_halfedge_in_halfface(he_now, hf_now);
				
				boolHFProp_Checked[hf_now] = true; 
				//�����ֶ����ȥ��
				this->hf_all_set.insert(hf_now); 
				++temp_count; 
			}
			
		}*/
	};

	return temp_count; 
}

//�Ապ��͵ı���
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
	//������ǲ����ڱ߽���
	if (mesh->is_boundary(mesh->face_handle(hf_now))) {


		if (mesh->is_boundary(mesh->opposite_halfface_handle(hf_now))) {
			hf_all_set.insert(mesh->opposite_halfface_handle(hf_now));
			boolHFProp_Checked[mesh->opposite_halfface_handle(hf_now)] = true;
		}
		//else//�����ϲ���Ҫ��������  ���������Ƿ�Ӱ���ָ�
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
				std::cout << "�������쳣��";
			}
			he_now = mesh->next_halfedge_in_halfface(he_now, hf_now);

			//boolHEProp_Checked[he_now] = true;

			
			//�����������
			if (intEProp_sigular[mesh->edge_handle(he_now)] !=0) {
				break;
			}

			//�Ապϵ�������Ժܿ��ܱպϵķָ���ȫ�ڱ���  ������ȷ����3��  ���Է����ַ�������
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

				//�Ա߽�ķ������ʵ��Ҫ����������
				hf_all_set.insert(hf_now);
				
				//hf_all_set.insert(mesh->opposite_halfface_handle(hf_now));
				//++temp_count_sface;
				
				++temp_count_sface;

			}
			else {//�Ǳ��������ڲ�
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
				else {//�����ϲ���Ӧ�ò�Ӱ�찡  ���Ƿָ����
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

			//�Ապϵ�������Ժܿ��ܱպϵķָ���ȫ�ڱ���  ������ȷ����3��  ���Է����ַ�������
			
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


//�Ż��Ż� 50������������� 1500��Ԫջ��� ���ͷŽ��
//�����������  ���ʹ�ó�ʼ��Ԫ
void BaseComplexSetFiller::HexCellFlooding(const unsigned char from_oriention , OpenVolumeMesh::CellPropertyT<bool>& boolCellChecked,const CellHandle& initHandle) {

	CellHandle now_cell_handle;
	HalfFaceHandle now_halfface_handle, tmp_halfface_handle;
	//��ʼ�ı�������
	unsigned char now_oriention = mesh->XF;
	//unsigned char now_oriention = from_oriention; 
	now_cell_handle = initHandle;

	int cycle_orientions = 0;
	if (!initHandle) {
		//��ʼ����Ϊ�յ����
		//�ô�ʹ����Σ�գ���������
		//���ظ�����ͬһ��ָ�֣�����
		now_cell_handle = *mesh->cells_begin();
	}

	//��ǰ����
	if (boolCellChecked[now_cell_handle]) {
		return;
	}

	cell_temp_set.insert(now_cell_handle);
	boolCellChecked[now_cell_handle] = true;
	//ʹ�ó�ʼ���ӿ�ʼ���
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
			hf_all_set.end() == hf_all_set.find(mesh->opposite_halfface_handle(tmp_halfface_handle))) {//δ�ﵽ�߽���߷ָ���
			//��ǰ���뼯��
			CellHandle temp_cell = mesh->incident_cell(mesh->opposite_halfface_handle(tmp_halfface_handle));
			HexCellFlooding(now_oriention%2?now_oriention-1:now_oriention+1,boolCellChecked, temp_cell);
		}


		//�¸�����
		++now_oriention; 

	} while (now_oriention<=5);
}

//������set clear���µ���ǰ�ͷſռ����⣨Ϊʲô������
void BaseComplexSetFiller::OneBaseComplexFilling(std::vector<SingularEdge> se_vector,std::set<BaseComplex,compare_BaseComplex>& baseComplexSet,const CellHandle& initHandle) {
	int count_cell = 0; 
	CellHandle initCell;
	//��������嵥Ԫ����
	OpenVolumeMesh::CellPropertyT<bool> boolCellChecked = mesh->request_cell_property<bool>("boolCellChecked");

	int i = 0; 
	//��ǰ�ѱ߽����ϵ�ɸѡ����
	std::set<VertexHandle, compare_OVM> surface_vex_set; 
	for (std::set<HalfFaceHandle, compare_OVM>::iterator hf_iter = hf_all_set.begin(); hf_iter != hf_all_set.end(); ++hf_iter) {
		std::vector<HalfEdgeHandle> vector_he = mesh->halfface(*hf_iter).halfedges();
		for (std::vector<HalfEdgeHandle>::iterator he_iter = vector_he.begin(); he_iter != vector_he.end(); ++he_iter) {
			surface_vex_set.insert(mesh->halfedge(*he_iter).from_vertex());
			surface_vex_set.insert(mesh->halfedge(*he_iter).to_vertex());
		}
	}

	

	//��ʼ�������
	while (count_cell<mesh->n_cells()) {
		//Ѱ��δ�����ĳ�ʼ����
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
		//����ݴ�										
		std::cout << " {" << (int)cell_temp_set.size()<<"} "<<"\n";
		cell_temp_set.clear();
	}
	std::cout << " " << count_cell;


	//TODO:���ϱ��
	bool is_have_point[250] = {false};

	/*1.����ÿ����
	2.ѡȡÿ���ߵ��ս��֮һ
	3.�����ĸ�bc�����ýڵ㣬����bc��ʼ��Ԫȷ��
	*/
	//��base_complex�����е�Ԫ��������ʼ��Ԫ
	for (std::vector<SingularEdge>::iterator se_iter = se_vector.begin(); se_iter != se_vector.end(); ++se_iter) {

		//TODO:
		//�������α�
		if ((*se_iter).Get_is_loopped()) {
			continue;
		}

		//��ʼ�㲻����ѡ ��se���ڲ������о���
		//����ʼ���ڽ���
		(*se_iter).FindSeparatedFaceHandles();
		//���ڽ�cell��Ԫ
		(*se_iter).FindCellAroundSE();
		

		int coi = 0;
		for (std::set<BaseComplex, compare_BaseComplex>::iterator bc_iter = baseComplexSet.begin();
			bc_iter != baseComplexSet.end(); ++bc_iter,++coi) {
			//�Ƿ���ڸ�cell
			bool cell_existing = false;
			
			const BaseComplex &bc_o = const_cast<BaseComplex&>(*bc_iter);
			BaseComplex  &bc = const_cast<BaseComplex&>(bc_o);

			//����bc�Ƿ��Ѿ�����ʼ��Ԫ
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
			
			//��ʼ��Ԫ��չλ��
			unsigned char origination_cell_index = 8;
			//�ս��1 2
			VertexHandle v1_v2_arr[2] = { (*se_iter).Get_end_point(true),(*se_iter).Get_end_point(false) };
			
			for (int v_index = 0; v_index < 2; ++v_index) {
				if (outter_ve_set.find(v1_v2_arr[v_index]) != outter_ve_set.end()) {
					ve_temp = v1_v2_arr[v_index];
					
					std::set<CellHandle, compare_OVM_For_Singular> set_cell =(*se_iter).Get_cell_aroud_set(!v_index);
					
					if ((bc.Idx() == 10)||(bc.Idx() == 22)||(bc.Idx() == 28)) {
						std::cout << "";
					}
					//������ʼ���ڽӵ�Ԫ �Ե�Ԫȷ��λ��
					for (std::set<CellHandle, compare_OVM_For_Singular>::iterator orig_cell_iter = set_cell.begin();
						orig_cell_iter != set_cell.end(); ++orig_cell_iter) {
						//�ҵ��õ�Ԫ
						if (bc.find_cell(*orig_cell_iter)) {
							
							//���Ҷ�Ӧ��8�ϲ���ʼλ��
							bc.set_origination_cell(*orig_cell_iter);

							//Ѱ�Ҹõ�Ԫ�������ʼ���λ��
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
					//�ж��Ƿ��ҵ���cell��bc�ڲ�
					if (cell_existing) {
						break;
					}
					
				}//if end
			}
			
		
		}
	}

	//���һ��
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


//���ÿ���ָ��
void BaseComplexSetFiller::InsertObject2BaseComplex(BaseComplex& baseComplex, std::set<VertexHandle, compare_OVM>& surface_vex_set) {
	
	std::set<VertexHandle, compare_OVM> v_all_set;

	for (std::set<CellHandle, compare_OVM>::iterator bc_iter = cell_temp_set.begin();
		bc_iter != cell_temp_set.end();
		++bc_iter) {
		
		baseComplex.insertCellHandle(*bc_iter);
		//����ȫ�㼯
		std::pair<OpenVolumeMesh::CellVertexIter, OpenVolumeMesh::CellVertexIter> cell_v_pair = mesh->cell_vertices(*bc_iter);
		for (OpenVolumeMesh::CellVertexIter cell_v_iter = cell_v_pair.first; cell_v_iter != cell_v_pair.second; ++cell_v_iter) {
			v_all_set.insert(*cell_v_iter); 
		}

		std::vector<HalfFaceHandle> hf_ve = mesh->cell(*bc_iter).halffaces();
		for (std::vector<HalfFaceHandle>::iterator hf_iter = hf_ve.begin(); hf_iter != hf_ve.end(); ++hf_iter) {
			//�Ա߽����Ĳ���
			if (mesh->is_boundary(mesh->face_handle(*hf_iter))|| (hf_all_set.find(*hf_iter) != hf_all_set.end())) {//���ҵ� ���ڷָ����߽���
				baseComplex.insertHalfFace(*hf_iter);
			}
			

		}
	
		

	}
	//���������ֵ㼯�����
	std::set<HalfFaceHandle, compare_OVM> hf_temp = baseComplex.getHalfFaceSet();
	//���㼯
	for (std::set<HalfFaceHandle, compare_OVM>::iterator hf_iter = hf_temp.begin(); hf_iter != hf_temp.end(); ++hf_iter) {
		std::pair<OpenVolumeMesh::HalfFaceVertexIter, OpenVolumeMesh::HalfFaceVertexIter> hf_v_pair = mesh->halfface_vertices(*hf_iter);
		for (OpenVolumeMesh::HalfFaceVertexIter hf_v_iter = hf_v_pair.first; hf_v_iter != hf_v_pair.second; ++hf_v_iter) {
			baseComplex.insertVertex(*hf_v_iter, false);
		}
	}

	std::set<VertexHandle, compare_OVM> ve_bdy_temp = baseComplex.getVes();
	//����ɸ���ڲ��㼯
	for (std::set<VertexHandle, compare_OVM>::iterator expect_v_iter = v_all_set.begin(); expect_v_iter != v_all_set.end(); ++expect_v_iter) {
		//�Ҳ���
		if ((surface_vex_set.find(*expect_v_iter) == surface_vex_set.end())&&!mesh->is_boundary(*expect_v_iter)) {
			baseComplex.insertVertex(*expect_v_iter, true);
		}
	}
	std::cout << " {" <<(int)baseComplex.getVesInner().size()<< "} ";
	//�����¿ռ�
	v_all_set.clear(); 
	hf_temp.clear();
	ve_bdy_temp.clear();

	
}
