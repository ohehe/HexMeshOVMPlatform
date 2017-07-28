#include"FileHandler.h"
#include "SingularObject.h"
#include "OVMViewer.h"
#include "FaceExpandAndHexFlooding.h"
#include "FileConvertHelper.h"
#include "BaseComplexSimplify.h"

int main(int argc, char** argv) {
	if (argc < 4) {
		std::cout << "Usage:[show wire frame:bool] [need convert:bool] [path] [filename]\n "; 
		system("pause");
		return 0; 
	}
	std::string path = std::string(argv[3]); 
	OpenVolumeMesh::IO::FileManager fileManager;
	std::string outputfilename; 
	std::string filename_converted = "";

	filename_converted = string(argv[4]);
	filename_converted.replace(filename_converted.find("\."), filename_converted.find("\.") + 4, ".ovm");
	//奇异边集合
	std::vector<SingularEdge> se_vector;
	if (string(argv[2]).compare("true") == 0|| string(argv[2]).compare("TRUE") == 0) { //先转换 再直接显示
		
		HexMeshV3f* hex;
		HexMeshV3f hex_init;
		hex = &hex_init;
		FileConverterHelper co(argv[3],argv[4]);
		co.Convert2HexMeshObj(hex, true, true, true);
		//fileManager.readFile(path + filename_converted, *hex, true, true);

		SingularObject sin = SingularObject(hex);
		//sin.FindSingularObject(true, true);
		//sin.FindSingularObject_simple();
		
		sin.FindSingularObjectForBC(se_vector,true, true);
		writeMesh2File(path, filename_converted, hex);

	}
	

	
	outputfilename = path + filename_converted; 

	HexMeshV3f showHexMesh;
	fileManager.readFile(outputfilename, showHexMesh, true, true);

	SingularObject sin = SingularObject(&showHexMesh);
	
	sin.FindSingularObjectForBC(se_vector,true, true);
	//分割面
	BaseComplexSetFiller bac(&showHexMesh);
	bac.SFaceSetSeeking_Simple(true);
	//base-complex

	std::set<BaseComplex, compare_BaseComplex> baseComplexSet;

	//添加奇异边 重新订立起点
	bac.OneBaseComplexFilling(se_vector,baseComplexSet);

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(500, 500);
	glutCreateWindow("Singulation");
	
	//init_line(&showHexMesh,(std::string(argv[1])).compare("true")==0 ? true:false);
	
	//init_faceset(showHexMesh,bac.f_all_set);

	init_basecomplexset(showHexMesh,baseComplexSet);
	
	BaseComplexSimplify base_simplify(&showHexMesh,bac.hf_all_set,bac.f_all_set,baseComplexSet);
	
	std::vector< std::vector<BC_8> > v_cell_vector = base_simplify.HandleBCSimplify(BaseComplexSimplify::re_8_1,BaseComplexSimplify::re_4_1);
	//Init_BC_8_Set(showHexMesh, v_cell_vector);

	glutReshapeFunc(reshape);
	glutDisplayFunc(display2);
	glutIdleFunc(display2);  //设置不断调用显示函数
	glutMouseFunc(Mouse);
	glutKeyboardFunc(KeyFunc);
	glutMotionFunc(onMouseMove);
	glutMainLoop();

	return 0;
}