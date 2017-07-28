#include "BaseComplexSimplify.h"
#include "FileHandler.h"



static bool cmp(const VertexHandle & t1, const VertexHandle & t2)
{
	return t1.idx() < t2.idx();
}


void showTempMsg(HexMeshV3f* mesh,BC_8 last_8_ell) {
	//查看一下上个单元的信息
	for (int i = 0; i < 8; ++i)
	{
		std::pair<OpenVolumeMesh::CellVertexIter, OpenVolumeMesh::CellVertexIter> hex_pair = mesh->cell_vertices(last_8_ell.bcs[i]);
		std::cout << "points: ";
		for (OpenVolumeMesh::CellVertexIter hex_iter = hex_pair.first; hex_iter != hex_pair.second; ++hex_iter) {
			std::cout << (int)(*hex_iter).idx() << " ";

		}
		std::cout << "\n";
		for (unsigned char j = 0; j < 6; ++j) {
			std::cout << (int)(mesh->face_handle(mesh->get_oriented_halfface(j, last_8_ell.bcs[i])).idx()) << " ";
		}
		std::cout << "\n";
	}
}

//选择目标位置点的统一方法
VertexHandle Get_Position_Vertex(HexMeshV3f* mesh, CellHandle cell, unsigned char index) {
	CellHandle temp_cell = cell;

	//cell_vertex里是按idx大小顺序排列的 改掉默认顺序  U_RELOCATED_CELL_OUT_INDEX
	VertexHandle ve_arr[4];
	int cout_arr[4] = { 0 };

	uint16_t now_i = 0;
	std::pair<OpenVolumeMesh::HalfFaceVertexIter, OpenVolumeMesh::HalfFaceVertexIter> pair_hfv1 =
		mesh->halfface_vertices(mesh->get_oriented_halfface(U_RELOCATED_CELL_OUT_INDEX_[index][0], temp_cell));
	now_i = 0;
	for (OpenVolumeMesh::HalfFaceVertexIter hfv_iter = pair_hfv1.first; hfv_iter != pair_hfv1.second; ++hfv_iter) {
		ve_arr[now_i++] = *hfv_iter;
	}

	std::pair<OpenVolumeMesh::HalfFaceVertexIter, OpenVolumeMesh::HalfFaceVertexIter> pair_hfv2 =
		mesh->halfface_vertices(mesh->get_oriented_halfface(U_RELOCATED_CELL_OUT_INDEX_[index][1], temp_cell));
	for (OpenVolumeMesh::HalfFaceVertexIter hfv_iter = pair_hfv2.first; hfv_iter != pair_hfv2.second; ++hfv_iter) {
		for (int i = 0; i < 4; ++i) {
			if (ve_arr[i].idx() == (*hfv_iter).idx()) {
				++cout_arr[i];
			}
		}
	}
	std::pair<OpenVolumeMesh::HalfFaceVertexIter, OpenVolumeMesh::HalfFaceVertexIter> pair_hfv3 =
		mesh->halfface_vertices(mesh->get_oriented_halfface(U_RELOCATED_CELL_OUT_INDEX_[index][2], temp_cell));

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

	return ve_arr[max_i];
}


BaseComplexSimplify::BaseComplexSimplify(HexMeshV3f * mesh, std::set<HalfFaceHandle, compare_OVM> hf_all, std::set<FaceHandle, compare_OVM> f_all, std::set<BaseComplex, compare_BaseComplex> baseComplexSet)
{
	this->mesh = mesh;
	this->f_all_set = f_all;
	this->hf_all_set = hf_all;
	this->bc_set = baseComplexSet;
}

std::vector< std::vector<BC_8> > BaseComplexSimplify::HandleBCSimplify(simplify_type type, simplify_type bdy_type)
{
	/*添加标记属性*/
	OpenVolumeMesh::CellPropertyT<int> cells_checked_index = mesh->request_cell_property<int>();
	int re_cell_count_temp;
	//目前合并只支持这种模式
	//添加cell标记属性
	OpenVolumeMesh::CellPropertyT<bool> boolCellChecked = mesh->request_cell_property<bool>("boolCellChecked_BC");
	OpenVolumeMesh::VertexPropertyT<bool> boolVexChecked = mesh->request_vertex_property<bool>("boolVertexChecked_BC");

	std::vector< std::vector<BC_8> > v_cell_vector;
	if (type == simplify_type::re_8_1) {
		for (std::set<BaseComplex, compare_BaseComplex>::iterator bc_all_iter = bc_set.begin();
			bc_all_iter != bc_set.end(); ++bc_all_iter) {
			BaseComplex bc = (*bc_all_iter);
			std::set<CellHandle, compare_OVM> cell_set = bc.getCellSet();
			//re_cell_count_temp = 0;

			//不加上约束条件 让自己递归终止退出

			
			//对单个bc的合并结果集合
			std::vector<BC_8> vector_bc_8;
			int dsada = (*bc_all_iter).Idx();
			
			HandlerBCSimplify_OneBC(bc,
				boolVexChecked,
				boolCellChecked,
				&vector_bc_8,
				BC_8(), INVALID_ORIENTATION);

			std::cout<<"============================================BaseComplex.idx:"<<(*bc_all_iter).Idx()<<"==CellSet:"<<bc.getCellSet().size()<<"\n";
			v_cell_vector.push_back(vector_bc_8); 
		}
	}

	HexMeshV3f re_mesh;
	re_mesh.enable_bottom_up_incidences(true);
	//直接进行网格生成
	int i = 0;
	for (std::vector< std::vector<BC_8> >::iterator iter = v_cell_vector.begin(); iter != v_cell_vector.end(); ++iter) {
		std::cout << ++i << "\n";
		for (std::vector<BC_8>::iterator bc_iter = (*iter).begin(); bc_iter != (*iter).end(); ++bc_iter) {
			std::vector<VertexHandle> add_in_v;
			for (int i = 0; i < 8; ++i) {
				if ((*bc_iter).vs[i].idx() == -1) {
					std::cout << "wocao";
				}
				add_in_v.push_back(re_mesh.add_vertex(mesh->vertex((*bc_iter).vs[i]))); 
			}
			
			re_mesh.add_cell(add_in_v);
		}
	}
	//该方法生成有误 实验下
	//
	//OpenVolumeMesh::VertexPropertyT<int> vprop = mesh->request_vertex_property<int>("count_temp");
	//for (std::vector< std::vector<BC_8> >::iterator iter = v_cell_vector.begin(); iter != v_cell_vector.end(); ++iter) {
	//	std::cout << ++i << "\n";
	//	for (std::vector<BC_8>::iterator bc_iter = (*iter).begin(); bc_iter != (*iter).end(); ++bc_iter) {
	//		std::vector<VertexHandle> add_in_v;
	//		for (int i = 0; i < 8; ++i) {
	//			if ((*bc_iter).vs[i].idx() == -1) {
	//				std::cout << "wocao";
	//			}
	//			vprop[(*bc_iter).vs[i]] += 1;
	//		}

	//		//re_mesh.add_cell(add_in_v);
	//	}
	//}
	//int max = 0,max_idx = -1;
	//for (OpenVolumeMesh::VertexIter ite =mesh->vertices_begin(); ite != mesh->vertices_end(); ++ite) {
	//	if (vprop[*ite] > max) {
	//		max = vprop[*ite];
	//		max_idx = (*ite).idx();
	//	}
	//}
	//std::cout << "\nmax:" << max << "\nmax_id:" << max_idx;
	writeMesh2File("F://","test1600.ovm",&re_mesh);

	return v_cell_vector;
}


