#include "FileHandler.h"

HexMeshV3f hexMeshV3f ;

HexMeshV3f* readMeshFile(std::string str_file_path, std::string str_file_name) {
	if (str_file_path.empty()) {
		std::cout << "�����ļ�·��Ϊ��!";
		
		return NULL;
	}
	else if (str_file_name.empty()) {
		std::cout << "�����ļ���Ϊ��!";
		
		return NULL;
	}

	if (str_file_name.find("ovm") == std::string::npos&&
		str_file_name.find("OVM") == std::string::npos) {
		std::cout << "�����ļ�����ΪOVM��ʽ!";

		return NULL;
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

	OpenVolumeMesh::IO::FileManager fileManager;

	if (!fileManager.readFile(result, hexMeshV3f, true, true)) {
		return &hexMeshV3f;
	}
	return &hexMeshV3f;
}

bool writeMesh2File(std::string str_file_path, std::string str_file_name,HexMeshV3f *hexMesh) {
	OpenVolumeMesh::IO::FileManager fileManager;
	if (str_file_path.empty()) {
		std::cout << "����ļ�·��Ϊ��!";
		return false;
	}
	else if (str_file_name.empty()) {
		std::cout << "����ļ���Ϊ��!";
		return false;
	}

	if (str_file_name.find("ovm") == std::string::npos&&
		str_file_name.find("OVM") == std::string::npos) {
		std::cout << "����ļ�����ΪOVM��ʽ!";
		return false;
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

	return fileManager.writeFile(result, *hexMesh);

}
