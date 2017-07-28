#ifndef _HEXOVM_FILECONVERTHELPER_H_
#define _HEXOVM_FILECONVERTHELPER_H_
#include "ComHead.h"

 using namespace std;

//读入hexmesh和写出转换的ovm文件
class FileConverterHelper {
	
private:
	typedef typename HexMeshV3f::PointT Point;
	HexMeshV3f *pm_convert_from;
	ifstream mesh_iff; 
	ofstream mesh_off;
	bool checkIsFilled; 
	bool isReading; 
	string filepath_checked; 
private:
	bool getCleanLine(istream& _ifs, string& _string, bool _skipEmptyLines = true) const;
	void trimString(string& _string) const;
	void extractQuotedText(string& _string) const;
public:
	FileConverterHelper(string str_file_path, string str_file_name);
	
	bool Convert2HexMeshObj(HexMeshV3f *hexmesh , bool isWrite2File, bool _computBottom, bool _topologyCheck);
	
};

#endif // !_HEXOVM_FILECONVERTHELPER_H_