//前面base-complex归并递归时候栈炸了
void BaseComplexSimplify::HandlerBCSimplify_OneBC(BaseComplex & bc, OpenVolumeMesh::VertexPropertyT<bool>& boolVexChecked,
	OpenVolumeMesh::CellPropertyT<bool>& boolCellChecked, std::vector<BC_8>* vector_bc8, BC_8 last_8_cell, const unsigned char from_oriention)
{
	/*std::pair<OpenVolumeMesh::HexVertexIter, OpenVolumeMesh::HexVertexIter> hex_pair = mesh->hex_vertices(*mesh->cells_begin());
	std::pair<OpenVolumeMesh::CellVertexIter, OpenVolumeMesh::CellVertexIter> oc = mesh->cell_vertices(*mesh->cells_begin());
	for (OpenVolumeMesh::CellVertexIter hx = oc.first; hx != oc.second; ++hx) {
		VertexHandle v = *hx;
		int j = 0;
	}*/
	

	//更改初始化策略 改为每个basecomplex的内置的单元来起始
	if (!last_8_cell.bc_is_Init()) {//上个单元未初始化,为空单元

		//在bc内部点集中寻找未遍历结点 拓展8个单元
		BC_8 now_bc_8;
		
		//判断是否还存在未遍历的内部点
		bool can_jmp_out = false;
		int temp_count_bc = 0;
			
		CellHandle origination_cell = bc.get_origination_cell();
		unsigned char origination_index = bc.get_origination_index();

		//TODO:出现意外情况 中途出现重新选取初始点的情况 存在特殊结构
		if (boolCellChecked[origination_cell]) {
			return;
		}

		//确定8类型合并的中心起始点
		VertexHandle origination_ver = Get_Position_Vertex(mesh,origination_cell,sc_diagonal_index[origination_index]);

		//选取其中一个未遍历点开始进行下一步合并 合并后块的顶点标志位置true
		std::pair<OpenVolumeMesh::VertexCellIter, OpenVolumeMesh::VertexCellIter> v_c_pair = mesh->vertex_cells(origination_ver);
		//判断下中心点的连接边度 是不是6
		std::cout << "valence" << (int)mesh->valence(origination_ver) << "\n";
		if (mesh->valence(origination_ver) != 6) {
			std::cout << "valence" << (int)mesh->valence(origination_ver) << "\n";
			now_bc_8.clearElem();
			//证明不能拼接8个
			//改标示 跳4个合并
			goto START_ORIGIN_4_CELL;
		}
		for (OpenVolumeMesh::VertexCellIter v_c_iter = v_c_pair.first;
			v_c_iter != v_c_pair.second; ++v_c_iter) {
			//按照原顺序插入 之后可能需要改
			now_bc_8.bc_insertCell(*v_c_iter);
			if (boolCellChecked[*v_c_iter]||!(bc.find_cell(*v_c_iter))) {
				//一旦有重复 舍弃
				now_bc_8.clearElem();
				goto START_ORIGIN_4_CELL;
			}
		}
		//对标示位更改
		for (uint16_t i_t = 0; i_t < 8; ++i_t) {
			boolCellChecked[now_bc_8.bcs[i_t]] = true;
			//对其中的点进行标志位调整 TODO :可优化
			std::pair<OpenVolumeMesh::CellVertexIter, OpenVolumeMesh::CellVertexIter> cv_pair = mesh->cell_vertices(now_bc_8.bcs[i_t]);
			for (OpenVolumeMesh::CellVertexIter cv_iter = cv_pair.first; cv_iter != cv_pair.second; ++cv_iter) {
				boolVexChecked[*cv_iter] = true;
			}

		}
				

		//按围绕顺序重排
		now_bc_8.bc_relocated_8(mesh, origination_ver);
		//showTempMsg(mesh, now_bc_8); 
		//完成now_bc_8填充 跳出
		can_jmp_out = true;
		


		
		//未找到 不存在未遍历的在内部点 直接跳出 底下会有对于剩余单层的拓展方式
		/*
			遇到此种情况只有一种可能 即该bc中只有单层
		*/
		START_ORIGIN_4_CELL:
		if (!can_jmp_out) {
			//添加下个4类型起始单元
			
			//1.使用初始的边界单元，测是否有4个面在边界上
			//2.找到该单元的邻接单元 获取在bc内部的三个 置入合并
			std::set<CellHandle, compare_OVM> cell_set = bc.getCellSet();
			std::set<HalfFaceHandle, compare_OVM> hf_set_temp;
			int count = 0 ;
			
			
			count = 0; 
			std::vector<HalfFaceHandle> hf_vector = mesh->cell(origination_cell).halffaces();
			//记录边界面
			uint16_t expand_hf_arr[4] = { 0 };
			//TODO：等会改下试试 改成与总的分割面集合中查找  或者计算面的重合点集的大小
			hf_set_temp = bc.getHalfFaceSet();
			for (std::vector<HalfFaceHandle>::iterator hf_iter = hf_vector.begin(); hf_iter != hf_vector.end()&&count!=4; ++hf_iter) {
					
				if (hf_set_temp.find(*hf_iter) != hf_set_temp.end()) {
					expand_hf_arr[count++] = hf_iter - hf_vector.begin();
				}
					
			}
			if (count == 4) { //跑一次跳出
				now_bc_8.bc_setType(false,true);
				//该单元在角落 找其他三个单元
				now_bc_8.bc_insertCell(origination_cell);
					
				//直接根据面来找
				HalfFaceHandle hf_expand_op_arr[2] = { NULL,NULL };
				
				uint16_t count_for_diff = 0; 
				uint16_t i_hf_arr_index = 0;
				for (uint16_t i_hf = 0; i_hf < 4; ++i_hf) {
					count_for_diff = 0; 
					for (uint16_t i_hf_2 = 0; i_hf_2 < 4; ++i_hf_2) {
							
						if (mesh->opposite_halfface_handle_in_cell(hf_vector[expand_hf_arr[i_hf_2]], origination_cell).idx() != hf_vector[expand_hf_arr[i_hf]].idx()) {
							++count_for_diff;
								
						}
					}
					if (count_for_diff-1 == 3) {
						hf_expand_op_arr[i_hf_arr_index++] = hf_vector[expand_hf_arr[i_hf]];
							
					}
				}

				if (i_hf_arr_index!=2) {
					//先标记该单元 直接退出返回 寻找下个适合的单元
					boolCellChecked[origination_cell] = true;
					std::pair<OpenVolumeMesh::CellVertexIter, OpenVolumeMesh::CellVertexIter> pair_hexcell_ce = mesh->cell_vertices(origination_cell);
					for (OpenVolumeMesh::CellVertexIter hv_iter = pair_hexcell_ce.first; hv_iter != pair_hexcell_ce.second; ++hv_iter) {
						boolVexChecked[*hv_iter] = true;
					}
					now_bc_8.clearElem(); 
					return;
				}
				else {
					//找到其他三个cell
						
					now_bc_8.bc_insertCell( mesh->incident_cell(mesh->opposite_halfface_handle(mesh->opposite_halfface_handle_in_cell(hf_expand_op_arr[0],origination_cell))) );
						
					unsigned char ori_to_2 = mesh->orientation(mesh->opposite_halfface_handle_in_cell(hf_expand_op_arr[0], origination_cell),origination_cell);
					now_bc_8.bc_insertCell( mesh->incident_cell(mesh->opposite_halfface_handle(mesh->opposite_halfface_handle_in_cell(hf_expand_op_arr[1], origination_cell)))  );
						
					//查找到第四个单元
					//通过2 按3的贴合方向 找oppsite face
						
					now_bc_8.bc_insertCell( mesh->incident_cell(mesh->opposite_halfface_handle(mesh->get_oriented_halfface(ori_to_2,now_bc_8.bcs[2]))) );

					if (boolCellChecked[now_bc_8.bcs[1]] || boolCellChecked[now_bc_8.bcs[2]] || boolCellChecked[now_bc_8.bcs[3]]) {
						//有已遍历的情况，清空下次
						now_bc_8.clearElem();
						return;
					}
					CellHandle cell_temp_bool;
					//对其中点和单元设置标识
					for (int i = 0; i < 4; ++i) {
						//根据预定序列对其中单元操作（以后要添上自己的迭代器）
						cell_temp_bool = now_bc_8.bcs[i];
						boolCellChecked[cell_temp_bool] = true;
						std::pair<OpenVolumeMesh::CellVertexIter, OpenVolumeMesh::CellVertexIter> pair = mesh->cell_vertices(cell_temp_bool);
						for (OpenVolumeMesh::CellVertexIter cell_ver_iter = pair.first; cell_ver_iter != pair.second; ++cell_ver_iter) {
							boolVexChecked[*cell_ver_iter] = true;
						}
					}

					//4类型重排
					now_bc_8.bc_relocated_4(mesh, bc);
					
				}

			}
			else {
				//4类型不成立 gg
				return;
			}


			
			
		}
		
		/*
			上面已确定bc_8块  跳出if 块 到结尾位置开始拓展
		*/
		//判断是否有初始化bc_8块 无则返回
		if (!now_bc_8.bc_is_Init()) {
			return;
		}

		vector_bc8->push_back(now_bc_8);
		//TODO :打印一会删
		std::cout << "vector_bc8.size: " << vector_bc8->size() << "\t now_bc_8.isType8:" << now_bc_8.isType8() << "\t now_bc_8.ori" << (int)(now_bc_8.isType8() ? from_oriention : now_bc_8.getOriFor4()) << "\n";

		HandlerBCSimplify_OneBC(bc, boolVexChecked,
			boolCellChecked, vector_bc8, now_bc_8, now_bc_8.isType8() ? INVALID_ORIENTATION : now_bc_8.getOriFor4());
		return; 
	}
	else {//next 传值非空 通过来向拓展
		//按照六个方向拓展下个单元  考虑到单层的情况
		/*
		1、考虑到单层 
		2、分4、8两种分类
		*/
		
		//都从xf开始 避过来的方向
		


		std::vector<HalfFaceHandle> vector_halffaces;
		std::vector<CellHandle> vector_cells;
		CellHandle _8_in_temp_cell;
		std::set<CellHandle, compare_OVM> set_cell;
		bool goto_next = false; 
		bool is_4_in_next = false;

		for (unsigned char now_orientation = con_orientations[0]; now_orientation <= con_orientations[5]; ++now_orientation) {
			
			//首先避开来向的遍历
			if (from_oriention!=6&&(now_orientation == (from_oriention % 2 ? from_oriention - 1 : from_oriention + 1))) {
				continue;
			}
			goto_next = false;
			BC_8 now_bc_8;

			if (last_8_cell.isType8()) {
				//初始单元为8个
				//固定方向拓展双层

				
				//1.找到对应延伸方向的对应点(提取四个边界面求共点)

				//2.该点找v-8cell 去重四个  判断是否在内部

				//3.同样的方法拓展四个 判断是否在bc内部

				//4.递归（1）8 （2）4


				//另一种方式
				//1.找到延伸方向上的opposite halfface
				
				for (int i = 0; i < 4; ++i) {//四个面
					vector_halffaces.push_back(mesh->get_oriented_halfface(now_orientation, last_8_cell.bcs[ORI_FACES_VER[now_orientation][i]]));
				}

				//2.找到所属单元
				set_cell = bc.getCellSet();
				for (std::vector<HalfFaceHandle>::iterator _4_hf_iter = vector_halffaces.begin();
					_4_hf_iter != vector_halffaces.end(); ++_4_hf_iter) {
					_8_in_temp_cell = mesh->incident_cell(mesh->opposite_halfface_handle(*_4_hf_iter));
					//set_cell = bc.getCellSet();
					//TODO:测试删掉
					bool a = boolCellChecked[_8_in_temp_cell];
					bool b = (set_cell.find(_8_in_temp_cell) == set_cell.end());

					if (boolCellChecked[_8_in_temp_cell]||(set_cell.find(_8_in_temp_cell) == set_cell.end())) {
						//一旦存在不在内部的或者存在已经遍历单元的情况 清空vector 跳出
						vector_cells.swap(std::vector<CellHandle>());

						//因为完整的单层都不存在  需要把原来的bc8清空下  直接跳出到下个方向的遍历
						now_bc_8.clearElem();

						goto_next = true;
						break;
					}
					vector_cells.push_back(_8_in_temp_cell);
					now_bc_8.bc_insertCell(_8_in_temp_cell);
					
					
				}

				if (goto_next) {
					//做下清理工作 跳转下次
					vector_cells.swap(std::vector<CellHandle>());
					vector_halffaces.swap(std::vector<HalfFaceHandle>());
					now_bc_8.clearElem();
					continue;
				}


				//3.重复1.2.再进一层 
				vector_halffaces.swap(std::vector<HalfFaceHandle>());

				for (std::vector<CellHandle>::iterator iter_cell = vector_cells.begin();
					iter_cell != vector_cells.end(); ++iter_cell) {//四个面
					vector_halffaces.push_back(mesh->get_oriented_halfface(now_orientation, *iter_cell));
				}

				is_4_in_next = true;
				vector_cells.swap(std::vector<CellHandle>());
				for (std::vector<HalfFaceHandle>::iterator iter_hf = vector_halffaces.begin();
					iter_hf != vector_halffaces.end(); ++iter_hf) {
					_8_in_temp_cell = mesh->incident_cell(mesh->opposite_halfface_handle(*iter_hf));

					/*boolCellChecked[_8_in_temp_cell] || */
					//讲道理这里可以除掉这部分  不在边界上怎么办（四个的出现在内部？有没有可能？）
					if (boolCellChecked[_8_in_temp_cell] || set_cell.find(_8_in_temp_cell) == set_cell.end()) {
						//一旦存在不在内部的情况 清空vector 跳出
						vector_cells.swap(std::vector<CellHandle>());

						is_4_in_next = false;
						break;
					}
					vector_cells.push_back(_8_in_temp_cell);

				}

				//4.判断是否在bc内部  组合bc_8下次迭代
				if (!is_4_in_next) {//不在内部
					now_bc_8.bc_setType(false, false);
					
					//对4类型该方向迭代下个bc8
					
					//1.对其中单元标识更改
					for (int i = 0; i < 4; ++i) {
						if (now_bc_8.bcs[i] != HexMeshV3f::InvalidCellHandle) {
							boolCellChecked[now_bc_8.bcs[i]] = true;
							std::pair<OpenVolumeMesh::CellVertexIter, OpenVolumeMesh::CellVertexIter> pair_c_v = mesh->cell_vertices(now_bc_8.bcs[i]);

							for (OpenVolumeMesh::CellVertexIter c_v_iter = pair_c_v.first; c_v_iter != pair_c_v.second; ++c_v_iter) {
								boolVexChecked[*c_v_iter] = true;
							}
						}
					}
					now_bc_8.bc_relocated_4(mesh, bc);
					vector_bc8->push_back(now_bc_8);

					//TODO :打印一会删
					std::cout << "vector_bc8.size: " << vector_bc8->size() << "\t now_bc_8.isType8:" << now_bc_8.isType8() << "\t now_bc_8.ori" << (int)(now_bc_8.isType8() ? from_oriention : now_bc_8.getOriFor4()) << "\n";


					//2.迭代项
					HandlerBCSimplify_OneBC(bc, boolVexChecked, boolCellChecked, vector_bc8, now_bc_8, now_orientation);

				}
				else {//在内部

					//1.方向选择 该方向是否在边界上
					

					//2.该方向拓两层
					

					for (std::vector<CellHandle>::iterator cell_iter = vector_cells.begin();
						cell_iter != vector_cells.end(); ++cell_iter) {
						now_bc_8.bc_insertCell(*cell_iter);
					}

					//对8类型该方向迭代下个bc8

					//1.更改标识
					for (int i = 0; i < 8; ++i) {
						
						boolCellChecked[now_bc_8.bcs[i]] = true;
						std::pair<OpenVolumeMesh::CellVertexIter, OpenVolumeMesh::CellVertexIter> pair_c_v = mesh->cell_vertices(now_bc_8.bcs[i]);

						for (OpenVolumeMesh::CellVertexIter c_v_iter = pair_c_v.first; c_v_iter != pair_c_v.second; ++c_v_iter) {
							boolVexChecked[*c_v_iter] = true;
						}
						
					}
					now_bc_8.bc_relocated_8(mesh);
					vector_bc8->push_back(now_bc_8);


					//TODO :打印一会删
					std::cout << "vector_bc8.size: " << vector_bc8->size() << "\t now_bc_8.isType8:" << now_bc_8.isType8() << "\t now_bc_8.ori" << (int)(now_bc_8.isType8() ? from_oriention : now_bc_8.getOriFor4()) << "\n";


					//2.下次迭代
					HandlerBCSimplify_OneBC(bc, boolVexChecked, boolCellChecked, vector_bc8, now_bc_8, now_orientation);

				}
			}
			else {//type4 TODO:看到这

				//因为该类型是在边界上 跳过对last_8_cell 方向和相反方向的遍历
				if (now_orientation == last_8_cell.getOriFor4() || now_orientation == mesh->opposite_orientation(last_8_cell.getOriFor4())) {
					continue;
				}
				std::set<CellHandle, compare_OVM> cell_set = bc.getCellSet();

				vector_halffaces.swap(std::vector<HalfFaceHandle>());
				//1.延伸方向上的oppsite halfface 求所属的两个单元 判断是否bc内部
				std::pair<int, int> pair_index_cells = last_8_cell.bc_find2_in_ori(now_orientation);
				vector_halffaces.push_back(mesh->get_oriented_halfface(now_orientation, last_8_cell.bcs[pair_index_cells.first]));
				vector_halffaces.push_back(mesh->get_oriented_halfface(now_orientation, last_8_cell.bcs[pair_index_cells.second]));


				//清空
				vector_cells.swap(std::vector<CellHandle>());
				for (std::vector<HalfFaceHandle>::iterator hf_iter = vector_halffaces.begin(); hf_iter != vector_halffaces.end(); ++hf_iter) {

					now_bc_8.bc_insertCell(mesh->incident_cell(mesh->opposite_halfface_handle(*hf_iter)));
				}

				//出现已遍历的情况 该方向舍弃 跳到下个
				if (boolCellChecked[now_bc_8.bcs[0]] || boolCellChecked[now_bc_8.bcs[1]]
					|| cell_set.find(now_bc_8.bcs[0]) == cell_set.end() || cell_set.find(now_bc_8.bcs[1]) == cell_set.end()) {
					now_bc_8.clearElem();
					continue;
				}

				bool is_break_continue = false;
				//2.重复1.再拓一次 判断是否内部
				for (int i = 0; i < 2; ++i) {

					_8_in_temp_cell = mesh->incident_cell(mesh->opposite_halfface_handle(mesh->get_oriented_halfface(now_orientation, now_bc_8.bcs[i])));
					if (boolCellChecked[_8_in_temp_cell] || (cell_set.find(_8_in_temp_cell) == cell_set.end())||
						(_8_in_temp_cell.idx()== now_bc_8.bcs[i].idx())) {
						//存在找不到的情况 即该块只有三个或两个的情况 TODO : 暂且搁置这种情况 直接清空 之后再做处理
						
						now_bc_8.clearElem();
						vector_halffaces.swap(std::vector<HalfFaceHandle>()); 
						is_break_continue = true; 
						break;
					}
					now_bc_8.bc_insertCell(_8_in_temp_cell);
				}


				//3.组合bc_8 迭代到下次
				if (is_break_continue) {
					continue; 
				}
				else {
					
					//对标识位标志
					for (int i = 0; i < 4; ++i) {
						boolCellChecked[now_bc_8.bcs[i]] = true;
						std::pair<OpenVolumeMesh::CellVertexIter, OpenVolumeMesh::CellVertexIter> pair_hexv = mesh->cell_vertices(now_bc_8.bcs[i]);

						for (OpenVolumeMesh::CellVertexIter hex_v_iter = pair_hexv.first; hex_v_iter != pair_hexv.second; ++hex_v_iter) {
							boolVexChecked[*hex_v_iter] = true; 
						}
					}
					now_bc_8.bc_setType(false,false);
					now_bc_8.bc_relocated_4(mesh,bc);
					vector_bc8->push_back(now_bc_8);


					//TODO :打印一会删
					std::cout << "vector_bc8.size: " << vector_bc8->size() << "\t now_bc_8.isType8:" << now_bc_8.isType8() << "\t now_bc_8.ori" << (int)(now_bc_8.isType8() ? from_oriention : now_bc_8.getOriFor4())<< "\n";


					HandlerBCSimplify_OneBC(bc,boolVexChecked,boolCellChecked,vector_bc8,now_bc_8,now_orientation);
				}

			}
			//TODO :打印一会删
			//std::cout << "vector_bc8.size: " << vector_bc8->size() << "\t now_bc_8.isType8:" << now_bc_8.isType8() << "\t now_bc_8.ori" << (int)(now_bc_8.isType8() ? from_oriention : now_bc_8.getOriFor4()) << "\n";


		}
		

	}


}
void BC_8::bc_relocated_8(HexMeshV3f * mesh) {
	//首先求中心点
	//取其中一个六面体单元 看哪个点邻接8个单元在集合中 
	uint16_t count_repeat = 0; 
	std::pair<OpenVolumeMesh::CellVertexIter, OpenVolumeMesh::CellVertexIter> hv_pair = mesh->cell_vertices(this->bcs[0]);
	for (OpenVolumeMesh::CellVertexIter hv_iter = hv_pair.first; hv_iter!=hv_pair.second; ++hv_iter) {
		std::pair<OpenVolumeMesh::VertexCellIter, OpenVolumeMesh::VertexCellIter> vc_pair = mesh->vertex_cells(*hv_iter);
		for (OpenVolumeMesh::VertexCellIter vc_iter = vc_pair.first;
			vc_iter != vc_pair.second; ++vc_iter) {
			if (this->includeCell(*vc_iter)) {
				++count_repeat;
			}
		}
		if (!(count_repeat - 8)) {
			this->bc_relocated_8(mesh,*hv_iter);
			return;
		}
		count_repeat = 0 ;
	}
}
void BC_8::bc_relocated_8(HexMeshV3f * mesh,VertexHandle middle_vertex)
{
	if (this->bc_is_Init()) {
		//对其中的单位进行重排序
		//0-7 取0为初始空
		


		uint16_t *index = new uint16_t[8];
		memset(index, (char)0, sizeof(uint16_t) * 8);
		CellHandle confirm_now_cell,temp_cell;
		HalfFaceHandle hf_handle_temp;
		//用来暂存面排序的vector
		std::set<unsigned char> set_temp_hf_index_3;
		unsigned char *arr_temp_hf_index_3 = new unsigned char[3];
		CellHandle *relocated_cell_result = new CellHandle[8]; 
		int uc_i;

		for (int i = 0; i < 8; ++i) {
			//对其中单位过滤
			confirm_now_cell = this->getCellHandle(i);
			
			//对其中的点观察对应关系对不对TODO:测试用的
			/*
			VertexHandle v;
			for (OpenVolumeMesh::HexVertexIter pair_h_v_it = mesh->hex_vertices(confirm_now_cell).first; pair_h_v_it.valid(); ++pair_h_v_it) {
				v = *(pair_h_v_it);
			}*/
			
			//判面上是否确定三个面包含有中心点 依次来对应
			//6个方向面 有几个包含中心点
			for (unsigned char ori = 0; ori < 6; ++ori) {
				hf_handle_temp = mesh->get_oriented_halfface(ori,confirm_now_cell);
				std::pair<OpenVolumeMesh::HalfFaceVertexIter, OpenVolumeMesh::HalfFaceVertexIter> pair_hf_v = mesh->halfface_vertices(hf_handle_temp);
					
				for (OpenVolumeMesh::HalfFaceVertexIter hf_v_iter = pair_hf_v.first;
					hf_v_iter != pair_hf_v.second; ++hf_v_iter) {
					//顺序加载
					if ((*hf_v_iter).idx() == middle_vertex.idx()) {
						set_temp_hf_index_3.insert(ori);
						//break;
						
					}
				}
			}
			uc_i = 0;
			for (std::set<unsigned char>::iterator iter_uc = set_temp_hf_index_3.begin();
				iter_uc != set_temp_hf_index_3.end();++iter_uc) {
				arr_temp_hf_index_3[uc_i++] = *iter_uc;
			}
			//对其中的排序与U_RELOCATED_CELL_INDEX进行对应匹配
			for (int cell_i = 0; cell_i < 8; ++cell_i) {
				//比对一致
				if (memcmp(U_RELOCATED_CELL_INDEX[cell_i], arr_temp_hf_index_3, 3 * sizeof(unsigned char))==0){
					index[i] = cell_i;
					break;
				}
			}

			set_temp_hf_index_3.swap(std::set<unsigned char>());
			
		}

		//开始重排
		for (uc_i = 0; uc_i < 8; ++uc_i) {
			//relocated_cell_result[uc_i] = this->bcs[index[uc_i]];
			relocated_cell_result[index[uc_i]] = this->bcs[uc_i];

		}
		for (uc_i = 0; uc_i < 8; ++uc_i) {
			this->bcs[uc_i] = relocated_cell_result[uc_i];
		}
		
		//将合并后的8顶点进行重排
		
		for (uc_i = 0; uc_i < 8; ++uc_i) {
			//TODO 一会测是不是可行
			temp_cell = this->getCellHandle(uc_i) ;
			
			//cell_vertex里是按idx大小顺序排列的 改掉默认顺序  U_RELOCATED_CELL_OUT_INDEX
			VertexHandle ve_arr[4];
			int cout_arr[4] = {0};

			uint16_t now_i = 0; 
			std::pair<OpenVolumeMesh::HalfFaceVertexIter, OpenVolumeMesh::HalfFaceVertexIter> pair_hfv1 = 
				mesh->halfface_vertices(mesh->get_oriented_halfface(U_RELOCATED_CELL_OUT_INDEX[uc_i][0],this->bcs[uc_i]));
			now_i = 0; 
			for (OpenVolumeMesh::HalfFaceVertexIter hfv_iter = pair_hfv1.first; hfv_iter != pair_hfv1.second; ++hfv_iter) {
				ve_arr[now_i++] = *hfv_iter;
			}
			
			std::pair<OpenVolumeMesh::HalfFaceVertexIter, OpenVolumeMesh::HalfFaceVertexIter> pair_hfv2 =
				mesh->halfface_vertices(mesh->get_oriented_halfface(U_RELOCATED_CELL_OUT_INDEX[uc_i][1], this->bcs[uc_i]));
			for (OpenVolumeMesh::HalfFaceVertexIter hfv_iter = pair_hfv2.first; hfv_iter != pair_hfv2.second; ++hfv_iter) {
				for (int i = 0; i < 4; ++i) {
					if (ve_arr[i].idx() == (*hfv_iter).idx()) {
						++cout_arr[i];
					}
				}
			}
			std::pair<OpenVolumeMesh::HalfFaceVertexIter, OpenVolumeMesh::HalfFaceVertexIter> pair_hfv3 =
				mesh->halfface_vertices(mesh->get_oriented_halfface(U_RELOCATED_CELL_OUT_INDEX[uc_i][2], this->bcs[uc_i]));

			for (OpenVolumeMesh::HalfFaceVertexIter hfv_iter = pair_hfv3.first; hfv_iter != pair_hfv3.second; ++hfv_iter) {
				for (int i = 0; i < 4; ++i) {
					if (ve_arr[i].idx() == (*hfv_iter).idx()) {
						++cout_arr[i];
					}
				}
			}
			int max =0 , max_i =0;
			for (int i = 0; i < 4; ++i) {
				if (cout_arr[i] > max) {
					max = cout_arr[i];
					max_i = i;
				}
			}
			

			this->setConnernVer((unsigned char)uc_i, ve_arr[max_i]);
			

		}
		//校验下
		if (!this->check_relocated()) {
			std::cout << "cao";
		}
		for (int i = 0; i < 8; ++i) {
			if (this->vs[i].idx() == 0|| this->vs[i].idx() == 1266) {
				std::cout << "guale";
			}
		}
		//释放空间
		delete[] index;
		//奇技淫巧 clear无卵用
		set_temp_hf_index_3.swap(std::set<unsigned char>());
		delete[] arr_temp_hf_index_3;
		delete[] relocated_cell_result;
	}
	
}

