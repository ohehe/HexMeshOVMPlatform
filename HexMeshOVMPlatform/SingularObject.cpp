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

	//添加奇异点属性记录
	OpenVolumeMesh::VertexPropertyT<bool> boolVProp_sigular = mesh->request_vertex_property<bool>("IsSigularPoint");
	OpenVolumeMesh::VertexPropertyT<bool> boolVProp_bdy = mesh->request_vertex_property<bool>("IsVeOnBoundy");

	//声明持久化
	mesh->set_persistent(boolVProp_bdy);
	mesh->set_persistent(boolVProp_sigular);
	for (OpenVolumeMesh::VertexIter ve_it = mesh->vertices_begin(); ve_it != mesh->vertices_end(); ve_it++) {
		//std::pair<OpenVolumeMesh::VertexVertexIter, OpenVolumeMesh::VertexVertexIter> pair = mesh->vertex_vertices(*ve_it);
		boolVProp_bdy[*ve_it] = mesh->is_boundary(*ve_it);
		int valence = boolVProp_bdy[*ve_it] ? VERTICE_BDY_REGULAR_COUNT : VERTICE_INNER_REGULAR_COUNT;

		if (mesh->valence(*ve_it) != valence) {
			//奇异点
			vector_singular_vertex_handles.push_back(*ve_it);
			//添加属性记录
			boolVProp_sigular[*ve_it] = true;

		}
		else {
			//非奇异点
			boolVProp_sigular[*ve_it] = false;
		}
	}


	//奇异边
	//添加奇异边属性记录

	//OpenVolumeMesh::EdgePropertyT<bool> boolEProp_sigular = mesh->request_edge_property<bool>("IsSingularEdge");
	//边界标记
	OpenVolumeMesh::EdgePropertyT<bool> boolEProp_bdy = mesh->request_edge_property<bool>("IsEdgeOnBoundy");
	//奇异边属性 序号为0的边为规则边 序号n为编号n的
	OpenVolumeMesh::EdgePropertyT<int> intEProp_sigular = mesh->request_edge_property<int>("SingularEdgeIndex");

	//声明持久化
	mesh->set_persistent(intEProp_sigular);
	mesh->set_persistent(boolEProp_bdy);

	//检测是否遍历过的属性
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

	//添加奇异点奇异边属性记录
	OpenVolumeMesh::VertexPropertyT<bool> boolVProp_sigular = mesh->request_vertex_property<bool>("IsSigularPoint");
	OpenVolumeMesh::VertexPropertyT<bool> boolVProp_bdy = mesh->request_vertex_property<bool>("IsVeOnBoundy");

	//声明持久化
	mesh->set_persistent(boolVProp_bdy);
	mesh->set_persistent(boolVProp_sigular);

	//边界标记
	OpenVolumeMesh::EdgePropertyT<bool> boolEProp_bdy = mesh->request_edge_property<bool>("IsEdgeOnBoundy");
	//奇异边属性 序号为0的边为规则边 序号n为编号n的
	OpenVolumeMesh::EdgePropertyT<int> intEProp_sigular = mesh->request_edge_property<int>("SingularEdgeIndex");

	//声明持久化
	mesh->set_persistent(intEProp_sigular);
	mesh->set_persistent(boolEProp_bdy);

	//检测是否遍历过的属性
	OpenVolumeMesh::EdgePropertyT<bool> boolEProp_isChecked = mesh->request_edge_property<bool>("IsChecked");
	
	//*******************************************************************************
	//查找奇异点
	for (OpenVolumeMesh::VertexIter ve_it = mesh->vertices_begin(); ve_it != mesh->vertices_end(); ve_it++) {
		//std::pair<OpenVolumeMesh::VertexVertexIter, OpenVolumeMesh::VertexVertexIter> pair = mesh->vertex_vertices(*ve_it);
		boolVProp_bdy[*ve_it] = mesh->is_boundary(*ve_it);
		int valence = boolVProp_bdy[*ve_it] ? VERTICE_BDY_REGULAR_COUNT : VERTICE_INNER_REGULAR_COUNT;

		if (mesh->valence(*ve_it) != valence) {
			//奇异点
			vector_singular_vertex_handles.push_back(*ve_it);
			//添加属性记录
			boolVProp_sigular[*ve_it] = true;

		}
		else {
			//非奇异点
			boolVProp_sigular[*ve_it] = false;
		}
	}

	//标记边界边
	for (OpenVolumeMesh::EdgeIter edge_iter = mesh->edges_begin(); edge_iter != mesh->edges_end(); ++edge_iter) {
		if (mesh->is_boundary(mesh->edge(*edge_iter).from_vertex()) &&
			mesh->is_boundary(mesh->edge(*edge_iter).to_vertex())) {
			boolEProp_bdy[*edge_iter] = true;
		}
		else {
			boolEProp_bdy[*edge_iter] = false;
		}

	}

	//查找奇异边 并对其进行一定方向排序
	std::pair<OpenVolumeMesh::EdgeIter, OpenVolumeMesh::EdgeIter> pair_edge = mesh->edges();

	//奇异边记序
	uint32_t now_s_edge_index = 0;
	uint16_t now_s_edge_valance = 0;
	bool is_singular = false;

	for (OpenVolumeMesh::EdgeIter edge_iter = pair_edge.first; edge_iter != pair_edge.second; ++edge_iter) {
		//跳过已遍历边
		if (boolEProp_isChecked[*edge_iter]) continue;
		is_singular = false;
		//判断是否是奇异边
		//TODO:先试一下 后面可能会改掉
		uint16_t now_edge_valance = mesh->valence(*edge_iter);

		//TODO:找错误
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
		//TODO: 这边不要直接标记
		boolEProp_isChecked[*edge_iter] = true;
		//判断边是否在边界上
		if (boolEProp_bdy[*edge_iter]) {
			//在边界上

			if (now_edge_valance != EDGE_BDY_REGULAR_COUNT)
				is_singular = true;
			else
				is_singular = false;

		}
		else {
			//在内部

			if (now_edge_valance != EDGE_INNER_REGULAR_COUNT)
				is_singular = true;
			else
				is_singular = false;
		}

		//非奇异边 遍历下次
		if (!is_singular) continue;

		OpenVolumeMesh::OpenVolumeMeshEdge edge = mesh->edge(*edge_iter);

		//对close的处理 标记是否边全在边界
		bool is_always_bdy = boolEProp_bdy[*edge_iter];
		VertexHandle now_v_handle = edge.from_vertex();
		//记录下次
		bool first = true;

		//创建其中需要填充的内容 
		//SingularEdge(HexMeshV3f *mesh, std::vector<VertexHandle> ver_vector, std::vector<EdgeHandle> edge_vector,VertexHandle endv1 , VertexHandle endv2) 

		//分配空间
		std::vector<VertexHandle> ver_vector;
		std::vector<EdgeHandle> edge_vector;
		VertexHandle end_v_from = edge.from_vertex();
		VertexHandle end_v_to = edge.to_vertex();

		//插入当前终结点
		ver_vector.push_back(end_v_from);
		ver_vector.push_back(end_v_to);
		//插入当前遍历边
		edge_vector.push_back(*edge_iter);

		//存在奇异边序号+1
		++now_s_edge_index;
		//达到边界标号
		bool b_meet_bdy_from = false;
		bool b_meet_bdy_to = false;
		//to方向和from方向
		do {//循环两个方向
			now_v_handle = (first ? edge.from_vertex() : edge.to_vertex());

			//创建一个singularEdge

			//TODO:该处循环终止 改为达到该处没有可延伸边截止 判断
			do {//拓展单方向延伸边
				//TODO:以后改掉这个部分 不用goto
			START_NEW_EDGE_PART:
				std::pair<OpenVolumeMesh::VertexOHalfEdgeIter, OpenVolumeMesh::VertexOHalfEdgeIter> _v_e_pair =
					mesh->outgoing_halfedges(now_v_handle);
				
				//TODO:加上对当前遍历点的更改 now_v_handle
				OpenVolumeMesh::VertexOHalfEdgeIter vohe_iter = _v_e_pair.first;
				for (; vohe_iter != _v_e_pair.second; ++vohe_iter) {
					//跳过已遍历边
					if (boolEProp_isChecked[mesh->edge_handle(*vohe_iter)]) {
						continue;
					}
					//标记属性
					//TODO:是不是应该改掉 这里添加属性标记早了 可能是奇异边 但度数不一样
					//boolEProp_isChecked[mesh->edge_handle(*vohe_iter)] = true;

					//是已遍历起始边 跳过
					if (mesh->edge_handle(*vohe_iter) == *edge_iter) continue;
					//不是奇异边一部分 跳过
					//TODO:改 可能与起始的内部外部类型不同
					
					//原始在边界还是内部
					//if (is_always_bdy) {
					//	// 判断当前边是否在边界 是否度数相同
					//	if (boolEProp_bdy[mesh->edge_handle(*vohe_iter)]) {
					//		if (mesh->valence(mesh->edge_handle(*vohe_iter)) != now_edge_valance) continue;
					//	}
					//	else {
					//		//在内部 从外部到内部 跳过
					//		continue;
					//	}
					//}
					//else {
					//	if (boolEProp_bdy[mesh->edge_handle(*vohe_iter)]) {
					//		//在外部 外部到内部 跳过
					//		continue;
					//	}
					//	else {
					//		//内部到内部
					//		if (mesh->valence(mesh->edge_handle(*vohe_iter)) != now_edge_valance) continue;
					//	}

					//}
					if (is_always_bdy == boolEProp_bdy[mesh->edge_handle(*vohe_iter)]) {
						if (mesh->valence(mesh->edge_handle(*vohe_iter)) != now_edge_valance) continue;
					}
					else {
						continue;
					}
					

					//添加属性标记
					boolEProp_isChecked[mesh->edge_handle(*vohe_iter)] = true;
					//延伸
					//填充集合 判断终点

					OpenVolumeMesh::OpenVolumeMeshEdge edge_next = mesh->edge(mesh->edge_handle(*vohe_iter));
					//只要存在一条在内部的部分就非外部环
					is_always_bdy = is_always_bdy ? (boolEProp_bdy[mesh->edge_handle(*vohe_iter)]) : false;
					//判断当前方向 选择前后顺序
					if (first) {//from
								//1、判定成环
								//判断当前遍历边的终结点是否与起始点end_v_to相接 now_v_handl的另一点

						if ((now_v_handle =
							(edge_next.from_vertex() == now_v_handle) ? edge_next.to_vertex() : edge_next.from_vertex())
							== end_v_to) {
							//是否成外环
							if (is_always_bdy) {
								//添加最后一条边和点
								ver_vector.insert(ver_vector.begin(),end_v_to);
								edge_vector.insert(edge_vector.begin(),mesh->edge_handle(*vohe_iter));
								//成外环 对其中所有有效边进行标记处理
								for (std::vector<EdgeHandle>::iterator ed_iter = edge_vector.begin();
									ed_iter != edge_vector.end(); ++ed_iter) {

									//插入数据结构 做标记
									//标记奇异边序号属性
									intEProp_sigular[*ed_iter] = -now_s_edge_index;

								}
								//插入数据结构中
								se_vector.push_back(SingularEdge(mesh, ver_vector, edge_vector, *(ver_vector.begin()), *(ver_vector.end() - 1), true));


							}
							else {//成内部环
								  //添加最后一个点和边
								ver_vector.insert(ver_vector.begin() ,end_v_to);
								edge_vector.insert(edge_vector.begin() ,mesh->edge_handle(*vohe_iter));
								//成外环 对其中所有有效边进行标记处理
								for (std::vector<EdgeHandle>::iterator ed_iter = edge_vector.begin();
									ed_iter != edge_vector.end(); ++ed_iter) {

									//插入数据结构 做标记
									//标记奇异边序号属性
									intEProp_sigular[*ed_iter] = -now_s_edge_index;

								}
								//插入数据结构中
								se_vector.push_back(SingularEdge(mesh, ver_vector, edge_vector, *(ver_vector.begin()), *(ver_vector.end() - 1),true));

							}
							//跳出 该奇异边结束
							goto OUTTER;
						}

						//2、判定非环形到边界
						edge_vector.insert(edge_vector.begin(), mesh->edge_handle(*vohe_iter));
						ver_vector.insert(ver_vector.begin(), now_v_handle);
						//重置下个遍历点 TODO:不需要 在最开始的判断中就做掉了


						//判断是否到达边界 跳转to方向 在to方向记录所有结果
						if (boolVProp_bdy[now_v_handle]) {
							//改标记 到达边界
							b_meet_bdy_from = true;
							//到达边界 跳转to方向

							goto OUTER_IN;
						}
						else {
							//未到达边界

							//TODO:后面务必去掉 这个goto
							//继续遍历该奇异边上下条边
							goto START_NEW_EDGE_PART;
						}
					}
					else {//to
						  //1、判定成环
						  //判断当前遍历边的终结点是否与起始点end_v_to相接 now_v_handl的另一点

						//这里其实不必要的 画蛇添足````
						if ((now_v_handle =
							(edge_next.to_vertex() == now_v_handle) ? edge_next.from_vertex() : edge_next.to_vertex())
							== end_v_from) {
							//是否成外环
							if (is_always_bdy) {
								//添加最后一条边和点
								ver_vector.push_back(end_v_from);
								edge_vector.push_back(mesh->edge_handle(*vohe_iter));
								//成外环 对其中所有有效边进行标记处理
								for (std::vector<EdgeHandle>::iterator ed_iter = edge_vector.begin();
									ed_iter != edge_vector.end(); ++ed_iter) {

									//插入数据结构 做标记
									//标记奇异边序号属性
									intEProp_sigular[*ed_iter] = -now_s_edge_index;

								}
								//插入数据结构中
								se_vector.push_back(SingularEdge(mesh, ver_vector, edge_vector, *(ver_vector.begin()), *(ver_vector.end() - 1),true));


							}
							else {//成内部环
								  //添加最后一个点和边
								ver_vector.push_back(end_v_from);
								edge_vector.push_back(mesh->edge_handle(*vohe_iter));
								//成外环 对其中所有有效边进行标记处理
								for (std::vector<EdgeHandle>::iterator ed_iter = edge_vector.begin();
									ed_iter != edge_vector.end(); ++ed_iter) {

									//插入数据结构 做标记
									//标记奇异边序号属性
									intEProp_sigular[*ed_iter] = -now_s_edge_index;

								}
								//插入数据结构中
								se_vector.push_back(SingularEdge(mesh, ver_vector, edge_vector, *(ver_vector.begin()), *(ver_vector.end() - 1),true));

							}
							//跳出 该奇异边结束
							goto OUTTER;
						}

						//2、判定非环形到边界
						edge_vector.push_back( mesh->edge_handle(*vohe_iter));
						ver_vector.push_back( now_v_handle);
						//重置下个遍历点 TODO:不需要 在最开始的判断中就做掉了


						//判断是否到达边界 跳转to方向 在to方向记录所有结果
						if (boolVProp_bdy[now_v_handle]) {
							//做标记 到达边界
							b_meet_bdy_to = true;
							//到达边界 跳出最内层循环

							break;
						}
						else {
							//未到达边界

							//TODO:后面务必去掉 这个goto
							//继续遍历该奇异边上下条边
							goto START_NEW_EDGE_PART;
						}
					}

				}

				//1、from方向到边界 to未开始
				//2、from方向到边界 to在内部终结
				//3、from方向到边界 to到边界
				//4、from方向在内部终止 to未开始
				//5、from方向在内部终止 to到达边界
				//6、-------几乎不可能：from方向和to方向都在内部终结

				if (b_meet_bdy_from) {//from到边界
					
					if (first) {//to未开始
						//手动跳转到to方向
						goto OUTER_IN; 
					}
					else {//to在边界或者内部终结
						if (b_meet_bdy_to) {//to到达边界
							//填充singular_edge 跳出
							for (std::vector<EdgeHandle>::iterator com_edge_iter = edge_vector.begin();
								com_edge_iter != edge_vector.end(); ++com_edge_iter) {
								intEProp_sigular[*com_edge_iter] = now_s_edge_index;
							}
							//插入数据结构中
							se_vector.push_back(SingularEdge(mesh, ver_vector, edge_vector, *(ver_vector.begin()), *(ver_vector.end() - 1),false));
							//跳出寻找下一条完整奇异边
							goto OUTTER;
						}	
						else if ((*vohe_iter) == *(_v_e_pair.second)) {//to内部终结
																	   //填充singular_edge 跳出
							for (std::vector<EdgeHandle>::iterator com_edge_iter = edge_vector.begin();
								com_edge_iter != edge_vector.end(); ++com_edge_iter) {
								intEProp_sigular[*com_edge_iter] = now_s_edge_index;
							}
							//插入数据结构中
							se_vector.push_back(SingularEdge(mesh, ver_vector, edge_vector, *(ver_vector.begin()), *(ver_vector.end() - 1),false));
							//跳出寻找下一条完整奇异边
							goto OUTTER;
						}
						
					}
					
				}
				else if ((*vohe_iter) == *(_v_e_pair.second)) {//from未到边界且from在内部终止
					if (first) {//from在内部终结 to未开始
						goto OUTER_IN;
					}

					if (b_meet_bdy_to) {//to方向到达边界
						//填充singular_edge 跳出
						for (std::vector<EdgeHandle>::iterator com_edge_iter = edge_vector.begin();
							com_edge_iter != edge_vector.end(); ++com_edge_iter) {
							intEProp_sigular[*com_edge_iter] = now_s_edge_index;
						}
						//插入数据结构中
						se_vector.push_back(SingularEdge(mesh, ver_vector, edge_vector, *(ver_vector.begin()), *(ver_vector.end() - 1),false));
						
						//跳出寻找下一条完整奇异边
						goto OUTTER;
					}
					else {//to在内部终结 （正常情况下是不需要的）
						//填充singular_edge 跳出
						for (std::vector<EdgeHandle>::iterator com_edge_iter = edge_vector.begin();
							com_edge_iter != edge_vector.end(); ++com_edge_iter) {
							intEProp_sigular[*com_edge_iter] = now_s_edge_index;
						}
						//插入数据结构中
						se_vector.push_back(SingularEdge(mesh, ver_vector, edge_vector, *(ver_vector.begin()), *(ver_vector.end() - 1),false));

						//跳出寻找下一条完整奇异边
						goto OUTTER;
					}

				}

				
			} while (true);//拓展单方向延伸边

		OUTER_IN:
			//改点下次跳出
			first = !first;
		} while (!first); //循环两个方向
	OUTTER:
		std::cout << "s_edge index:" << now_s_edge_index << " is_always_bdy:" << is_always_bdy <<
			" edge_count:"<< (*(se_vector.end() - 1)).edge_vector.size()<<
			" from_point:" << (int)((*(se_vector.end() - 1)).ver_vector[0].idx()) <<
			" to_point:" << (int)((*((*(se_vector.end() - 1)).ver_vector.end()-1)).idx())<<"\n";
		
	}//遍历网格中每条边

	//TODO:测一下
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

