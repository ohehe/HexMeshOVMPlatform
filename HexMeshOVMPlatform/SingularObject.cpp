#include "SingularObject.h"

SingularObject::SingularObject() {

}
SingularObject::SingularObject(HexMeshV3f* in_mesh) {

	mesh = in_mesh;
	B_READY = true;
}

SingularObject::~SingularObject() {

};
void SingularObject::FindSingularObject_simple() {
	if (!B_READY) return;

	uint64_t count_of_sin = 0;

	//�����������Լ�¼
	OpenVolumeMesh::VertexPropertyT<bool> boolVProp_sigular = mesh->request_vertex_property<bool>("IsSigularPoint");
	OpenVolumeMesh::VertexPropertyT<bool> boolVProp_bdy = mesh->request_vertex_property<bool>("IsVeOnBoundy");

	//�����־û�
	mesh->set_persistent(boolVProp_bdy);
	mesh->set_persistent(boolVProp_sigular);
	for (OpenVolumeMesh::VertexIter ve_it = mesh->vertices_begin(); ve_it != mesh->vertices_end(); ve_it++) {
		//std::pair<OpenVolumeMesh::VertexVertexIter, OpenVolumeMesh::VertexVertexIter> pair = mesh->vertex_vertices(*ve_it);
		boolVProp_bdy[*ve_it] = mesh->is_boundary(*ve_it);
		int valence = boolVProp_bdy[*ve_it] ? VERTICE_BDY_REGULAR_COUNT : VERTICE_INNER_REGULAR_COUNT;

		if (mesh->valence(*ve_it) != valence) {
			//�����
			vector_singular_vertex_handles.push_back(*ve_it);
			//������Լ�¼
			boolVProp_sigular[*ve_it] = true;

		}
		else {
			//�������
			boolVProp_sigular[*ve_it] = false;
		}
	}


	//�����
	//�����������Լ�¼

	//OpenVolumeMesh::EdgePropertyT<bool> boolEProp_sigular = mesh->request_edge_property<bool>("IsSingularEdge");
	//�߽���
	OpenVolumeMesh::EdgePropertyT<bool> boolEProp_bdy = mesh->request_edge_property<bool>("IsEdgeOnBoundy");
	//��������� ���Ϊ0�ı�Ϊ����� ���nΪ���n��
	OpenVolumeMesh::EdgePropertyT<int> intEProp_sigular = mesh->request_edge_property<int>("SingularEdgeIndex");

	//�����־û�
	mesh->set_persistent(intEProp_sigular);
	mesh->set_persistent(boolEProp_bdy);

	//����Ƿ������������
	OpenVolumeMesh::EdgePropertyT<bool> boolEProp_isChecked = mesh->request_edge_property<bool>("IsChecked");
	for (OpenVolumeMesh::EdgeIter e_iter = mesh->edges_begin(); e_iter != mesh->edges_end(); ++e_iter) {
		boolEProp_isChecked[*e_iter] = false;
	}

	for (OpenVolumeMesh::EdgeIter e_iter = mesh->edges_begin(); e_iter != mesh->edges_end(); ++e_iter) {
		boolEProp_isChecked[*e_iter] = true;
		bool checkGoto = false;
		int count = 0;

		/*for (OpenVolumeMesh::FaceIter fa_iter = mesh->faces_begin(); fa_iter != mesh->faces_end(); ++fa_iter) {
		OpenVolumeMesh::OpenVolumeMeshFace face = mesh->face(*fa_iter);

		for (std::vector<HalfEdgeHandle>::const_iterator it = face.halfedges().begin(); it != face.halfedges().end(); ++it) {
		if (mesh->edge_handle(*it).idx() == (*e_iter).idx()) {
		count++;
		}
		}
		}*/

		count = mesh->valence(*e_iter);

		if (mesh->is_boundary(*e_iter)) {
			if (count != 3) {
				intEProp_sigular[*e_iter] = 1;
				vector_singular_edge_handles.push_back(*e_iter);
				std::cout << "select:" << count << "-bdy-" << (*e_iter).idx() << std::endl;
				++count_of_sin;
			}
			else {
				std::cout << "unselect:" << count << "-bdy-" << (*e_iter).idx() << std::endl;
			}
		}
		else {
			if (count != 4) {
				intEProp_sigular[*e_iter] = 1;
				vector_singular_edge_handles.push_back(*e_iter);
				std::cout << "select:" << count << "-ine-" << (*e_iter).idx() << std::endl;
				++count_of_sin;
			}
			else {
				std::cout << "unselect:" << count << "-ine-" << (*e_iter).idx() << std::endl;
			}
		}
	}
	std::cout << "count_select:" << count_of_sin << std::endl;
}