void BC_8::relocated_complex_real(HexMeshV3f * mesh, unsigned char* index) {
	//将合并后的8顶点进行重排

	for (uint16_t uc_i = 0; uc_i < 8; ++uc_i) {
		//TODO 一会测是不是可行
		CellHandle temp_cell = this->getCellHandle(index[uc_i]);
		//cell_vertex里是按idx大小顺序排列的 改掉默认顺序  U_RELOCATED_CELL_OUT_INDEX
		VertexHandle ve_arr[4];
		int cout_arr[4] = { 0 };

		uint16_t now_i = 0;
		std::pair<OpenVolumeMesh::HalfFaceVertexIter, OpenVolumeMesh::HalfFaceVertexIter> pair_hfv1 =
			mesh->halfface_vertices(mesh->get_oriented_halfface(U_RELOCATED_CELL_OUT_INDEX[uc_i][0], temp_cell));
		now_i = 0;
		for (OpenVolumeMesh::HalfFaceVertexIter hfv_iter = pair_hfv1.first; hfv_iter != pair_hfv1.second; ++hfv_iter) {
			ve_arr[now_i++] = *hfv_iter;
		}

		std::pair<OpenVolumeMesh::HalfFaceVertexIter, OpenVolumeMesh::HalfFaceVertexIter> pair_hfv2 =
			mesh->halfface_vertices(mesh->get_oriented_halfface(U_RELOCATED_CELL_OUT_INDEX[uc_i][1],temp_cell));
		for (OpenVolumeMesh::HalfFaceVertexIter hfv_iter = pair_hfv2.first; hfv_iter != pair_hfv2.second; ++hfv_iter) {
			for (int i = 0; i < 4; ++i) {
				if (ve_arr[i].idx() == (*hfv_iter).idx()) {
					++cout_arr[i];
				}
			}
		}
		std::pair<OpenVolumeMesh::HalfFaceVertexIter, OpenVolumeMesh::HalfFaceVertexIter> pair_hfv3 =
			mesh->halfface_vertices(mesh->get_oriented_halfface(U_RELOCATED_CELL_OUT_INDEX[uc_i][2], temp_cell));

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


		this->setConnernVer((unsigned char)uc_i, ve_arr[max_i]);

	}
	//校验下
	if (!this->check_relocated()) {
		std::cout << "cao";
	}
	for (int i = 0; i < 8; ++i) {
		if (this->vs[i].idx() == 0 || this->vs[i].idx() == 1266) {
			std::cout << "guale";
		}
	}
}