//找奇异边部分以半边模式为标准
void SingularObject::FindSingularObject(bool b_need_ver, bool b_need_edge) {
	if (!B_READY) return;

	//查找奇异点
	if (b_need_ver) {

		//添加奇异点属性记录
		OpenVolumeMesh::VertexPropertyT<bool> boolVProp_sigular = mesh->request_vertex_property<bool>("IsSigularPoint");
		OpenVolumeMesh::VertexPropertyT<bool> boolVProp_bdy = mesh->request_vertex_property<bool>("IsVeOnBoundy");

		//声明持久化
		mesh->set_persistent(boolVProp_bdy);
		mesh->set_persistent(boolVProp_sigular);


		for (OpenVolumeMesh::VertexIter ve_it = mesh->vertices_begin(); ve_it != mesh->vertices_end(); ve_it++) {
			//std::pair<OpenVolumeMesh::VertexVertexIter, OpenVolumeMesh::VertexVertexIter> pair = mesh->vertex_vertices(*ve_it);
			boolVProp_bdy[*ve_it] = mesh->is_boundary(*ve_it);
			int valence = boolVProp_bdy[*ve_it] ? VERTICE_BDY_REGULAR_COUNT : VERTICE_INNER_REGULAR_COUNT;

			if (mesh->valence(*ve_it) != valence) {
				//奇异点
				vector_singular_vertex_handles.push_back(*ve_it);
				//添加属性记录
				boolVProp_sigular[*ve_it] = true;

			}
			else {
				//非奇异点
				boolVProp_sigular[*ve_it] = false;
			}
		}
	}
	if (b_need_edge) {

		//添加奇异边属性记录

		//OpenVolumeMesh::EdgePropertyT<bool> boolEProp_sigular = mesh->request_edge_property<bool>("IsSingularEdge");
		//边界标记
		OpenVolumeMesh::EdgePropertyT<bool> boolEProp_bdy = mesh->request_edge_property<bool>("IsEdgeOnBoundy");
		//奇异边属性 序号为0的边为规则边 序号n为编号n的
		OpenVolumeMesh::EdgePropertyT<int> intEProp_sigular = mesh->request_edge_property<int>("SingularEdgeIndex");

		//声明持久化
		mesh->set_persistent(intEProp_sigular);
		mesh->set_persistent(boolEProp_bdy);

		//检测是否遍历过的属性
		OpenVolumeMesh::EdgePropertyT<bool> boolEProp_isChecked = mesh->request_edge_property<bool>("IsChecked");
		for (OpenVolumeMesh::EdgeIter e_iter = mesh->edges_begin(); e_iter != mesh->edges_end(); ++e_iter) {
			boolEProp_isChecked[*e_iter] = false;
		}

		int d_index = 0;

		//跳转标记
		bool JUMP2NEXT = false;
		for (OpenVolumeMesh::EdgeIter ed_it = mesh->edges_begin(); ed_it != mesh->edges_end(); ed_it++) {
			JUMP2NEXT = false;

			//判断是否已经遍历过
			if (boolEProp_isChecked[*ed_it]) {
				continue;
			}
			boolEProp_bdy[*ed_it] = mesh->is_boundary(*ed_it);
			boolEProp_isChecked[*ed_it] = true;
			int valence = boolEProp_bdy[*ed_it] ? EDGE_BDY_REGULAR_COUNT : EDGE_INNER_REGULAR_COUNT;
			//当前遍历边的维度
			int valence_p = 0;
			if ((valence_p = mesh->valence(*ed_it)) != valence) {//不规则边
																 //奇异边
				vector_singular_edge_handles.push_back(*ed_it);
				//添加属性记录
				intEProp_sigular[*ed_it] = ++d_index;

				//对连续相同valence的边串联起来
				//edge为当前遍历边的上一条边
				Edge edge = mesh->edge(*ed_it);

				//记录起始边 判断是开放的还是闭合的
				OpenVolumeMesh::EdgeHandle start_edge_handler = *ed_it;

				//起始终点
				OpenVolumeMesh::VertexHandle ve_from_handler = edge.from_vertex();
				OpenVolumeMesh::VertexHandle ve_to_handler = edge.to_vertex();



				//通过两终点遍历到下一条边
				//通过面迭代到下一条边
				//OpenVolumeMesh::HalfEdgeHandle he_handler = mesh->halfedge(ve_from_handler, ve_to_handler);
				//OpenVolumeMesh::HalfEdgeHalfFaceIter hf_hf_iter = mesh->halfedge_halffaces(he_handler).first;

				//当前迭代边
				OpenVolumeMesh::OpenVolumeMeshEdge edge_now = edge;
				//--->
				//该处用to点是否在边界上作为终止是不对的····  应该是再没有其他相同度数的奇异边连接在该终点上
				short count_link_s_line = 0;
				while (true) {
					//判断是否可以跳出to方向遍历
					count_link_s_line = 0;
					//从内部延伸到边界的情况需要改变valence_p标准 边界上边的判定valence为3
					for (OpenVolumeMesh::VertexOHalfEdgeIter vohe_it_init = mesh->outgoing_halfedges(ve_to_handler).first;
						vohe_it_init != mesh->outgoing_halfedges(ve_to_handler).second; ++vohe_it_init) {
						if (mesh->valence(mesh->edge_handle(*vohe_it_init)) !=
							(mesh->is_boundary(*vohe_it_init) ? EDGE_BDY_REGULAR_COUNT : EDGE_INNER_REGULAR_COUNT)) {
							if (mesh->valence(mesh->edge_handle(*vohe_it_init)) == valence_p) {
								++count_link_s_line;
							}
						}

					}
					//仅有一条相同度数边与之相连即可判定无法延伸 跳出
					if (count_link_s_line == 1) {
						break;
					}


					OpenVolumeMesh::VertexOHalfEdgeIter vohe_iter = mesh->outgoing_halfedges(ve_to_handler).first;
					while (vohe_iter != mesh->outgoing_halfedges(ve_to_handler).second) {//while start
						edge_now = mesh->halfedge(*vohe_iter);
						if (edge_now.to_vertex().idx() != ve_from_handler.idx()) {//判两终点序号和是否相同 其他边的情况  到底这b玩意是怎么半边映射到边的····醉了

							if (mesh->valence(mesh->edge_handle(*vohe_iter)) == valence_p) {//度是否一致 奇异边可以继续延展下去

								if (boolEProp_isChecked[mesh->edge_handle(*vohe_iter)]) {//已经遍历过
																						 //对属性数组中所有的该序号下的置负
									for (OpenVolumeMesh::EdgeIter ed_iter = mesh->edges_begin(); ed_iter != mesh->edges_end(); ++ed_iter) {
										if (intEProp_sigular[*ed_iter] == d_index) {
											intEProp_sigular[*ed_iter] = -d_index;
										}
									}
									//跳出循环
									JUMP2NEXT = true;
									break;
								}
								else {
									vector_singular_edge_handles.push_back(mesh->edge_handle(*vohe_iter));
								}

								//添加属性序号
								intEProp_sigular[mesh->edge_handle(*vohe_iter)] = d_index;
								std::cout << "select:" << (*vohe_iter).idx() << " id:" << d_index << std::endl;
								//添加属性记录 此边已遍历过
								boolEProp_bdy[mesh->edge_handle(*vohe_iter)] = mesh->is_boundary(mesh->edge_handle(*vohe_iter));
								boolEProp_isChecked[mesh->edge_handle(*vohe_iter)] = true;

								//重置当前遍历边 为下次遍历准备
								if (edge_now.from_vertex().idx() == edge.to_vertex().idx()) {//正序
									ve_from_handler = edge_now.from_vertex();
									ve_to_handler = edge_now.to_vertex();
								}
								else {
									ve_from_handler = edge_now.to_vertex();
									ve_to_handler = edge_now.from_vertex();
								}

								//度一致 跳到正方向下一条边 跳出内部的while循环
								//移动前一条奇异边的指针
								edge = edge_now;
								break;
							}
							else {
								//不能在这贸然序号增加 还有逆方向的遍历没有执行
								//++d_index;
								++vohe_iter;
							}


						}
						else {//与原始边相逆的边 
							  //遍历连接该点的下条边
							++vohe_iter;

						}
					}//对终结点to的邻接边 while end


					if (JUMP2NEXT) {//closed型奇异边的情况
						break;
					}
					else {//1、是奇异边且未闭合的情况 则转而到正向下条边
						  //2、是奇异边且正向没有下一条维度相同奇异边

					}
				}//对to方向的遍历结束

				 //如果跳转标记为true的话 需要跳过逆方向的搜索，对环形闭合边没有意义
				if (JUMP2NEXT) {
					continue;
				}

				//下面是对逆方向的from开始遍历
				//还原指向前一条边的指针 还原终点
				edge = mesh->edge(*ed_it);
				ve_from_handler = edge.from_vertex();
				ve_to_handler = edge.to_vertex();

				while (true) {

					//判断是否可以跳出from方向遍历
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
					//仅有一条相同度数边与之相连即可判定无法延伸 跳出
					if (count_link_s_line == 1) {
						break;
					}
					OpenVolumeMesh::VertexOHalfEdgeIter vohe_iter = mesh->outgoing_halfedges(ve_from_handler).first;
					while (vohe_iter != mesh->outgoing_halfedges(ve_from_handler).second) {//while start
						edge_now = mesh->halfedge(*vohe_iter);
						if (edge_now.to_vertex().idx() != ve_to_handler.idx()) {//判两终点序号和是否相同 其他边的情况  到底这b玩意是怎么半边映射到边的····醉了
							if (mesh->valence(mesh->edge_handle(*vohe_iter)) == valence_p) {//度是否一致 奇异边可以继续延展下去
																							//以防万一还是加上已遍历边的过滤
																							/*if (boolEProp_isChecked[mesh->edge_handle(*vohe_iter)]) {
																							std::cout << "我TM到底看看有没有这种情况····";
																							}*/
								vector_singular_edge_handles.push_back(mesh->edge_handle(*vohe_iter));

								//添加属性序号
								intEProp_sigular[mesh->edge_handle(*vohe_iter)] = d_index;
								std::cout << "select:" << (*vohe_iter).idx() << " id:" << d_index << std::endl;
								//添加属性记录 此边已遍历过
								boolEProp_bdy[mesh->edge_handle(*vohe_iter)] = mesh->is_boundary(mesh->edge_handle(*vohe_iter));
								boolEProp_isChecked[mesh->edge_handle(*vohe_iter)] = true;

								//重置当前遍历边 为下次遍历准备
								if (edge_now.to_vertex().idx() == edge.from_vertex().idx()) {//正序
									ve_from_handler = edge_now.from_vertex();
									ve_to_handler = edge_now.to_vertex();
								}
								else {
									ve_from_handler = edge_now.to_vertex();
									ve_to_handler = edge_now.from_vertex();
								}

								//度一致 跳到正方向下一条边 跳出内部的while循环
								//移动前一条奇异边的指针
								edge = edge_now;
								break;

							}
							else {
								//不能在这贸然序号增加 还有逆方向的遍历没有执行
								//++d_index;
								++vohe_iter;
							}

						}
						else {//与原始边相逆的边 
							  //遍历连接该点的下条边
							++vohe_iter;

						}
					}//while end 对终结点from的邻接边

				}//对from方向的遍历结束
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


//初始化网格
bool SingularObject::InitMesh() {
	return true;
}

std::set<HalfFaceHandle, compare_OVM_For_Singular> SingularEdge::FindSeparatedFaceHandles()
{
	//填充未完成返回空集合
	if (!this->is_filled_complish)
		return std::set<HalfFaceHandle, compare_OVM_For_Singular>();

	EdgeHandle growing_edge = mesh->InvalidEdgeHandle;

	VertexHandle handle_point[2] = {endPoint_1,endPoint_2}; 
	for (int i = 0; i < 2; ++i) {
		//TODO:要大改
		if ((mesh->edge(edge_vector[0]).from_vertex() == handle_point[i]) || (mesh->edge(edge_vector[0]).to_vertex() == handle_point[i]))
			growing_edge = edge_vector[0];
		if ((mesh->edge(*(edge_vector.end() - 1)).from_vertex() == handle_point[i]) || (mesh->edge(*(edge_vector.end() - 1)).to_vertex() == handle_point[i]))
			growing_edge = *(edge_vector.end() - 1);

		std::pair<OpenVolumeMesh::HalfEdgeHalfFaceIter, OpenVolumeMesh::HalfEdgeHalfFaceIter> he_hf_pair0 = mesh->halfedge_halffaces(mesh->halfedge_handle(growing_edge, 0));
		std::pair<OpenVolumeMesh::HalfEdgeHalfFaceIter, OpenVolumeMesh::HalfEdgeHalfFaceIter> he_hf_pair1 = mesh->halfedge_halffaces(mesh->halfedge_handle(growing_edge, 1));

		std::set<HalfFaceHandle, compare_OVM_For_Singular> set_hf;

		//插入集合
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
	// 结合分割面确定单元
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
	//TODO:临时改为直接把o_ver邻接cell全加入
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
//确定该点位于该cell的哪个位置
unsigned char SingularEdge::JudgeVer2CellIndex(CellHandle cell,VertexHandle point)
{
	CellHandle temp_cell = cell;
	//TODO: idx 9
	//TODO:测试用
	std::pair<OpenVolumeMesh::CellVertexIter, OpenVolumeMesh::CellVertexIter> pair_cv = mesh->cell_vertices(temp_cell);
	for (OpenVolumeMesh::CellVertexIter cv_iter = pair_cv.first; cv_iter != pair_cv.second; ++cv_iter) {
		std::cout << "vertex:" << (int)((*cv_iter).idx())<<"\n";
	}

	std::set<HalfFaceHandle, compare_OVM_For_Singular> set_hf;
	for (unsigned char uc_i = 0; uc_i != 8; ++uc_i) {
		//cell_vertex里是按idx大小顺序排列的 改掉默认顺序  U_RELOCATED_CELL_OUT_INDEX
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