void SingularObject::FindSingularObjectForBC(std::vector<SingularEdge>& se_vector, bool b_need_ver, bool b_need_edge) {
	if ((!b_need_ver) || (!b_need_edge)) return ;
	if (!B_READY) return ;

	//����������������Լ�¼
	OpenVolumeMesh::VertexPropertyT<bool> boolVProp_sigular = mesh->request_vertex_property<bool>("IsSigularPoint");
	OpenVolumeMesh::VertexPropertyT<bool> boolVProp_bdy = mesh->request_vertex_property<bool>("IsVeOnBoundy");

	//�����־û�
	mesh->set_persistent(boolVProp_bdy);
	mesh->set_persistent(boolVProp_sigular);

	//�߽���
	OpenVolumeMesh::EdgePropertyT<bool> boolEProp_bdy = mesh->request_edge_property<bool>("IsEdgeOnBoundy");
	//��������� ���Ϊ0�ı�Ϊ����� ���nΪ���n��
	OpenVolumeMesh::EdgePropertyT<int> intEProp_sigular = mesh->request_edge_property<int>("SingularEdgeIndex");

	//�����־û�
	mesh->set_persistent(intEProp_sigular);
	mesh->set_persistent(boolEProp_bdy);

	//����Ƿ������������
	OpenVolumeMesh::EdgePropertyT<bool> boolEProp_isChecked = mesh->request_edge_property<bool>("IsChecked");
	
	//*******************************************************************************
	//���������
	for (OpenVolumeMesh::VertexIter ve_it = mesh->vertices_begin(); ve_it != mesh->vertices_end(); ve_it++) {
		//std::pair<OpenVolumeMesh::VertexVertexIter, OpenVolumeMesh::VertexVertexIter> pair = mesh->vertex_vertices(*ve_it);
		boolVProp_bdy[*ve_it] = mesh->is_boundary(*ve_it);
		int valence = boolVProp_bdy[*ve_it] ? VERTICE_BDY_REGULAR_COUNT : VERTICE_INNER_REGULAR_COUNT;

		if (mesh->valence(*ve_it) != valence) {
			//�����
			vector_singular_vertex_handles.push_back(*ve_it);
			//������Լ�¼
			boolVProp_sigular[*ve_it] = true;

		}
		else {
			//�������
			boolVProp_sigular[*ve_it] = false;
		}
	}

	//��Ǳ߽��
	for (OpenVolumeMesh::EdgeIter edge_iter = mesh->edges_begin(); edge_iter != mesh->edges_end(); ++edge_iter) {
		if (mesh->is_boundary(mesh->edge(*edge_iter).from_vertex()) &&
			mesh->is_boundary(mesh->edge(*edge_iter).to_vertex())) {
			boolEProp_bdy[*edge_iter] = true;
		}
		else {
			boolEProp_bdy[*edge_iter] = false;
		}

	}

	//��������� ���������һ����������
	std::pair<OpenVolumeMesh::EdgeIter, OpenVolumeMesh::EdgeIter> pair_edge = mesh->edges();

	//����߼���
	uint32_t now_s_edge_index = 0;
	uint16_t now_s_edge_valance = 0;
	bool is_singular = false;

	for (OpenVolumeMesh::EdgeIter edge_iter = pair_edge.first; edge_iter != pair_edge.second; ++edge_iter) {
		//�����ѱ�����
		if (boolEProp_isChecked[*edge_iter]) continue;
		is_singular = false;
		//�ж��Ƿ��������
		//TODO:����һ�� ������ܻ�ĵ�
		uint16_t now_edge_valance = mesh->valence(*edge_iter);

		//TODO:�Ҵ���
		Edge edge_d = mesh->edge(*edge_iter);
		if ((edge_d.from_vertex().idx() == 41635)) {
			std::cout << "";
		}

		if ((edge_d.from_vertex().idx() == 33929)) {
			std::cout << "";
		}
		if (edge_d.from_vertex().idx() == 33391) {
			std::cout << "";
		}
		//TODO: ��߲�Ҫֱ�ӱ��
		boolEProp_isChecked[*edge_iter] = true;
		//�жϱ��Ƿ��ڱ߽���
		if (boolEProp_bdy[*edge_iter]) {
			//�ڱ߽���

			if (now_edge_valance != EDGE_BDY_REGULAR_COUNT)
				is_singular = true;
			else
				is_singular = false;

		}
		else {
			//���ڲ�

			if (now_edge_valance != EDGE_INNER_REGULAR_COUNT)
				is_singular = true;
			else
				is_singular = false;
		}

		//������� �����´�
		if (!is_singular) continue;

		OpenVolumeMesh::OpenVolumeMeshEdge edge = mesh->edge(*edge_iter);

		//��close�Ĵ��� ����Ƿ��ȫ�ڱ߽�
		bool is_always_bdy = boolEProp_bdy[*edge_iter];
		VertexHandle now_v_handle = edge.from_vertex();
		//��¼�´�
		bool first = true;

		//����������Ҫ�������� 
		//SingularEdge(HexMeshV3f *mesh, std::vector<VertexHandle> ver_vector, std::vector<EdgeHandle> edge_vector,VertexHandle endv1 , VertexHandle endv2) 

		//����ռ�
		std::vector<VertexHandle> ver_vector;
		std::vector<EdgeHandle> edge_vector;
		VertexHandle end_v_from = edge.from_vertex();
		VertexHandle end_v_to = edge.to_vertex();

		//���뵱ǰ�ս��
		ver_vector.push_back(end_v_from);
		ver_vector.push_back(end_v_to);
		//���뵱ǰ������
		edge_vector.push_back(*edge_iter);

		//������������+1
		++now_s_edge_index;
		//�ﵽ�߽���
		bool b_meet_bdy_from = false;
		bool b_meet_bdy_to = false;
		//to�����from����
		do {//ѭ����������
			now_v_handle = (first ? edge.from_vertex() : edge.to_vertex());

			//����һ��singularEdge

			//TODO:�ô�ѭ����ֹ ��Ϊ�ﵽ�ô�û�п�����߽�ֹ �ж�
			do {//��չ�����������
				//TODO:�Ժ�ĵ�������� ����goto
			START_NEW_EDGE_PART:
				std::pair<OpenVolumeMesh::VertexOHalfEdgeIter, OpenVolumeMesh::VertexOHalfEdgeIter> _v_e_pair =
					mesh->outgoing_halfedges(now_v_handle);
				
				//TODO:���϶Ե�ǰ������ĸ��� now_v_handle
				OpenVolumeMesh::VertexOHalfEdgeIter vohe_iter = _v_e_pair.first;
				for (; vohe_iter != _v_e_pair.second; ++vohe_iter) {
					//�����ѱ�����
					if (boolEProp_isChecked[mesh->edge_handle(*vohe_iter)]) {
						continue;
					}
					//�������
					//TODO:�ǲ���Ӧ�øĵ� ����������Ա������ ����������� ��������һ��
					//boolEProp_isChecked[mesh->edge_handle(*vohe_iter)] = true;

					//���ѱ�����ʼ�� ����
					if (mesh->edge_handle(*vohe_iter) == *edge_iter) continue;
					//���������һ���� ����
					//TODO:�� ��������ʼ���ڲ��ⲿ���Ͳ�ͬ
					
					//ԭʼ�ڱ߽绹���ڲ�
					//if (is_always_bdy) {
					//	// �жϵ�ǰ���Ƿ��ڱ߽� �Ƿ������ͬ
					//	if (boolEProp_bdy[mesh->edge_handle(*vohe_iter)]) {
					//		if (mesh->valence(mesh->edge_handle(*vohe_iter)) != now_edge_valance) continue;
					//	}
					//	else {
					//		//���ڲ� ���ⲿ���ڲ� ����
					//		continue;
					//	}
					//}
					//else {
					//	if (boolEProp_bdy[mesh->edge_handle(*vohe_iter)]) {
					//		//���ⲿ �ⲿ���ڲ� ����
					//		continue;
					//	}
					//	else {
					//		//�ڲ����ڲ�
					//		if (mesh->valence(mesh->edge_handle(*vohe_iter)) != now_edge_valance) continue;
					//	}

					//}
					if (is_always_bdy == boolEProp_bdy[mesh->edge_handle(*vohe_iter)]) {
						if (mesh->valence(mesh->edge_handle(*vohe_iter)) != now_edge_valance) continue;
					}
					else {
						continue;
					}
					

					//������Ա��
					boolEProp_isChecked[mesh->edge_handle(*vohe_iter)] = true;
					//����
					//��伯�� �ж��յ�

					OpenVolumeMesh::OpenVolumeMeshEdge edge_next = mesh->edge(mesh->edge_handle(*vohe_iter));
					//ֻҪ����һ�����ڲ��Ĳ��־ͷ��ⲿ��
					is_always_bdy = is_always_bdy ? (boolEProp_bdy[mesh->edge_handle(*vohe_iter)]) : false;
					//�жϵ�ǰ���� ѡ��ǰ��˳��
					if (first) {//from
								//1���ж��ɻ�
								//�жϵ�ǰ�����ߵ��ս���Ƿ�����ʼ��end_v_to��� now_v_handl����һ��

						if ((now_v_handle =
							(edge_next.from_vertex() == now_v_handle) ? edge_next.to_vertex() : edge_next.from_vertex())
							== end_v_to) {
							//�Ƿ���⻷
							if (is_always_bdy) {
								//������һ���ߺ͵�
								ver_vector.insert(ver_vector.begin(),end_v_to);
								edge_vector.insert(edge_vector.begin(),mesh->edge_handle(*vohe_iter));
								//���⻷ ������������Ч�߽��б�Ǵ���
								for (std::vector<EdgeHandle>::iterator ed_iter = edge_vector.begin();
									ed_iter != edge_vector.end(); ++ed_iter) {

									//�������ݽṹ �����
									//���������������
									intEProp_sigular[*ed_iter] = -now_s_edge_index;

								}
								//�������ݽṹ��
								se_vector.push_back(SingularEdge(mesh, ver_vector, edge_vector, *(ver_vector.begin()), *(ver_vector.end() - 1), true));


							}
							else {//���ڲ���
								  //������һ����ͱ�
								ver_vector.insert(ver_vector.begin() ,end_v_to);
								edge_vector.insert(edge_vector.begin() ,mesh->edge_handle(*vohe_iter));
								//���⻷ ������������Ч�߽��б�Ǵ���
								for (std::vector<EdgeHandle>::iterator ed_iter = edge_vector.begin();
									ed_iter != edge_vector.end(); ++ed_iter) {

									//�������ݽṹ �����
									//���������������
									intEProp_sigular[*ed_iter] = -now_s_edge_index;

								}
								//�������ݽṹ��
								se_vector.push_back(SingularEdge(mesh, ver_vector, edge_vector, *(ver_vector.begin()), *(ver_vector.end() - 1),true));

							}
							//���� ������߽���
							goto OUTTER;
						}

						//2���ж��ǻ��ε��߽�
						edge_vector.insert(edge_vector.begin(), mesh->edge_handle(*vohe_iter));
						ver_vector.insert(ver_vector.begin(), now_v_handle);
						//�����¸������� TODO:����Ҫ ���ʼ���ж��о�������


						//�ж��Ƿ񵽴�߽� ��תto���� ��to�����¼���н��
						if (boolVProp_bdy[now_v_handle]) {
							//�ı�� ����߽�
							b_meet_bdy_from = true;
							//����߽� ��תto����

							goto OUTER_IN;
						}
						else {
							//δ����߽�

							//TODO:�������ȥ�� ���goto
							//�����������������������
							goto START_NEW_EDGE_PART;
						}
					}
					else {//to
						  //1���ж��ɻ�
						  //�жϵ�ǰ�����ߵ��ս���Ƿ�����ʼ��end_v_to��� now_v_handl����һ��

						//������ʵ����Ҫ�� ��������````
						if ((now_v_handle =
							(edge_next.to_vertex() == now_v_handle) ? edge_next.from_vertex() : edge_next.to_vertex())
							== end_v_from) {
							//�Ƿ���⻷
							if (is_always_bdy) {
								//������һ���ߺ͵�
								ver_vector.push_back(end_v_from);
								edge_vector.push_back(mesh->edge_handle(*vohe_iter));
								//���⻷ ������������Ч�߽��б�Ǵ���
								for (std::vector<EdgeHandle>::iterator ed_iter = edge_vector.begin();
									ed_iter != edge_vector.end(); ++ed_iter) {

									//�������ݽṹ �����
									//���������������
									intEProp_sigular[*ed_iter] = -now_s_edge_index;

								}
								//�������ݽṹ��
								se_vector.push_back(SingularEdge(mesh, ver_vector, edge_vector, *(ver_vector.begin()), *(ver_vector.end() - 1),true));


							}
							else {//���ڲ���
								  //������һ����ͱ�
								ver_vector.push_back(end_v_from);
								edge_vector.push_back(mesh->edge_handle(*vohe_iter));
								//���⻷ ������������Ч�߽��б�Ǵ���
								for (std::vector<EdgeHandle>::iterator ed_iter = edge_vector.begin();
									ed_iter != edge_vector.end(); ++ed_iter) {

									//�������ݽṹ �����
									//���������������
									intEProp_sigular[*ed_iter] = -now_s_edge_index;

								}
								//�������ݽṹ��
								se_vector.push_back(SingularEdge(mesh, ver_vector, edge_vector, *(ver_vector.begin()), *(ver_vector.end() - 1),true));

							}
							//���� ������߽���
							goto OUTTER;
						}

						//2���ж��ǻ��ε��߽�
						edge_vector.push_back( mesh->edge_handle(*vohe_iter));
						ver_vector.push_back( now_v_handle);
						//�����¸������� TODO:����Ҫ ���ʼ���ж��о�������


						//�ж��Ƿ񵽴�߽� ��תto���� ��to�����¼���н��
						if (boolVProp_bdy[now_v_handle]) {
							//����� ����߽�
							b_meet_bdy_to = true;
							//����߽� �������ڲ�ѭ��

							break;
						}
						else {
							//δ����߽�

							//TODO:�������ȥ�� ���goto
							//�����������������������
							goto START_NEW_EDGE_PART;
						}
					}

				}

				//1��from���򵽱߽� toδ��ʼ
				//2��from���򵽱߽� to���ڲ��ս�
				//3��from���򵽱߽� to���߽�
				//4��from�������ڲ���ֹ toδ��ʼ
				//5��from�������ڲ���ֹ to����߽�
				//6��-------���������ܣ�from�����to�������ڲ��ս�

				if (b_meet_bdy_from) {//from���߽�
					
					if (first) {//toδ��ʼ
						//�ֶ���ת��to����
						goto OUTER_IN; 
					}
					else {//to�ڱ߽�����ڲ��ս�
						if (b_meet_bdy_to) {//to����߽�
							//���singular_edge ����
							for (std::vector<EdgeHandle>::iterator com_edge_iter = edge_vector.begin();
								com_edge_iter != edge_vector.end(); ++com_edge_iter) {
								intEProp_sigular[*com_edge_iter] = now_s_edge_index;
							}
							//�������ݽṹ��
							se_vector.push_back(SingularEdge(mesh, ver_vector, edge_vector, *(ver_vector.begin()), *(ver_vector.end() - 1),false));
							//����Ѱ����һ�����������
							goto OUTTER;
						}	
						else if ((*vohe_iter) == *(_v_e_pair.second)) {//to�ڲ��ս�
																	   //���singular_edge ����
							for (std::vector<EdgeHandle>::iterator com_edge_iter = edge_vector.begin();
								com_edge_iter != edge_vector.end(); ++com_edge_iter) {
								intEProp_sigular[*com_edge_iter] = now_s_edge_index;
							}
							//�������ݽṹ��
							se_vector.push_back(SingularEdge(mesh, ver_vector, edge_vector, *(ver_vector.begin()), *(ver_vector.end() - 1),false));
							//����Ѱ����һ�����������
							goto OUTTER;
						}
						
					}
					
				}
				else if ((*vohe_iter) == *(_v_e_pair.second)) {//fromδ���߽���from���ڲ���ֹ
					if (first) {//from���ڲ��ս� toδ��ʼ
						goto OUTER_IN;
					}

					if (b_meet_bdy_to) {//to���򵽴�߽�
						//���singular_edge ����
						for (std::vector<EdgeHandle>::iterator com_edge_iter = edge_vector.begin();
							com_edge_iter != edge_vector.end(); ++com_edge_iter) {
							intEProp_sigular[*com_edge_iter] = now_s_edge_index;
						}
						//�������ݽṹ��
						se_vector.push_back(SingularEdge(mesh, ver_vector, edge_vector, *(ver_vector.begin()), *(ver_vector.end() - 1),false));
						
						//����Ѱ����һ�����������
						goto OUTTER;
					}
					else {//to���ڲ��ս� ������������ǲ���Ҫ�ģ�
						//���singular_edge ����
						for (std::vector<EdgeHandle>::iterator com_edge_iter = edge_vector.begin();
							com_edge_iter != edge_vector.end(); ++com_edge_iter) {
							intEProp_sigular[*com_edge_iter] = now_s_edge_index;
						}
						//�������ݽṹ��
						se_vector.push_back(SingularEdge(mesh, ver_vector, edge_vector, *(ver_vector.begin()), *(ver_vector.end() - 1),false));

						//����Ѱ����һ�����������
						goto OUTTER;
					}

				}

				
			} while (true);//��չ�����������

		OUTER_IN:
			//�ĵ��´�����
			first = !first;
		} while (!first); //ѭ����������
	OUTTER:
		std::cout << "s_edge index:" << now_s_edge_index << " is_always_bdy:" << is_always_bdy <<
			" edge_count:"<< (*(se_vector.end() - 1)).edge_vector.size()<<
			" from_point:" << (int)((*(se_vector.end() - 1)).ver_vector[0].idx()) <<
			" to_point:" << (int)((*((*(se_vector.end() - 1)).ver_vector.end()-1)).idx())<<"\n";
		
	}//����������ÿ����

	//TODO:��һ��
	std::set<EdgeHandle, compare_OVM_For_Singular> en_set; 
	for (std::vector<SingularEdge>::iterator it = se_vector.begin(); it != se_vector.end(); ++it) {

		const SingularEdge &bc_o = const_cast<SingularEdge&>(*it);
		SingularEdge  &bc = const_cast<SingularEdge&>(bc_o);

		for (std::vector<EdgeHandle>::iterator ii = bc.edge_vector.begin(); ii != bc.edge_vector.end(); ++ii) {
			en_set.insert((*ii));
		}
		//if (bc.Get_is_loopped()) {
			Edge edge = mesh->edge(bc.edge_vector[0]);
			Edge edge1 = mesh->edge(*(bc.edge_vector.end() - 1));
			std::cout << "";
		//}
	}
	std::cout << "\nsize:" << (int)en_set.size()<<"\n";

}

