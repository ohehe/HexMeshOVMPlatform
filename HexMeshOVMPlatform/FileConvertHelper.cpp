#include "FileConvertHelper.h"
//
//ConverterForMesh2HEXOVM::ConverterForMesh2HEXOVM()
//{
//	isReading = false;
//	checkIsFilled = false;
//}

struct compare_OVM_N
{
	bool operator()(const OpenVolumeMesh::OpenVolumeMeshHandle &a, const OpenVolumeMesh::OpenVolumeMeshHandle &b) const
	{
		return a.idx()<b.idx();
	}
};
FileConverterHelper::FileConverterHelper(string str_file_path, string str_file_name)
{
	if (str_file_path.empty()) {
		std::cout << "�����ļ�·��Ϊ��!";
		isReading = false;
		checkIsFilled = false; 
		return ;
	}
	else if (str_file_name.empty()) {
		std::cout << "�����ļ���Ϊ��!";
		isReading = false;
		checkIsFilled = false;
		return ;
	}

	if (str_file_name.find("mesh") == std::string::npos&&
		str_file_name.find("MESH") == std::string::npos) {
		std::cout << "����ת���ļ�����ΪMesh��ʽ!";
		isReading = false;
		checkIsFilled = false;
		return ;
	}

	//ƴ���ַ���
	std::stringstream stream;
	std::string result;


	stream << str_file_path;
	if (str_file_path.rfind("\\") != str_file_path.length() - 1 &&
		str_file_path.rfind("/") != str_file_path.length() - 1) {

		stream << "\\";

	}
	stream << str_file_name;
	stream >> result;

	filepath_checked = result; 
	isReading = true; 
	checkIsFilled = false;
}

void FileConverterHelper::extractQuotedText(string& _string) const {

	// Trim Both leading and trailing quote marks
	size_t start = _string.find_first_of("\""); ++start;
	size_t end = _string.find_last_not_of("\"");

	if ((string::npos == start) || (string::npos == end)) {
		_string = "";
	}
	else {
		_string = _string.substr(start, end - start + 1);
	}
}
void FileConverterHelper::trimString(string& _string) const {

	// Trim Both leading and trailing spaces
	size_t start = _string.find_first_not_of(" \t\r\n");
	size_t end = _string.find_last_not_of(" \t\r\n");

	if ((string::npos == start) || (string::npos == end)) {
		_string = "";
	}
	else {
		_string = _string.substr(start, end - start + 1);
	}
}

