#ifndef _HEXMESH_FILEHANDLER
#define _HEXMESH_FILEHANDLER

#include "ComHead.h"

//���ļ���ȡ�ȵȵĴ�����
HexMeshV3f* readMeshFile(std::string str_file_path, std::string str_file_name);

bool writeMesh2File(std::string str_file_path, std::string str_file_name,HexMeshV3f *hexmesh); 

#endif // !_HEXMESH_FILEHANDLER