//������߲����԰��ģʽΪ��׼
void SingularObject::FindSingularObject(bool b_need_ver, bool b_need_edge) {
	if (!B_READY) return;

	//���������
	if (b_need_ver) {

		//�����������Լ�¼
		OpenVolumeMesh::VertexPropertyT<bool> boolVProp_sigular = mesh->request_vertex_property<bool>("IsSigularPoint");
		OpenVolumeMesh::VertexPropertyT<bool> boolVProp_bdy = mesh->request_vertex_property<bool>("IsVeOnBoundy");

		//�����־û�
		mesh->set_persistent(boolVProp_bdy);
		mesh->set_persistent(boolVProp_sigular);


		for (OpenVolumeMesh::VertexIter ve_it = mesh->vertices_begin(); ve_it != mesh->vertices_end(); ve_it++) {
			//std::pair<OpenVolumeMesh::VertexVertexIter, OpenVolumeMesh::VertexVertexIter> pair = mesh->vertex_vertices(*ve_it);
			boolVProp_bdy[*ve_it] = mesh->is_boundary(*ve_it);
			int valence = boolVProp_bdy[*ve_it] ? VERTICE_BDY_REGULAR_COUNT : VERTICE_INNER_REGULAR_COUNT;

			if (mesh->valence(*ve_it) != valence) {
				//�����
				vector_singular_vertex_handles.push_back(*ve_it);
				//������Լ�¼
				boolVProp_sigular[*ve_it] = true;

			}
			else {
				//�������
				boolVProp_sigular[*ve_it] = false;
			}
		}
	}
	if (b_need_edge) {

		//�����������Լ�¼

		//OpenVolumeMesh::EdgePropertyT<bool> boolEProp_sigular = mesh->request_edge_property<bool>("IsSingularEdge");
		//�߽���
		OpenVolumeMesh::EdgePropertyT<bool> boolEProp_bdy = mesh->request_edge_property<bool>("IsEdgeOnBoundy");
		//��������� ���Ϊ0�ı�Ϊ����� ���nΪ���n��
		OpenVolumeMesh::EdgePropertyT<int> intEProp_sigular = mesh->request_edge_property<int>("SingularEdgeIndex");

		//�����־û�
		mesh->set_persistent(intEProp_sigular);
		mesh->set_persistent(boolEProp_bdy);

		//����Ƿ������������
		OpenVolumeMesh::EdgePropertyT<bool> boolEProp_isChecked = mesh->request_edge_property<bool>("IsChecked");
		for (OpenVolumeMesh::EdgeIter e_iter = mesh->edges_begin(); e_iter != mesh->edges_end(); ++e_iter) {
			boolEProp_isChecked[*e_iter] = false;
		}

		int d_index = 0;

		//��ת���
		bool JUMP2NEXT = false;
		for (OpenVolumeMesh::EdgeIter ed_it = mesh->edges_begin(); ed_it != mesh->edges_end(); ed_it++) {
			JUMP2NEXT = false;

			//�ж��Ƿ��Ѿ�������
			if (boolEProp_isChecked[*ed_it]) {
				continue;
			}
			boolEProp_bdy[*ed_it] = mesh->is_boundary(*ed_it);
			boolEProp_isChecked[*ed_it] = true;
			int valence = boolEProp_bdy[*ed_it] ? EDGE_BDY_REGULAR_COUNT : EDGE_INNER_REGULAR_COUNT;
			//��ǰ�����ߵ�ά��
			int valence_p = 0;
			if ((valence_p = mesh->valence(*ed_it)) != valence) {//�������
																 //�����
				vector_singular_edge_handles.push_back(*ed_it);
				//������Լ�¼
				intEProp_sigular[*ed_it] = ++d_index;

				//��������ͬvalence�ıߴ�������
				//edgeΪ��ǰ�����ߵ���һ����
				Edge edge = mesh->edge(*ed_it);

				//��¼��ʼ�� �ж��ǿ��ŵĻ��Ǳպϵ�
				OpenVolumeMesh::EdgeHandle start_edge_handler = *ed_it;

				//��ʼ�յ�
				OpenVolumeMesh::VertexHandle ve_from_handler = edge.from_vertex();
				OpenVolumeMesh::VertexHandle ve_to_handler = edge.to_vertex();



				//ͨ�����յ��������һ����
				//ͨ�����������һ����
				//OpenVolumeMesh::HalfEdgeHandle he_handler = mesh->halfedge(ve_from_handler, ve_to_handler);
				//OpenVolumeMesh::HalfEdgeHalfFaceIter hf_hf_iter = mesh->halfedge_halffaces(he_handler).first;

				//��ǰ������
				OpenVolumeMesh::OpenVolumeMeshEdge edge_now = edge;
				//--->
				//�ô���to���Ƿ��ڱ߽�����Ϊ��ֹ�ǲ��Եġ�������  Ӧ������û��������ͬ����������������ڸ��յ���
				short count_link_s_line = 0;
				while (true) {
					//�ж��Ƿ��������to�������
					count_link_s_line = 0;
					//���ڲ����쵽�߽�������Ҫ�ı�valence_p��׼ �߽��ϱߵ��ж�valenceΪ3
					for (OpenVolumeMesh::VertexOHalfEdgeIter vohe_it_init = mesh->outgoing_halfedges(ve_to_handler).first;
						vohe_it_init != mesh->outgoing_halfedges(ve_to_handler).second; ++vohe_it_init) {
						if (mesh->valence(mesh->edge_handle(*vohe_it_init)) !=
							(mesh->is_boundary(*vohe_it_init) ? EDGE_BDY_REGULAR_COUNT : EDGE_INNER_REGULAR_COUNT)) {
							if (mesh->valence(mesh->edge_handle(*vohe_it_init)) == valence_p) {
								++count_link_s_line;
							}
						}

					}
					//����һ����ͬ��������֮���������ж��޷����� ����
					if (count_link_s_line == 1) {
						break;
					}


					OpenVolumeMesh::VertexOHalfEdgeIter vohe_iter = mesh->outgoing_halfedges(ve_to_handler).first;
					while (vohe_iter != mesh->outgoing_halfedges(ve_to_handler).second) {//while start
						edge_now = mesh->halfedge(*vohe_iter);
						if (edge_now.to_vertex().idx() != ve_from_handler.idx()) {//�����յ���ź��Ƿ���ͬ �����ߵ����  ������b��������ô���ӳ�䵽�ߵġ�����������

							if (mesh->valence(mesh->edge_handle(*vohe_iter)) == valence_p) {//���Ƿ�һ�� ����߿��Լ�����չ��ȥ

								if (boolEProp_isChecked[mesh->edge_handle(*vohe_iter)]) {//�Ѿ�������
																						 //���������������еĸ�����µ��ø�
									for (OpenVolumeMesh::EdgeIter ed_iter = mesh->edges_begin(); ed_iter != mesh->edges_end(); ++ed_iter) {
										if (intEProp_sigular[*ed_iter] == d_index) {
											intEProp_sigular[*ed_iter] = -d_index;
										}
									}
									//����ѭ��
									JUMP2NEXT = true;
									break;
								}
								else {
									vector_singular_edge_handles.push_back(mesh->edge_handle(*vohe_iter));
								}

								//����������
								intEProp_sigular[mesh->edge_handle(*vohe_iter)] = d_index;
								std::cout << "select:" << (*vohe_iter).idx() << " id:" << d_index << std::endl;
								//������Լ�¼ �˱��ѱ�����
								boolEProp_bdy[mesh->edge_handle(*vohe_iter)] = mesh->is_boundary(mesh->edge_handle(*vohe_iter));
								boolEProp_isChecked[mesh->edge_handle(*vohe_iter)] = true;

								//���õ�ǰ������ Ϊ�´α���׼��
								if (edge_now.from_vertex().idx() == edge.to_vertex().idx()) {//����
									ve_from_handler = edge_now.from_vertex();
									ve_to_handler = edge_now.to_vertex();
								}
								else {
									ve_from_handler = edge_now.to_vertex();
									ve_to_handler = edge_now.from_vertex();
								}

								//��һ�� ������������һ���� �����ڲ���whileѭ��
								//�ƶ�ǰһ������ߵ�ָ��
								edge = edge_now;
								break;
							}
							else {
								//��������óȻ������� �����淽��ı���û��ִ��
								//++d_index;
								++vohe_iter;
							}


						}
						else {//��ԭʼ������ı� 
							  //�������Ӹõ��������
							++vohe_iter;

						}
					}//���ս��to���ڽӱ� while end


					if (JUMP2NEXT) {//closed������ߵ����
						break;
					}
					else {//1�����������δ�պϵ���� ��ת��������������
						  //2���������������û����һ��ά����ͬ�����

					}
				}//��to����ı�������

				 //�����ת���Ϊtrue�Ļ� ��Ҫ�����淽����������Ի��αպϱ�û������
				if (JUMP2NEXT) {
					continue;
				}

				//�����Ƕ��淽���from��ʼ����
				//��ԭָ��ǰһ���ߵ�ָ�� ��ԭ�յ�
				edge = mesh->edge(*ed_it);
				ve_from_handler = edge.from_vertex();
				ve_to_handler = edge.to_vertex();

				while (true) {

					//�ж��Ƿ��������from�������
					count_link_s_line = 0;
					for (OpenVolumeMesh::VertexOHalfEdgeIter vohe_it_init = mesh->outgoing_halfedges(ve_from_handler).first;
						vohe_it_init != mesh->outgoing_halfedges(ve_from_handler).second; ++vohe_it_init) {
						if (mesh->valence(mesh->edge_handle(*vohe_it_init)) !=
							(mesh->is_boundary(*vohe_it_init) ? EDGE_BDY_REGULAR_COUNT : EDGE_INNER_REGULAR_COUNT)) {
							if (mesh->valence(mesh->edge_handle(*vohe_it_init)) == valence_p) {
								++count_link_s_line;
							}
						}
					}
					//����һ����ͬ��������֮���������ж��޷����� ����
					if (count_link_s_line == 1) {
						break;
					}
					OpenVolumeMesh::VertexOHalfEdgeIter vohe_iter = mesh->outgoing_halfedges(ve_from_handler).first;
					while (vohe_iter != mesh->outgoing_halfedges(ve_from_handler).second) {//while start
						edge_now = mesh->halfedge(*vohe_iter);
						if (edge_now.to_vertex().idx() != ve_to_handler.idx()) {//�����յ���ź��Ƿ���ͬ �����ߵ����  ������b��������ô���ӳ�䵽�ߵġ�����������
							if (mesh->valence(mesh->edge_handle(*vohe_iter)) == valence_p) {//���Ƿ�һ�� ����߿��Լ�����չ��ȥ
																							//�Է���һ���Ǽ����ѱ����ߵĹ���
																							/*if (boolEProp_isChecked[mesh->edge_handle(*vohe_iter)]) {
																							std::cout << "��TM���׿�����û�����������������";
																							}*/
								vector_singular_edge_handles.push_back(mesh->edge_handle(*vohe_iter));

								//����������
								intEProp_sigular[mesh->edge_handle(*vohe_iter)] = d_index;
								std::cout << "select:" << (*vohe_iter).idx() << " id:" << d_index << std::endl;
								//������Լ�¼ �˱��ѱ�����
								boolEProp_bdy[mesh->edge_handle(*vohe_iter)] = mesh->is_boundary(mesh->edge_handle(*vohe_iter));
								boolEProp_isChecked[mesh->edge_handle(*vohe_iter)] = true;

								//���õ�ǰ������ Ϊ�´α���׼��
								if (edge_now.to_vertex().idx() == edge.from_vertex().idx()) {//����
									ve_from_handler = edge_now.from_vertex();
									ve_to_handler = edge_now.to_vertex();
								}
								else {
									ve_from_handler = edge_now.to_vertex();
									ve_to_handler = edge_now.from_vertex();
								}

								//��һ�� ������������һ���� �����ڲ���whileѭ��
								//�ƶ�ǰһ������ߵ�ָ��
								edge = edge_now;
								break;

							}
							else {
								//��������óȻ������� �����淽��ı���û��ִ��
								//++d_index;
								++vohe_iter;
							}

						}
						else {//��ԭʼ������ı� 
							  //�������Ӹõ��������
							++vohe_iter;

						}
					}//while end ���ս��from���ڽӱ�

				}//��from����ı�������
			}
			else {
				intEProp_sigular[*ed_it] = 0;
				std::cout << "unselect:" << (*ed_it).idx() << std::endl;
			}

		}
	}
	return;
}
void SingularObject::SetColorAttr(int d_color_s_ver, int d_color_o_ver, int d_color_s_edge, int d_color_i_edge) {

}