bool FileConverterHelper::getCleanLine(istream& _ifs,string& _string, bool _skipEmptyLines) const {

	// While we are not at the end of the file
	while (true) {

		// Get the current line:
		getline(_ifs, _string);

		// Remove whitespace at beginning and end
		trimString(_string);

		// Check if string is not empty ( otherwise we continue
		if (_string.size() != 0) {

			// Check if string is a comment ( starting with # )
			if (_string[0] != '#') {
				return true;
			}

		}
		else {
			if (!_skipEmptyLines)
				return true;
		}

		if (_ifs.eof()||_string.compare("End")) {
			cerr << "End of file reached while searching for input!" << endl;
			return false;
		}
	}

	return false;
}
//TODO:�ظ� �������Ĳ����¼ �����Ŵ򿪶�
bool FileConverterHelper::Convert2HexMeshObj(HexMeshV3f *hexmesh, bool isWrite2File, bool _computBottom,bool _topologyCheck)
{
	if (!isReading) {
		return false; 
	}
	this->pm_convert_from = hexmesh; 

	mesh_iff = ifstream(filepath_checked.c_str() , ios::in);

	if (!mesh_iff.good()) {
		std::cerr << "Error: Could not open file " << filepath_checked << " for reading!" << std::endl;
		mesh_iff.close();
		return false;
	}

	stringstream sstr; 
	string line; 
	string s_tmp; 
	uint64_t c = 0u; 
	
	Point v = Point(0.00, 0.00, 0.00); 
	int v_arr[8];
	
	pm_convert_from = hexmesh;
	pm_convert_from->clear(true); 
	
	//��ʱֹͣbottom_up����
	pm_convert_from->enable_bottom_up_incidences(true); 

	//*************************
	//header
	bool header_found = true; 

	//���ļ�ͷ
	getCleanLine(mesh_iff, line); 
	sstr.str(line); 
	//���ͷ

	// Check header
	sstr >> s_tmp;
	transform(s_tmp.begin(), s_tmp.end(), s_tmp.begin(), ::toupper);
	if (s_tmp != "MESHVERSIONFORMATTED") {
		//iff.close();
		header_found = false;
		mesh_iff.close(); 
		std::cerr << "The specified file might not be in .mesh format!" << std::endl;
		return false;
	}

	sstr.clear();
	getCleanLine(mesh_iff, line); 
	sstr.str(line); 

	// Get Dimension
	string s_dim; 
	sstr >> s_tmp;
	/*sstr.clear();
	getCleanLine(mesh_iff, line);*/
	//sstr.str(line);
	sstr >> s_dim;
	transform(s_tmp.begin(), s_tmp.end(), s_tmp.begin(), ::toupper);
	if (s_tmp != "DIMENSION"|| s_dim != "3") {
		mesh_iff.close();
		std::cerr << "Other dimension for files are not supported at the moment!" << std::endl;
		return false;
	}
	
	//�㼯
	if (!header_found) {
		sstr.clear();
		sstr.str(line); 
	}
	else {
		sstr.clear();
		getCleanLine(mesh_iff, line); 
		sstr.str(line); 
	}

	sstr >> s_tmp; 
	std::transform(s_tmp.begin(), s_tmp.end(), s_tmp.begin(), ::toupper);
	if (s_tmp != "VERTICES") {
		mesh_iff.close();
		std::cerr << "No vertex section defined!" << std::endl;
		return false;
	}
	else {
		//��������
		getCleanLine(mesh_iff, line);
		sstr.clear();
		sstr.str(line);
		sstr >> c;
		//������
		for (uint64_t i = 0u; i < c; ++i) {

			getCleanLine(mesh_iff, line);
			sstr.clear();
			sstr.str(line);
			sstr >> v[0];
			sstr >> v[1];
			sstr >> v[2];
			pm_convert_from->add_vertex(v);
		}

	}

	//�߼����漯 
	//���յĴ�ɽ��ģ��  �滹�ǲ�ȫ�� ֻ�б���
	//getCleanLine(mesh_iff, line);
	//sstr.clear();
	//sstr.str(line);
	//sstr >> s_tmp;
	//std::transform(s_tmp.begin(), s_tmp.end(), s_tmp.begin(), ::toupper);
	//if (s_tmp != "QUADRILATERALS") {
	//	mesh_iff.close();
	//	std::cerr << "No QUADRILATERALS section defined!" << std::endl;
	//	return false;
	//}
	//else {
	//	//����Ƭ��
	//	getCleanLine(mesh_iff, line);
	//	sstr.clear();
	//	sstr.str(line);
	//	sstr >> c;

	//	for (uint64_t i = 0u; i < c; ++i) {

	//		getCleanLine(mesh_iff, line);
	//		sstr.clear();
	//		sstr.str(line);

	//		vector<VertexHandle> vec_temp_v;
	//		for (unsigned int e = 0; e < 4; ++e) {
	//			sstr >> v_arr[e];
	//			VertexHandle v_tmp(v_arr[e]); 
	//			vec_temp_v.push_back(v_tmp);
	//		}
	//		//for (unsigned int e = 0; e < 4; ++e) {
	//		//	//���ϴ��ڱ�У��
	//		//	pm_convert_from->add_edge(VertexHandle(v_arr[e]), VertexHandle(v_arr[(e + 1) % 4]),false); 
	//		//}
	//		
	//		//���ϱ���� (��ֱ�Ӱ����������е��¡�����)
	//		pm_convert_from->add_face(vec_temp_v);
	//	}

	//	

	//}

	//���������
	//while (true)
	//{
	//	getCleanLine(mesh_iff, line);
	//	if (line == "Hexahedra")break; 
	//}
	//getCleanLine(mesh_iff, line);
	//sstr.clear();
	//sstr.str(line);
	//sstr >> c;
	//int temp_inde = 0;
	//for (unsigned int f = 0; f < c; ++f) {
	//	cout << f<<":"<<c << endl;
	//	vector<VertexHandle> temp_vetex;
	//	getCleanLine(mesh_iff, line);
	//	sstr.clear();
	//	sstr.str(line);
	//	
	//	for (int i = 0; i < 8; ++i) {
	//		sstr >> temp_inde;
	//		temp_vetex.push_back(VertexHandle(temp_inde-1));
	//	}

	//	vector<HalfFaceHandle> temp_faces;

	//	//����������
	//	//zb
	//	vector<VertexHandle> temp1;
	//	temp1.push_back(temp_vetex[0]);
	//	temp1.push_back(temp_vetex[1]);
	//	temp1.push_back(temp_vetex[2]);
	//	temp1.push_back(temp_vetex[3]);
	//	temp_faces.push_back(pm_convert_from->halfface_handle(pm_convert_from->add_face(temp1), 0));
	//	temp1.clear();
	//	//zf
	//	//vector<VertexHandle> temp2;
	//	temp1.push_back(temp_vetex[4]);
	//	temp1.push_back(temp_vetex[7]);
	//	temp1.push_back(temp_vetex[6]);
	//	temp1.push_back(temp_vetex[5]);
	//	temp_faces.push_back(pm_convert_from->halfface_handle(pm_convert_from->add_face(temp1), 0));
	//	temp1.clear();
	//	//xf
	//	//vector<VertexHandle> temp3;
	//	temp1.push_back(temp_vetex[1]);
	//	temp1.push_back(temp_vetex[5]);
	//	temp1.push_back(temp_vetex[6]);
	//	temp1.push_back(temp_vetex[2]);
	//	temp_faces.push_back(pm_convert_from->halfface_handle(pm_convert_from->add_face(temp1),0));
	//	temp1.clear();
	//	//xb
	//	//vector<VertexHandle> temp4;
	//	temp1.push_back(temp_vetex[0]);
	//	temp1.push_back(temp_vetex[3]);
	//	temp1.push_back(temp_vetex[7]);
	//	temp1.push_back(temp_vetex[4]);
	//	temp_faces.push_back(pm_convert_from->halfface_handle(pm_convert_from->add_face(temp1), 0));
	//	temp1.clear();

	//	//yf
	//	//vector<VertexHandle> temp5;
	//	temp1.push_back(temp_vetex[2]);
	//	temp1.push_back(temp_vetex[6]);
	//	temp1.push_back(temp_vetex[7]);
	//	temp1.push_back(temp_vetex[3]);
	//	temp_faces.push_back(pm_convert_from->halfface_handle(pm_convert_from->add_face(temp1), 0));
	//	temp1.clear();

	//	//yb
	//	//vector<VertexHandle> temp6;
	//	temp1.push_back(temp_vetex[0]);
	//	temp1.push_back(temp_vetex[4]);
	//	temp1.push_back(temp_vetex[5]);
	//	temp1.push_back(temp_vetex[1]);
	//	temp_faces.push_back(pm_convert_from->halfface_handle(pm_convert_from->add_face(temp1), 0));
	//	temp1.clear();
	//	//pm_convert_from->add_cell(temp_vetex);
	//	
	//	pm_convert_from->add_cell(temp_faces, true);
	//}

	while (true)
	{
		getCleanLine(mesh_iff, line);
		if (line == "Hexahedra")break; 
	}
	getCleanLine(mesh_iff, line);
	sstr.clear();
	sstr.str(line);
	sstr >> c;
	int temp_inde = 0;
	for (unsigned int f = 0; f < c; ++f) {
		cout << f << ":" << c << endl;
		vector<VertexHandle> temp_vetex;
		vector<VertexHandle> temp_vetex_real;
		getCleanLine(mesh_iff, line);
		sstr.clear();
		sstr.str(line);

		for (int i = 0; i < 8; ++i) {
			sstr >> temp_inde;
			temp_vetex.push_back(VertexHandle(temp_inde - 1));
		}
		
		temp_vetex_real.push_back(temp_vetex[0]);
		temp_vetex_real.push_back(temp_vetex[1]);
		temp_vetex_real.push_back(temp_vetex[2]);
		temp_vetex_real.push_back(temp_vetex[3]);
		temp_vetex_real.push_back(temp_vetex[4]);
		temp_vetex_real.push_back(temp_vetex[7]);
		temp_vetex_real.push_back(temp_vetex[6]);
		temp_vetex_real.push_back(temp_vetex[5]);
		
		
		


		CellHandle cell_now = pm_convert_from->add_cell(temp_vetex_real, true);
		
		for (unsigned char ori = 0; ori < 6; ++ori) {
			bool is_ckecked_true = true;
			HalfFaceHandle hf_handle; 
			hf_handle = pm_convert_from->get_oriented_halfface(ori, cell_now);
			
			OpenVolumeMesh::OpenVolumeMeshFace face = pm_convert_from->halfface(hf_handle);
			std::vector<HalfEdgeHandle>::const_iterator c_ed_vector_i = face.halfedges().begin();
			std::set<VertexHandle, compare_OVM_N> ver_set;
			//****************
			while (c_ed_vector_i != face.halfedges().end()) {
				ver_set.insert(pm_convert_from->halfedge(*c_ed_vector_i).from_vertex());
				++c_ed_vector_i; 
			}
			if (ver_set.size() != 4) {
				std::cout << "wocao";
			}
			switch (ori) {
			case 0: {
				if (ver_set.find(temp_vetex_real[0]) == ver_set.end() ||
					ver_set.find(temp_vetex_real[1]) == ver_set.end() ||
					ver_set.find(temp_vetex_real[2]) == ver_set.end() ||
					ver_set.find(temp_vetex_real[3]) == ver_set.end()) {
					std::cout << "wocao";
				}
			}break;
			case 1: {
				if (ver_set.find(temp_vetex_real[4]) == ver_set.end() ||
					ver_set.find(temp_vetex_real[5]) == ver_set.end() ||
					ver_set.find(temp_vetex_real[6]) == ver_set.end() ||
					ver_set.find(temp_vetex_real[7]) == ver_set.end()) {
					std::cout << "wocao";
				}
			}break;
			case 2: {
				if (ver_set.find(temp_vetex_real[1]) == ver_set.end() ||
					ver_set.find(temp_vetex_real[2]) == ver_set.end() ||
					ver_set.find(temp_vetex_real[6]) == ver_set.end() ||
					ver_set.find(temp_vetex_real[7]) == ver_set.end()) {
					std::cout << "wocao";
				}
			}break;
			case 3: {
				if (ver_set.find(temp_vetex_real[0]) == ver_set.end() ||
					ver_set.find(temp_vetex_real[3]) == ver_set.end() ||
					ver_set.find(temp_vetex_real[4]) == ver_set.end() ||
					ver_set.find(temp_vetex_real[5]) == ver_set.end()) {
					std::cout << "wocao";
				}
			}break;
			case 5: {
				if (ver_set.find(temp_vetex_real[2]) == ver_set.end() ||
					ver_set.find(temp_vetex_real[3]) == ver_set.end() ||
					ver_set.find(temp_vetex_real[5]) == ver_set.end() ||
					ver_set.find(temp_vetex_real[6]) == ver_set.end()) {
					std::cout << "wocao";
				}
			}break;
			case 4: {
				if (ver_set.find(temp_vetex_real[0]) == ver_set.end() ||
					ver_set.find(temp_vetex_real[1]) == ver_set.end() ||
					ver_set.find(temp_vetex_real[4]) == ver_set.end() ||
					ver_set.find(temp_vetex_real[7]) == ver_set.end()) {
					std::cout << "wocao";
				}
			}break;
			}
		}
	}
	mesh_iff.close();
	
	
	

	return true;
}