//对4的特化版本  对于单层cell夹层的情况 默认映射到了xf yf zf方向
void BC_8::bc_relocated_4(HexMeshV3f * mesh,BaseComplex& bc) {
	
	//if (this->bcs[0].idx() == 2455) {
		std::cout << "\n";
		for (int i = 0; i < 4; ++i) 
		{
			OpenVolumeMesh::OpenVolumeMeshCell cell = mesh->cell(this->bcs[i]);
			std::vector<HalfFaceHandle> cell_arr = cell.halffaces(); 
			for (int j = 0; j < 6; ++j) {
				std::cout<<(int)(mesh->face_handle(cell_arr[j]).idx())<<" "; 
			}
			std::cout << "\n";
		}
	//}
	//依然是对应到0-7 其中四位
	
	//找到对齐朝向

	//简单直接找个set往里扔点 看集合大小 只有两种情况 12 9
	unsigned char max_ori_index = INVALID_ORIENTATION;
	std::set<VertexHandle, compare_OVM> remove_set;
	
	for (unsigned now_ori = 0; now_ori < INVALID_ORIENTATION; ++now_ori) {
		for (int i = 0; i < 4; ++i)
		{
			
			std::vector<HalfEdgeHandle> he_v = mesh->halfface(mesh->get_oriented_halfface(now_ori, this->bcs[i])).halfedges();
			for (std::vector<HalfEdgeHandle>::iterator he_itr = he_v.begin(); he_itr != he_v.end(); ++he_itr) {
				remove_set.insert(mesh->halfedge(*he_itr).from_vertex());
				//remove_set.insert(mesh->halfedge(*he_itr).to_vertex());

			}

		}
		if (remove_set.size() == 9) {
			max_ori_index = now_ori;
			remove_set.swap(std::set<VertexHandle, compare_OVM>());
			break;
		}
		remove_set.swap(std::set<VertexHandle, compare_OVM>());
	}
	this->orientation = max_ori_index; 
	
	CellHandle temp_cell_arr_b[2] = { NULL,NULL };
	CellHandle temp_cell_arr_f[2] = { NULL,NULL };
	FaceHandle temp_f_recod;
	uint16_t temp_cout_repetition = 0; 


	//分六方向对单元重排
	switch (max_ori_index) {
	case 0: {
		uint16_t fill_index = 0; 
		//按yf yb二分
		for (int i = 0; i < 4; ++i) {
			temp_cout_repetition = 0;
			//YF
			temp_f_recod = mesh->face_handle(mesh->get_oriented_halfface(con_orientations[2], this->bcs[i]));
			//与其他三个单元的yb求面重合次数
			for (int j = (i + 1) % 4; j !=i ; j = (j+1)%4) {
				if (mesh->face_handle(mesh->get_oriented_halfface(con_orientations[3], this->bcs[j]))==temp_f_recod) {
					++temp_cout_repetition;
				}

			}
			//出现重复次数为二的情况 则判定为yb部分
			if (temp_cout_repetition == 1) {
				temp_cell_arr_b[fill_index++] = this->bcs[i];
			}
		}
		fill_index = 0; 
		for (int i = 0; i < 4; ++i) {
			if (this->bcs[i] != temp_cell_arr_b[0] && this->bcs[i] != temp_cell_arr_b[1]) {
				temp_cell_arr_f[fill_index++] = this->bcs[i];
			}

		}

		//销毁原cell排序队列
		this->clearElem();

			//yf 按zf zb二分
		if (mesh->face_handle(mesh->get_oriented_halfface(con_orientations[4], temp_cell_arr_f[0]))
			== mesh->face_handle(mesh->get_oriented_halfface(con_orientations[5], temp_cell_arr_f[1]))) {
			this->bcs[2] = temp_cell_arr_f[0];
			this->bcs[1] = temp_cell_arr_f[1]; 
		}
		else {
			this->bcs[2] = temp_cell_arr_f[1];
			this->bcs[1] = temp_cell_arr_f[0];
		}
			//yb 按zf zb二分
		if (mesh->face_handle(mesh->get_oriented_halfface(con_orientations[4], temp_cell_arr_b[0]))
			== mesh->face_handle(mesh->get_oriented_halfface(con_orientations[5], temp_cell_arr_b[1]))) {
			this->bcs[3] = temp_cell_arr_b[0];
			this->bcs[0] = temp_cell_arr_b[1];
		}
		else {
			this->bcs[3] = temp_cell_arr_b[1];
			this->bcs[0] = temp_cell_arr_b[0];
		}
		
		//0-cell index
		unsigned char index[8] = {
			0,1,2,3,
			0,3,2,1
		};

		//对八个顶点重排TODO:待观察是否正确
		relocated_complex_real(mesh, index);
		/*for (int i = 0; i < 8; ++i) {
			this->setConnernVer(i, *(mesh->cell_vertices(this->bcs[index[i]]).first + i ));
		}*/
		//校验下
		if (!this->check_relocated()) {
			std::cout << "cao";
		}
		this->init = true;
		pos = 4;
	}break;
	case 1: {
		uint16_t fill_index = 0;
		//按yf yb二分
		for (int i = 0; i < 4; ++i) {
			temp_cout_repetition = 0;
			//YF
			temp_f_recod = mesh->face_handle(mesh->get_oriented_halfface(con_orientations[2], this->bcs[i]));
			//与其他三个单元的yb求面重合次数
			for (int j = (i + 1) % 4; j != i; j = (j + 1) % 4) {
				if (mesh->face_handle(mesh->get_oriented_halfface(con_orientations[3], this->bcs[j])) == temp_f_recod) {
					++temp_cout_repetition;
				}

			}
			//出现重复次数为二的情况 则判定为yb部分
			if (temp_cout_repetition == 1) {
				temp_cell_arr_b[fill_index++] = this->bcs[i];
			}
		}
		fill_index = 0;
		for (int i = 0; i < 4; ++i) {
			if (this->bcs[i] != temp_cell_arr_b[0] && this->bcs[i] != temp_cell_arr_b[1]) {
				temp_cell_arr_f[fill_index++] = this->bcs[i];
			}

		}

		//销毁原cell排序队列
		this->clearElem();


		//yf 按zf zb二分
		if (mesh->face_handle(mesh->get_oriented_halfface(con_orientations[4], temp_cell_arr_f[0]))
			== mesh->face_handle(mesh->get_oriented_halfface(con_orientations[5], temp_cell_arr_f[1]))) {
			this->bcs[6] = temp_cell_arr_f[0];
			this->bcs[7] = temp_cell_arr_f[1];
		}
		else {
			this->bcs[6] = temp_cell_arr_f[1];
			this->bcs[7] = temp_cell_arr_f[0];
		}
		//yb 按zf zb二分
		if (mesh->face_handle(mesh->get_oriented_halfface(con_orientations[4], temp_cell_arr_b[0]))
			== mesh->face_handle(mesh->get_oriented_halfface(con_orientations[5], temp_cell_arr_b[1]))) {
			this->bcs[5] = temp_cell_arr_b[0];
			this->bcs[4] = temp_cell_arr_b[1];
		}
		else {
			this->bcs[5] = temp_cell_arr_b[1];
			this->bcs[4] = temp_cell_arr_b[0];
		}


		//0-cell index
		unsigned char index[8] = {
			4,7,6,5,
			4,5,6,7
		};

		//对八个顶点重排TODO:待观察是否正确
		relocated_complex_real(mesh, index);
		/*for (int i = 0; i < 8; ++i) {
			this->setConnernVer(i, *(mesh->cell_vertices(this->bcs[index[i]]).first + i));
		}*/
		//校验下
		if (!this->check_relocated()) {
			std::cout << "cao";
		}
		this->init = true;
		pos = 4;
	}break;
	case 2: {
		uint16_t fill_index = 0;
		//按xf xb二分
		for (int i = 0; i < 4; ++i) {
			temp_cout_repetition = 0;
			//XF
			temp_f_recod = mesh->face_handle(mesh->get_oriented_halfface(con_orientations[0], this->bcs[i]));
			//与其他三个单元的xb求面重合次数
			for (int j = (i + 1) % 4; j != i; j = (j + 1) % 4) {
				if (mesh->face_handle(mesh->get_oriented_halfface(con_orientations[1], this->bcs[j])) == temp_f_recod) {
					++temp_cout_repetition;
				}

			}
			//出现重复次数为二的情况 则判定为xb部分
			if (temp_cout_repetition == 1) {
				temp_cell_arr_b[fill_index++] = this->bcs[i];
			}
		}
		fill_index = 0;
		for (int i = 0; i < 4; ++i) {
			if (this->bcs[i] != temp_cell_arr_b[0] && this->bcs[i] != temp_cell_arr_b[1]) {
				temp_cell_arr_f[fill_index++] = this->bcs[i];
			}

		}

		//销毁原cell排序队列
		this->clearElem();


		//xf 按zf zb二分
		if (mesh->face_handle(mesh->get_oriented_halfface(con_orientations[4], temp_cell_arr_f[0]))
			== mesh->face_handle(mesh->get_oriented_halfface(con_orientations[5], temp_cell_arr_f[1]))) {
			this->bcs[2] = temp_cell_arr_f[0];
			this->bcs[1] = temp_cell_arr_f[1];
		}
		else {
			this->bcs[2] = temp_cell_arr_f[1];
			this->bcs[1] = temp_cell_arr_f[0];
		}
		//xb 按zf zb二分
		if (mesh->face_handle(mesh->get_oriented_halfface(con_orientations[4], temp_cell_arr_b[0]))
			== mesh->face_handle(mesh->get_oriented_halfface(con_orientations[5], temp_cell_arr_b[1]))) {
			this->bcs[6] = temp_cell_arr_b[0];
			this->bcs[7] = temp_cell_arr_b[1];
		}
		else {
			this->bcs[6] = temp_cell_arr_b[1];
			this->bcs[7] = temp_cell_arr_b[0];
		}

		//0-cell index
		unsigned char index[8] = {
			1,1,2,2,
			7,6,6,7
		};

		//对八个顶点重排TODO:待观察是否正确
		relocated_complex_real(mesh, index);
		/*for (int i = 0; i < 8; ++i) {
			this->setConnernVer(i, *(mesh->cell_vertices(this->bcs[index[i]]).first + i));
		}*/
		//校验下
		if (!this->check_relocated()) {
			std::cout << "cao";
		}
		this->init = true;
		pos = 4;
	}break;
	case 3: {
		uint16_t fill_index = 0;
		//按xf xb二分
		for (int i = 0; i < 4; ++i) {
			temp_cout_repetition = 0;
			//XF
			temp_f_recod = mesh->face_handle(mesh->get_oriented_halfface(con_orientations[0], this->bcs[i]));
			//与其他三个单元的xb求面重合次数
			for (int j = (i + 1) % 4; j != i; j = (j + 1) % 4) {
				if (mesh->face_handle(mesh->get_oriented_halfface(con_orientations[1], this->bcs[j])) == temp_f_recod) {
					++temp_cout_repetition;
				}

			}
			//出现重复次数为二的情况 则判定为xb部分
			if (temp_cout_repetition == 1) {
				temp_cell_arr_b[fill_index++] = this->bcs[i];
			}
		}
		fill_index = 0;
		for (int i = 0; i < 4; ++i) {
			if (this->bcs[i] != temp_cell_arr_b[0] && this->bcs[i] != temp_cell_arr_b[1]) {
				temp_cell_arr_f[fill_index++] = this->bcs[i];
			}

		}

		//销毁原cell排序队列
		this->clearElem();


		//xf 按zf zb二分
		if (mesh->face_handle(mesh->get_oriented_halfface(con_orientations[4], temp_cell_arr_f[0]))
			== mesh->face_handle(mesh->get_oriented_halfface(con_orientations[5], temp_cell_arr_f[1]))) {
			this->bcs[3] = temp_cell_arr_f[0];
			this->bcs[0] = temp_cell_arr_f[1];
		}
		else {
			this->bcs[3] = temp_cell_arr_f[1];
			this->bcs[0] = temp_cell_arr_f[0];
		}
		//xb 按zf zb二分
		if (mesh->face_handle(mesh->get_oriented_halfface(con_orientations[4], temp_cell_arr_b[0]))
			== mesh->face_handle(mesh->get_oriented_halfface(con_orientations[5], temp_cell_arr_b[1]))) {
			this->bcs[5] = temp_cell_arr_b[0];
			this->bcs[4] = temp_cell_arr_b[1];
		}
		else {
			this->bcs[5] = temp_cell_arr_b[1];
			this->bcs[4] = temp_cell_arr_b[0];
		}

		//0-cell index
		unsigned char index[8] = {
			0,0,3,3,
			4,5,5,4
		};

		//对八个顶点重排TODO:待观察是否正确
		relocated_complex_real(mesh, index);
		/*for (int i = 0; i < 8; ++i) {
			this->setConnernVer(i, *(mesh->cell_vertices(this->bcs[index[i]]).first + i));
		}*/
		//校验下
		if (!this->check_relocated()) {
			std::cout << "cao";
		}
		this->init = true;
		pos = 4;
	}break;
	case 4: {

		uint16_t fill_index = 0;
		//按xf xb二分
		for (int i = 0; i < 4; ++i) {
			temp_cout_repetition = 0;
			//XF
			temp_f_recod = mesh->face_handle(mesh->get_oriented_halfface(con_orientations[0], this->bcs[i]));
			//与其他三个单元的xb求面重合次数
			
			for (int j = (i + 1)%4; j != i; j = (j + 1) % 4) {
				
				if (mesh->face_handle(mesh->get_oriented_halfface(con_orientations[1], this->bcs[j])) == temp_f_recod) {
					++temp_cout_repetition;
				}

			}
			//出现重复次数不为0的情况 则判定为xb部分
			if (temp_cout_repetition == 1) {
				temp_cell_arr_b[fill_index++] = this->bcs[i];
			}
		}
		fill_index = 0;
		for (int i = 0; i < 4; ++i) {
			if (this->bcs[i] != temp_cell_arr_b[0] && this->bcs[i] != temp_cell_arr_b[1]) {
				temp_cell_arr_f[fill_index++] = this->bcs[i];
			}

		}

		//销毁原cell排序队列
		this->clearElem();


		//xf 按yf yb二分
		if (mesh->face_handle(mesh->get_oriented_halfface(con_orientations[2], temp_cell_arr_f[0]))
			== mesh->face_handle(mesh->get_oriented_halfface(con_orientations[3], temp_cell_arr_f[1]))) {
			this->bcs[0] = temp_cell_arr_f[0];
			this->bcs[1] = temp_cell_arr_f[1];
		}
		else {
			this->bcs[0] = temp_cell_arr_f[1];
			this->bcs[1] = temp_cell_arr_f[0];
		}
		//xb 按yf yb二分
		if (mesh->face_handle(mesh->get_oriented_halfface(con_orientations[2], temp_cell_arr_b[0]))
			== mesh->face_handle(mesh->get_oriented_halfface(con_orientations[3], temp_cell_arr_b[1]))) {
			this->bcs[4] = temp_cell_arr_b[0];
			this->bcs[7] = temp_cell_arr_b[1];
		}
		else {
			this->bcs[4] = temp_cell_arr_b[1];
			this->bcs[7] = temp_cell_arr_b[0];
		}

		//0-cell index
		unsigned char index[8] = {
			0,1,1,0,
			4,4,7,7
		};

		//对八个顶点重排TODO:待观察是否正确
		relocated_complex_real(mesh, index);
		/*for (int i = 0; i < 8; ++i) {
			this->setConnernVer(i, *(mesh->cell_vertices(this->bcs[index[i]]).first + i));
		}*/
		//校验下
		if (!this->check_relocated()) {
			std::cout << "cao";
		}
		pos = 4;
		this->init = true;
	}break;
	case 5: {


		uint16_t fill_index = 0;
		//按xf xb二分
		for (int i = 0; i < 4; ++i) {
			temp_cout_repetition = 0;
			//XF
			temp_f_recod = mesh->face_handle(mesh->get_oriented_halfface(con_orientations[0], this->bcs[i]));
			//与其他三个单元的xb求面重合次数
			for (int j = (i + 1) % 4; j != i; j = (j + 1) % 4) {
				if (mesh->face_handle(mesh->get_oriented_halfface(con_orientations[1], this->bcs[j])) == temp_f_recod) {
					++temp_cout_repetition;
				}

			}
			//出现重复次数为二的情况 则判定为xb部分
			if (temp_cout_repetition == 1) {
				temp_cell_arr_b[fill_index++] = this->bcs[i];
			}
		}
		fill_index = 0;
		for (int i = 0; i < 4; ++i) {
			if (this->bcs[i] != temp_cell_arr_b[0] && this->bcs[i] != temp_cell_arr_b[1]) {
				temp_cell_arr_f[fill_index++] = this->bcs[i];
			}

		}

		//销毁原cell排序队列
		this->clearElem();


		//xf 按yf yb二分
		if (mesh->face_handle(mesh->get_oriented_halfface(con_orientations[2], temp_cell_arr_f[0]))
			== mesh->face_handle(mesh->get_oriented_halfface(con_orientations[3], temp_cell_arr_f[1]))) {
			this->bcs[3] = temp_cell_arr_f[0];
			this->bcs[2] = temp_cell_arr_f[1];
		}
		else {
			this->bcs[3] = temp_cell_arr_f[1];
			this->bcs[2] = temp_cell_arr_f[0];
		}
		//xb 按yf yb二分
		if (mesh->face_handle(mesh->get_oriented_halfface(con_orientations[2], temp_cell_arr_b[0]))
			== mesh->face_handle(mesh->get_oriented_halfface(con_orientations[3], temp_cell_arr_b[1]))) {
			this->bcs[5] = temp_cell_arr_b[0];
			this->bcs[6] = temp_cell_arr_b[1];
		}
		else {
			this->bcs[5] = temp_cell_arr_b[1];
			this->bcs[6] = temp_cell_arr_b[0];
		}
		
		//0-cell index
		unsigned char index[8] = {
			3,2,2,3,
			5,5,6,6
		};

		//对八个顶点重排TODO:待观察是否正确
		relocated_complex_real(mesh, index);
		/*for (int i = 0; i < 8; ++i) {
			this->setConnernVer(i, *(mesh->cell_vertices(this->bcs[index[i]]).first + i));
		}*/
		//校验下
		if (!this->check_relocated()) {
			std::cout << "cao";
		}
		this->init = true;
		pos = 4;
	}break;
	}

}