//��ʼ������
bool SingularObject::InitMesh() {
	return true;
}

std::set<HalfFaceHandle, compare_OVM_For_Singular> SingularEdge::FindSeparatedFaceHandles()
{
	//���δ��ɷ��ؿռ���
	if (!this->is_filled_complish)
		return std::set<HalfFaceHandle, compare_OVM_For_Singular>();

	EdgeHandle growing_edge = mesh->InvalidEdgeHandle;

	VertexHandle handle_point[2] = {endPoint_1,endPoint_2}; 
	for (int i = 0; i < 2; ++i) {
		//TODO:Ҫ���
		if ((mesh->edge(edge_vector[0]).from_vertex() == handle_point[i]) || (mesh->edge(edge_vector[0]).to_vertex() == handle_point[i]))
			growing_edge = edge_vector[0];
		if ((mesh->edge(*(edge_vector.end() - 1)).from_vertex() == handle_point[i]) || (mesh->edge(*(edge_vector.end() - 1)).to_vertex() == handle_point[i]))
			growing_edge = *(edge_vector.end() - 1);

		std::pair<OpenVolumeMesh::HalfEdgeHalfFaceIter, OpenVolumeMesh::HalfEdgeHalfFaceIter> he_hf_pair0 = mesh->halfedge_halffaces(mesh->halfedge_handle(growing_edge, 0));
		std::pair<OpenVolumeMesh::HalfEdgeHalfFaceIter, OpenVolumeMesh::HalfEdgeHalfFaceIter> he_hf_pair1 = mesh->halfedge_halffaces(mesh->halfedge_handle(growing_edge, 1));

		std::set<HalfFaceHandle, compare_OVM_For_Singular> set_hf;

		//���뼯��
		for (OpenVolumeMesh::HalfEdgeHalfFaceIter iter = he_hf_pair0.first; iter != he_hf_pair0.second; ++iter) {
			set_hf.insert(*iter);
		}
		for (OpenVolumeMesh::HalfEdgeHalfFaceIter iter = he_hf_pair1.first; iter != he_hf_pair1.second; ++iter) {
			set_hf.insert(*iter);
		}
		if(i==0)
			this->faces_aroud_set_p1.insert(set_hf.begin(), set_hf.end());
		else
			this->faces_aroud_set_p2.insert(set_hf.begin(), set_hf.end());
		
	}
	
	return this->faces_aroud_set_p1;
}

std::set<CellHandle, compare_OVM_For_Singular> SingularEdge::FindCellAroundSE()
{
	// ��Ϸָ���ȷ����Ԫ
	/*for (std::set<HalfFaceHandle, compare_OVM_For_Singular>::iterator iter = faces_aroud_set_p1.begin(); iter != faces_aroud_set_p1.end(); ++iter) {
		if (mesh->incident_cell(*iter) == mesh->InvalidCellHandle) {
			continue;
		}
		this->cells_around_set_p1.insert(mesh->incident_cell(*iter));
	}
	for (std::set<HalfFaceHandle, compare_OVM_For_Singular>::iterator iter = faces_aroud_set_p2.begin(); iter != faces_aroud_set_p2.end(); ++iter) {
		if (mesh->incident_cell(*iter) == mesh->InvalidCellHandle) {
			continue;
		}
		this->cells_around_set_p2.insert(mesh->incident_cell(*iter));
	}*/
	//TODO:��ʱ��Ϊֱ�Ӱ�o_ver�ڽ�cellȫ����
	std::pair<OpenVolumeMesh::VertexCellIter, OpenVolumeMesh::VertexCellIter> vc_pair = mesh->vertex_cells(ver_vector[0]);
	for (OpenVolumeMesh::VertexCellIter vc_iter = vc_pair.first; vc_iter != vc_pair.second; ++vc_iter) {
		cells_around_set_p1.insert(*vc_iter);
	}
	std::pair<OpenVolumeMesh::VertexCellIter, OpenVolumeMesh::VertexCellIter> vc_pair1 = mesh->vertex_cells(ver_vector[ver_vector.size()-1]);
	for (OpenVolumeMesh::VertexCellIter vc_iter = vc_pair1.first; vc_iter != vc_pair1.second; ++vc_iter) {
		cells_around_set_p2.insert(*vc_iter);
	}
	return this->cells_around_set_p1;
}
//ȷ���õ�λ�ڸ�cell���ĸ�λ��
unsigned char SingularEdge::JudgeVer2CellIndex(CellHandle cell,VertexHandle point)
{
	CellHandle temp_cell = cell;
	//TODO: idx 9
	//TODO:������
	std::pair<OpenVolumeMesh::CellVertexIter, OpenVolumeMesh::CellVertexIter> pair_cv = mesh->cell_vertices(temp_cell);
	for (OpenVolumeMesh::CellVertexIter cv_iter = pair_cv.first; cv_iter != pair_cv.second; ++cv_iter) {
		std::cout << "vertex:" << (int)((*cv_iter).idx())<<"\n";
	}

	std::set<HalfFaceHandle, compare_OVM_For_Singular> set_hf;
	for (unsigned char uc_i = 0; uc_i != 8; ++uc_i) {
		//cell_vertex���ǰ�idx��С˳�����е� �ĵ�Ĭ��˳��  U_RELOCATED_CELL_OUT_INDEX
		VertexHandle ve_arr[4];
		int cout_arr[4] = { 0 };

		uint16_t now_i = 0;
		std::pair<OpenVolumeMesh::HalfFaceVertexIter, OpenVolumeMesh::HalfFaceVertexIter> pair_hfv1 =
			mesh->halfface_vertices(mesh->get_oriented_halfface(U_RELOCATED_CELL_OUT_INDEX_[uc_i][0], temp_cell));
		now_i = 0;
		for (OpenVolumeMesh::HalfFaceVertexIter hfv_iter = pair_hfv1.first; hfv_iter != pair_hfv1.second; ++hfv_iter) {
			ve_arr[now_i++] = *hfv_iter;
		}

		std::pair<OpenVolumeMesh::HalfFaceVertexIter, OpenVolumeMesh::HalfFaceVertexIter> pair_hfv2 =
			mesh->halfface_vertices(mesh->get_oriented_halfface(U_RELOCATED_CELL_OUT_INDEX_[uc_i][1], temp_cell));
		for (OpenVolumeMesh::HalfFaceVertexIter hfv_iter = pair_hfv2.first; hfv_iter != pair_hfv2.second; ++hfv_iter) {
			for (int i = 0; i < 4; ++i) {
				if (ve_arr[i].idx() == (*hfv_iter).idx()) {
					++cout_arr[i];
				}
			}
		}
		std::pair<OpenVolumeMesh::HalfFaceVertexIter, OpenVolumeMesh::HalfFaceVertexIter> pair_hfv3 =
			mesh->halfface_vertices(mesh->get_oriented_halfface(U_RELOCATED_CELL_OUT_INDEX_[uc_i][2], temp_cell));

		for (OpenVolumeMesh::HalfFaceVertexIter hfv_iter = pair_hfv3.first; hfv_iter != pair_hfv3.second; ++hfv_iter) {
			for (int i = 0; i < 4; ++i) {
				if (ve_arr[i].idx() == (*hfv_iter).idx()) {
					++cout_arr[i];
				}
			}
		}
		int max = 0, max_i = 0;
		for (int i = 0; i < 4; ++i) {
			if (cout_arr[i] > max) {
				max = cout_arr[i];
				max_i = i;
			}
		}

		if (ve_arr[max_i] == point) {
			return uc_i;
		}
	}
	std::cout << "gg";
	return 8;
}
