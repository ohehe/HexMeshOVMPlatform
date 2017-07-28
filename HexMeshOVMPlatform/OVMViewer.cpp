
#include "OVMViewer.h"

using namespace Eigen;
using namespace std;
static float c = M_PI / 180.0f;
static int du = 90, oldmy = -1, oldmx = -1;
static float r = 5.0f, h = 0.0f;
static GLCamera * cam = nullptr;

static float wheel_scale = 1.00f; 
Mesh mesh;
int v_cnt;


GLushort * item_indices;
GLushort * wire_indices;

//对面的渲染顺序
GLushort * halfFace_indices ;  


GLfloat * vertices;
int vertices_len;
GLfloat * linesArray;

//对面点的排序数组
GLfloat * verticesArray;
//对面着色
GLfloat * faceColorArry;

int pos_linesArray = 0; 
static int pos_verticesArray = 0;

short * vertex_color; 
short * wire_color; 


GLboolean mouserdown = GL_FALSE;
GLboolean mouseldown = GL_FALSE;
GLboolean mousemdown = GL_FALSE;

vector<V3f> vertexVector;

/*
*      5-------6
*     /|      /|
*    / |     / |
*   3-------2  |
*   |  4----|--7
*   | /     | /
*   |/      |/
*   0-------1
*/


//
//奇异边渲染
void setupSingularInit(Mesh *m, bool draw_wire_frame) {
	int line_bufferLen = 0;
	OpenVolumeMesh::EdgePropertyT<int> intEProp_sigular = m->request_edge_property<int>("SingularEdgeIndex");

	if (!draw_wire_frame) {
		for (OpenVolumeMesh::EdgeIter edge_iter = m->edges_begin(); edge_iter != m->edges_end(); ++edge_iter) {
			if (intEProp_sigular[*edge_iter] != 0) {
				line_bufferLen++;
			}
		}
		line_bufferLen = line_bufferLen * 3 * 2;
	}
	else {
		line_bufferLen = m->n_edges() * 3 * 2;
	}
	
	
	verticesArray = new GLfloat[line_bufferLen];
	faceColorArry = new GLfloat[4 * (line_bufferLen / 3)];
	//bc渲染颜色
	GLfloat this_bc_color[4] = { 0.20f,0.20f,1.00f,1.0f };
	GLfloat this_no_color[4] = { 0.8f, 0.8f, 0.8f, 0.2f };


	//中心点调整调整
	float remove_xyz[3] = {0.00f,0.00f,0.00f};
	uint64_t count_vertex_m = m->n_vertices();
	for (OpenVolumeMesh::VertexIter v_iter = m->vertices_begin(); v_iter != m->vertices_end(); ++v_iter) {
		
		remove_xyz[0] += m->vertex(*v_iter)[0]/count_vertex_m;
		remove_xyz[1] += m->vertex(*v_iter)[1]/count_vertex_m;
		remove_xyz[2] += m->vertex(*v_iter)[2]/count_vertex_m;
	}


	int i_in = 0;
	for (OpenVolumeMesh::EdgeIter iter = m->edges_begin(); iter != m->edges_end(); ++iter) {
		if (!draw_wire_frame) {
			if (intEProp_sigular[*iter] == 0) continue;
		}
		std::cout << (*iter).idx()<<endl;
		Vec3f vf = m->vertex(m->edge(*iter).from_vertex());
		Vec3f vt = m->vertex(m->edge(*iter).to_vertex());

		if (intEProp_sigular[*iter]) {
			faceColorArry[i_in*4] = *&this_bc_color[0];
			faceColorArry[i_in * 4+1] = *&this_bc_color[1];
			faceColorArry[i_in * 4+2] = *&this_bc_color[2];
			faceColorArry[i_in * 4+3] = *&this_bc_color[3];
		}
		else {
			faceColorArry[i_in * 4] = *&this_no_color[0];
			faceColorArry[i_in * 4+1] = *&this_no_color[1];
			faceColorArry[i_in * 4+2] = *&this_no_color[2];
			faceColorArry[i_in * 4+3] = *&this_no_color[3];
		}
		verticesArray[pos_verticesArray++] = (vf[0]-remove_xyz[0]) * UNIT;
		verticesArray[pos_verticesArray++] = (vf[1]-remove_xyz[1]) * UNIT;
		verticesArray[pos_verticesArray++] = (vf[2]-remove_xyz[2]) * UNIT;
		++i_in;
		if (intEProp_sigular[*iter]) {
			faceColorArry[i_in * 4 ] = *&this_bc_color[0];
			faceColorArry[i_in * 4 +1] = *&this_bc_color[1];
			faceColorArry[i_in * 4 +2 ] = *&this_bc_color[2];
			faceColorArry[i_in * 4 +3 ] = *&this_bc_color[3];
		}
		else {
			faceColorArry[i_in * 4 ] = *&this_no_color[0];
			faceColorArry[i_in * 4 + 1] = *&this_no_color[1];
			faceColorArry[i_in * 4 + 2] = *&this_no_color[2];
			faceColorArry[i_in * 4 + 3] = *&this_no_color[3];
		}
		verticesArray[pos_verticesArray++] = (vt[0] - remove_xyz[0]) * UNIT;
		verticesArray[pos_verticesArray++] = (vt[1] - remove_xyz[1]) * UNIT;
		verticesArray[pos_verticesArray++] = (vt[2] - remove_xyz[2]) * UNIT;
		++i_in;
		
		
	}

	//对奇异点的渲染
	OpenVolumeMesh::VertexPropertyT<bool> boolVProp_sigular = m->request_vertex_property<bool>("IsSigularPoint");
	int size_arr = 0; 
	for (OpenVolumeMesh::VertexIter v_it = m->vertices_begin(); v_it != m->vertices_end(); ++v_it) {
		if (boolVProp_sigular[v_it]) {
			++size_arr; 
		}
	}

	
	/*vertices = new GLfloat[size_arr*3]; 
	int k = 0; 
	for (OpenVolumeMesh::VertexIter v_it = m->vertices_begin(); v_it != m->vertices_end(); ++v_it) {
		if (boolVProp_sigular[v_it]) {
			vertices[k++] = (m->vertex(*v_it))[0]*UNIT; 
			vertices[k++] = (m->vertex(*v_it))[1]*UNIT;
			vertices[k++] = (m->vertex(*v_it))[2]*UNIT;

		}
	}
	vertices_len = size_arr; */
}
//分割面渲染
void setupHalfFaceInit(std::set<HalfFaceHandle, compare_OVM> hf_all_s) {
	int hf_ve_bufferLen = 0; 
	int count_len = hf_all_s.size()*4;
	hf_ve_bufferLen = hf_all_s.size() * 4 * 3;
	verticesArray = new GLfloat[hf_ve_bufferLen];
	faceColorArry = new GLfloat[4 * (hf_ve_bufferLen / 3)];


	std::cout << "半面总计：" << count_len / 4;
	

	float f_remove_to[3] = { 0.00f,0.00f,0.00f };
	OpenVolumeMesh::Vec3f temp_point;
	for (std::set<HalfFaceHandle, compare_OVM>::iterator hf_iter = hf_all_s.begin(); hf_iter != hf_all_s.end(); ++hf_iter) {
		std::pair<OpenVolumeMesh::HalfFaceVertexIter, OpenVolumeMesh::HalfFaceVertexIter> pair = mesh.halfface_vertices(*hf_iter);
		
		for (OpenVolumeMesh::HalfFaceVertexIter hf_v_it = pair.first; hf_v_it != pair.second; ++hf_v_it) {
			temp_point = mesh.vertex(*hf_v_it);
			f_remove_to[0] += temp_point[0] / count_len;
			f_remove_to[1] += temp_point[1] / count_len; 
			f_remove_to[2] += temp_point[2] / count_len;
		}

	}

	//bc渲染颜色
	GLfloat this_bc_color[4] = { 0.20f,1.00f,0.20f,0.3f };

	for (std::set<HalfFaceHandle, compare_OVM>::iterator hf_iter = hf_all_s.begin(); hf_iter != hf_all_s.end(); ++hf_iter) {
		std::vector<HalfEdgeHandle> hf_v =  mesh.halfface(*hf_iter).halfedges();
		for (std::vector<HalfEdgeHandle>::iterator he_iter = hf_v.begin(); he_iter != hf_v.end(); ++he_iter) {
			Vec3f v = mesh.vertex(mesh.halfedge(*he_iter).from_vertex());

			faceColorArry[pos_verticesArray / 3] = *&this_bc_color[0];
			faceColorArry[1 + (pos_verticesArray / 3)] = *&this_bc_color[1];
			faceColorArry[2 + (pos_verticesArray / 3)] = *&this_bc_color[2];
			faceColorArry[3 + (pos_verticesArray / 3)] = *&this_bc_color[3];
			verticesArray[pos_verticesArray++] = (v[0]-f_remove_to[0]) * UNIT;
			verticesArray[pos_verticesArray++] = (v[1]-f_remove_to[1]) * UNIT;
			verticesArray[pos_verticesArray++] = (v[2]-f_remove_to[2]) * UNIT;
		}
	}
}

//基于面的
void setupFaceInit(std::set<FaceHandle, compare_OVM> f_all_s) {
	int hf_ve_bufferLen = 0;
	int count_len = f_all_s.size() * 4;
	hf_ve_bufferLen = f_all_s.size() * 4 * 3;
	verticesArray = new GLfloat[hf_ve_bufferLen];
	faceColorArry = new GLfloat[4 * (hf_ve_bufferLen / 3)];


	std::cout << "面总计：" << count_len / 4;

	

	float f_remove_to[3] = { 0.00f,0.00f,0.00f };
	OpenVolumeMesh::Vec3f temp_point;
	for (std::set<FaceHandle, compare_OVM>::iterator f_iter = f_all_s.begin(); f_iter != f_all_s.end(); ++f_iter) {
		OpenVolumeMesh::OpenVolumeMeshFace face = mesh.face(*f_iter);
		std::vector<HalfEdgeHandle> vector = face.halfedges();
		for (std::vector<HalfEdgeHandle>::iterator he_iter = vector.begin(); he_iter != vector.end(); ++he_iter) {
			temp_point = mesh.vertex(mesh.halfedge(*he_iter).from_vertex());
			f_remove_to[0] += temp_point[0] / count_len;
			f_remove_to[1] += temp_point[1] / count_len;
			f_remove_to[2] += temp_point[2] / count_len;
		}

	}

	//bc渲染颜色
	GLfloat this_bc_color[4] = { 0.20f,0.30f,1.00f,0.3f };
	
	VertexHandle vh_arr_temp[4];
	int pos = 0;

	for (std::set<FaceHandle, compare_OVM>::iterator f_iter = f_all_s.begin(); f_iter != f_all_s.end(); ++f_iter) {
		OpenVolumeMesh::OpenVolumeMeshFace face = mesh.face(*f_iter);
		std::vector<HalfEdgeHandle> vector = face.halfedges();
		for (std::vector<HalfEdgeHandle>::iterator he_iter = vector.begin(); he_iter != vector.end(); ++he_iter) {
			if (pos == 0) {
				vh_arr_temp[0] = mesh.halfedge(*he_iter).from_vertex();
				vh_arr_temp[1] = mesh.halfedge(*he_iter).to_vertex();
				pos += 2;
			}
			else{
				if (mesh.halfedge(*he_iter).from_vertex().idx() == vh_arr_temp[pos - 1]) {
					vh_arr_temp[pos++] = mesh.halfedge(*he_iter).to_vertex();
					for (std::vector<HalfEdgeHandle>::iterator he_i = vector.begin(); he_i != vector.end(); ++he_i) {
						if (vh_arr_temp[pos - 1].idx() == mesh.halfedge(*he_i).from_vertex().idx()) {
							vh_arr_temp[3] = mesh.halfedge(*he_i).to_vertex();
						}
					}
					break;
				}
				
			}
			
		}
		pos = 0; 
		
		for (int i = 0; i < 4; ++i) {
			Vec3f v = mesh.vertex(vh_arr_temp[i]);
			faceColorArry[(pos_verticesArray/3) * 4] = *&this_bc_color[0];
			faceColorArry[1 + (pos_verticesArray/3) * 4] = *&this_bc_color[1];
			faceColorArry[2 + (pos_verticesArray / 3) *4] = *&this_bc_color[2];
			faceColorArry[3 + (pos_verticesArray / 3) *4] = *&this_bc_color[3];
			verticesArray[pos_verticesArray++] = (v[0] - f_remove_to[0]) * UNIT;
			verticesArray[pos_verticesArray++] = (v[1] - f_remove_to[1]) * UNIT;
			verticesArray[pos_verticesArray++] = (v[2] - f_remove_to[2]) * UNIT;
		}
		
	}

}

void setupBaseComplexSet(std::set<BaseComplex, compare_BaseComplex>& baseComplexSet)
{
	//所有点的坐标计数
	uint64_t all_point_xyz_count = 0;
	GLfloat facecolor_arr_4f[5][4] = { {1.00f,0.00f,0.00f,1.00f},
	{0.00f,1.00f,0.00f,1.00f},
	{0.00f,0.00f,1.00f,1.00f},
	{1.00f,1.00f,0.00f,1.00f},
	{0.00f,1.00f,1.00f,1.00f} };

	for (std::set<BaseComplex, compare_BaseComplex>::iterator cb_iter = baseComplexSet.begin(); cb_iter != baseComplexSet.end(); ++cb_iter) {
		BaseComplex basecomplex = *cb_iter;
		all_point_xyz_count += basecomplex.getHalfFaceSet().size()*4*3;
	}
	//分配空间
	verticesArray = new GLfloat[all_point_xyz_count];
	faceColorArry = new GLfloat[4 * (all_point_xyz_count / 3)];


	//统计偏移量
	//中心点调整调整
	float remove_xyz[3] = { 0.00f,0.00f,0.00f };
	float *this_bc_color;

	uint64_t count_vertex_m = mesh.n_vertices();
	for (OpenVolumeMesh::VertexIter v_iter = mesh.vertices_begin(); v_iter != mesh.vertices_end(); ++v_iter) {

		remove_xyz[0] += mesh.vertex(*v_iter)[0] / count_vertex_m;
		remove_xyz[1] += mesh.vertex(*v_iter)[1] / count_vertex_m;
		remove_xyz[2] += mesh.vertex(*v_iter)[2] / count_vertex_m;
	}
	
	int index = 1; 
	for (std::set<BaseComplex, compare_BaseComplex>::iterator cb_iter = baseComplexSet.begin(); cb_iter != baseComplexSet.end(); ++cb_iter) {
		//TODO 很奇怪？？？ 只能是const
		BaseComplex basecomplex = *cb_iter; 
		std::set<HalfFaceHandle, compare_OVM> hf_set_temp = basecomplex.getHalfFaceSet();
		this_bc_color = facecolor_arr_4f[index++ % 5]; 

		for (std::set<HalfFaceHandle, compare_OVM>::iterator hf_iter = hf_set_temp.begin(); hf_iter != hf_set_temp.end(); ++hf_iter) {
			std::pair<OpenVolumeMesh::HalfFaceVertexIter, OpenVolumeMesh::HalfFaceVertexIter> pair = mesh.halfface_vertices(*hf_iter);
			
			for (OpenVolumeMesh::HalfFaceVertexIter hf_ve_it = pair.first; hf_ve_it != pair.second; ++hf_ve_it) {
				Vec3f v = mesh.vertex(*hf_ve_it);
				faceColorArry[(pos_verticesArray / 3) * 4] = this_bc_color[0];
				faceColorArry[1 + (pos_verticesArray / 3) * 4] = this_bc_color[1];
				faceColorArry[2 + (pos_verticesArray / 3) * 4] = this_bc_color[2];
				faceColorArry[3 + (pos_verticesArray / 3) * 4] = this_bc_color[3];
				verticesArray[pos_verticesArray++] = (v[0] - remove_xyz[0]) * UNIT;
				verticesArray[pos_verticesArray++] = (v[1] - remove_xyz[1]) * UNIT;
				verticesArray[pos_verticesArray++] = (v[2] - remove_xyz[2]) * UNIT;
			}

		}//hf
	}//bc


}

//complex面渲染的填充
void setupHalfFaceInit(set<BaseComplex, compare_BaseComplex> baseComplex_set) {
	//申请空间的长度
	int hf_ve_bufferLen = 0;
	for (set<BaseComplex, compare_BaseComplex>::iterator bc_iter = baseComplex_set.begin();
		bc_iter != baseComplex_set.end(); ++bc_iter) {
		BaseComplex bc = (*bc_iter);

		//strip形式存
		hf_ve_bufferLen += bc.getHalfFaceSet().size() * 4 * 3;
	}

	//面 点序列 和着色序列
	verticesArray = new GLfloat[hf_ve_bufferLen];
	faceColorArry = new GLfloat[4*(hf_ve_bufferLen/3)];
	//bc渲染颜色
	GLfloat this_bc_color[4] = {1.00f,1.00f,1.00f,0.5f};
	int time = 0;
	for (set<BaseComplex, compare_BaseComplex>::iterator bc_iter = baseComplex_set.begin(); bc_iter != baseComplex_set.end(); ++bc_iter) {
		BaseComplex bc_temp = (*bc_iter);
		
		if(++time / 2) {
			this_bc_color[0] -= 0.2f;
			this_bc_color[1] -= 0.0f;
			this_bc_color[2] -= 0.0f;
		}
		else {
			this_bc_color[0] -= 0.0f;
			this_bc_color[1] -= 0.2f;
			this_bc_color[2] -= 0.0f;
		}
		std::set<HalfFaceHandle, compare_OVM> hf_set_temp = bc_temp.getHalfFaceSet();

		for (set<HalfFaceHandle>::iterator hf_iter = hf_set_temp.begin();
			hf_iter != hf_set_temp.end(); ++hf_iter) {
			//填充面点数组
			OpenVolumeMesh::OpenVolumeMeshFace face = mesh.halfface(*hf_iter);

			

			std::vector<HalfEdgeHandle> halfedges_temp = face.halfedges();

			for (vector<HalfEdgeHandle>::const_iterator hfh_iter = halfedges_temp.begin();
				hfh_iter != halfedges_temp.end(); ++hfh_iter) {
				Vec3f v = mesh.vertex(mesh.halfedge(*hfh_iter).from_vertex());

				faceColorArry[pos_verticesArray / 3] = *&this_bc_color[0];
				faceColorArry[1 + (pos_verticesArray / 3)] = *&this_bc_color[1];
				faceColorArry[2 + (pos_verticesArray / 3)] = *&this_bc_color[2];
				faceColorArry[3 + (pos_verticesArray / 3)] = *&this_bc_color[3];
				verticesArray[pos_verticesArray++] = v[0]*UNIT;
				verticesArray[pos_verticesArray++] = v[1]*UNIT;
				verticesArray[pos_verticesArray++] = v[2]*UNIT;
			}

		}
	}
}

       
void RotateX(float angle)
{
	float d = cam->getDist();
	int cnt = 100;
	float theta = angle / cnt;
	float slide_d = -2 * d * sin(theta * M_PI / 360);
	cam->yaw(theta / 2);
	for (; cnt != 0; --cnt)
	{
		cam->slide(slide_d, 0, 0);
		cam->yaw(theta);
	}
	cam->yaw(-theta / 2);
}

void RotateY(float angle)
{
	float d = cam->getDist();
	int cnt = 100;
	float theta = angle / cnt;
	float slide_d = 2 * d * sin(theta * M_PI / 360);
	cam->pitch(theta / 2);
	for (; cnt != 0; --cnt)
	{
		cam->slide(0, slide_d, 0);
		cam->pitch(theta);
	}
	cam->pitch(-theta / 2);
}


void drawAllVertex() {
	
	int i = 0; 
	V3f v;
	while (i < vertexVector.size()) {
		v = vertexVector[i]; 
		glPointSize(3.0f);
		glBegin(GL_POINTS);
		if (vertex_color[i] == 1) {
			glColor3f(1.0f,0.0f,0.0f); 
		}
		else {
			glColor3f(1.0f,1.0f,1.0f); 
		}
		glVertex3f(v.x,v.y,v.z);
		glEnd();

		i++;
	}
}
void drawhex(V3f p0, V3f p1, V3f p2, V3f p3, V3f p4, V3f p5, V3f p6, V3f p7,
	V3f itemColor, V3f wireColor)
{
	//四周
	
	glBegin(GL_QUAD_STRIP);
	glColor3f(itemColor.x, itemColor.y, itemColor.z);
	glVertex3f(p0.x * UNIT, p0.y * UNIT, p0.z * UNIT);
	glVertex3f(p3.x * UNIT, p3.y * UNIT, p3.z * UNIT);
	glVertex3f(p1.x * UNIT, p1.y * UNIT, p1.z * UNIT);
	glVertex3f(p2.x * UNIT, p2.y * UNIT, p2.z * UNIT);
	glVertex3f(p7.x * UNIT, p7.y * UNIT, p7.z * UNIT);
	glVertex3f(p6.x * UNIT, p6.y * UNIT, p6.z * UNIT);
	glVertex3f(p4.x * UNIT, p4.y * UNIT, p4.z * UNIT);
	glVertex3f(p5.x * UNIT, p5.y * UNIT, p5.z * UNIT);
	glVertex3f(p0.x * UNIT, p0.y * UNIT, p0.z * UNIT);
	glVertex3f(p3.x * UNIT, p3.y * UNIT, p3.z * UNIT);
	glEnd();

	//上下盖
	glBegin(GL_QUADS);
	glColor3f(itemColor.x, itemColor.y, itemColor.z);
	glVertex3f(p2.x * UNIT, p2.y * UNIT, p2.z * UNIT);
	glVertex3f(p3.x * UNIT, p3.y * UNIT, p3.z * UNIT);
	glVertex3f(p5.x * UNIT, p5.y * UNIT, p5.z * UNIT);
	glVertex3f(p6.x * UNIT, p6.y * UNIT, p6.z * UNIT);
	glVertex3f(p0.x * UNIT, p0.y * UNIT, p0.z * UNIT);
	glVertex3f(p1.x * UNIT, p1.y * UNIT, p1.z * UNIT);
	glVertex3f(p7.x * UNIT, p7.y * UNIT, p7.z * UNIT);
	glVertex3f(p4.x * UNIT, p4.y * UNIT, p4.z * UNIT);
	glEnd();

	
}

// glBegin glEnd
void display()
{

	


	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glPushMatrix();

	glEnable(GL_POINT_SMOOTH);
	glHint(GL_POINT_SMOOTH, GL_NICEST);
	glEnable(GL_LINE_SMOOTH);
	glHint(GL_LINE_SMOOTH, GL_NICEST);

	glLoadIdentity();


	//gluLookAt(r*cos(c*du), h, r*sin(c*du), 0, 0, 0, 0, 1, 0); //从视点看远点,y轴方向(0,1,0)是上方向  
	//	gluLookAt(r*cos(c*du), 0, r*sin(c*du) + 50, 0, 0, 0, 0, 1, 0); //从视点看远点,y轴方向(0,1,0)是上方向  
	cam->setModelViewMatrix();

	drawAllVertex();
	//drawAllLines();
	/*
	int i=0,j = 0;
	while (i < (sizeof(vertices) / sizeof(vertices[0]))/24)
	{
		drawhex(
			V3f(vertices[j++], vertices[j++], vertices[j++]),
			V3f(vertices[j++], vertices[j++], vertices[j++]),
			V3f(vertices[j++], vertices[j++], vertices[j++]),
			V3f(vertices[j++], vertices[j++], vertices[j++]),
			V3f(vertices[j++], vertices[j++], vertices[j++]),
			V3f(vertices[j++], vertices[j++], vertices[j++]),
			V3f(vertices[j++], vertices[j++], vertices[j++]),
			V3f(vertices[j++], vertices[j++], vertices[j++]),
			V3f(vertices[j++], vertices[j++], vertices[j++]),
			V3f(vertices[j++], vertices[j++], vertices[j++]));
	}
	*/
	

	glPopMatrix();
	glutSwapBuffers();
}

// glDrawElements
void display2()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glPushMatrix();

	glEnable(GL_POINT_SMOOTH);
	glHint(GL_POINT_SMOOTH, GL_NICEST);
	glEnable(GL_LINE_SMOOTH);
	glHint(GL_LINE_SMOOTH, GL_NICEST);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	//glDisable(GL_DEPTH_TEST);
	
	glLoadIdentity();
	cam->setModelViewMatrix();
	glScalef(wheel_scale, wheel_scale, wheel_scale);

	const int indexNum = pos_verticesArray;
	// 启用
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	//加载
	glVertexPointer(3, GL_FLOAT, 0, verticesArray);
	glColorPointer(4, GL_FLOAT, 0, faceColorArry);
	//glColor4f(0.0f, 0.0f, 1.0f, 0.5f);
	//解引用
	GLuint *indices = new GLuint[pos_verticesArray / 3];
	for (int i = 0; i < pos_verticesArray / 3; ++i) {
		indices[i] = i;
	}

	glDrawElements(GL_QUADS, pos_verticesArray / 3, GL_UNSIGNED_INT, indices);
	glutSwapBuffers();

	delete indices;
}

// glDrawArray
void display3()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glPushMatrix();

	glEnable(GL_POINT_SMOOTH);
	glHint(GL_POINT_SMOOTH, GL_NICEST);
	glEnable(GL_LINE_SMOOTH);
	glHint(GL_LINE_SMOOTH, GL_NICEST);
	
	/*glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);
*/
	glLoadIdentity();
	cam->setModelViewMatrix();
	glScalef(wheel_scale, wheel_scale, wheel_scale);
	
	const int indexNum = pos_verticesArray;
	// 启用
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	//加载
	glVertexPointer(3, GL_FLOAT, 0, verticesArray);
	glColorPointer(4, GL_FLOAT, 0, faceColorArry);
	//glColor4f(0.0f, 0.0f, 1.0f, 0.5f);
	//解引用
	GLuint *indices = new GLuint[pos_verticesArray/3];
	for (int i = 0; i < pos_verticesArray / 3; ++i) {
		indices[i] = i;
	}

	glDrawElements(GL_LINES, pos_verticesArray / 3, GL_UNSIGNED_INT, indices);


	//glPointSize(3.0f);
	//glColor3f(1.00f,0.00f,0.00f); 
	//glBegin(GL_POINTS);//必须是加上s，要不然显示不了
	//for (int i = 0; i < vertices_len; ++i) {
	//	glVertex3f(vertices[i*3], vertices[i*3+1], vertices[i*3+2]);
	//}
	//glEnd();

	//glColor3f(0.3f, 0.8f, 0.3f);
	//glDrawArrays(GL_QUADS, 0, v_cnt * 3);
	/*
	// 绘制边
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, linesArray);
	glColor3f(1.0f, 1.0f, 1.0f);
	glDrawArrays(GL_LINES, 0, v_cnt * 3);
	*/
	glutSwapBuffers();
}
void KeyFunc(unsigned char key, int x, int y) {
	if (key == 'z') {
		wheel_scale += UNIT_ZOOM;
		glutPostRedisplay();
	}
	else if (key == 'x') {
		if (wheel_scale - UNIT_ZOOM > 0) {
			wheel_scale -= UNIT_ZOOM;
		}
		glutPostRedisplay();
	}
	
}
void Mouse(int button, int state, int x, int y)		// mouse click
{
	if (state == GLUT_DOWN)	// 第一次鼠标按下时记录在窗口中的坐标
	{
		if (button == GLUT_RIGHT_BUTTON)
			mouserdown = GL_TRUE;
		else if (button == GLUT_LEFT_BUTTON)
			mouseldown = GL_TRUE;
		else if (button == GLUT_MIDDLE_BUTTON)
			mousemdown = GL_TRUE;
	}
	else
	{
		if (button == GLUT_RIGHT_BUTTON)
			mouserdown = GL_FALSE;
		else if (button == GLUT_LEFT_BUTTON)
			mouseldown = GL_FALSE;
		else if (button == GLUT_MIDDLE_BUTTON)
			mousemdown = GL_FALSE;
	}
	oldmx = x, oldmy = y;
}

void onMouseMove(int x, int y)
{
	int dx = x - oldmx;
	int dy = y - oldmy;
	if (mouseldown == GL_TRUE)
	{
		RotateX(dx);
		RotateY(dy);
	}
	else if (mouserdown == GL_TRUE)
	{
		cam->roll(dx);
	}
	else if (mousemdown == GL_TRUE)
	{
		cam->slide(-dx, dy, 0);
	}
	oldmx = x;
	oldmy = y;
}

void init_line(Mesh * m,bool draw_wire_frame)
{
	glEnable(GL_DEPTH_TEST);
	Vector3d pos(0.0, 0.0, 100.0);
	Vector3d target(0.0, 0.0, 0.0);
	Vector3d up(0.0, 1.0, 0.0);
	cam = new GLCamera(pos, target, up);
	glClearColor(0.7f, 0.7f, 0.7f, 0.5f);
	//glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	//setupPointersByArray();
	//setupHalfFaceInit(baseComplex_set);
	//setupHalfFaceInit(hf_all_s);
	setupSingularInit(m,draw_wire_frame);
}

void init_faceset(Mesh m,std::set<FaceHandle, compare_OVM> f_all_s)
{
	mesh = m;
	glEnable(GL_DEPTH_TEST);
	Vector3d pos(0.0, 0.0, 100.0);
	Vector3d target(0.0, 0.0, 0.0);
	Vector3d up(0.0, 1.0, 0.0);
	cam = new GLCamera(pos, target, up);
	glClearColor(0.7f, 0.7f, 0.7f, 0.5f);

	//glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	//setupPointersByArray();
	//setupHalfFaceInit(baseComplex_set);
	//setupHalfFaceInit(hf_all_s);
	setupFaceInit(f_all_s);
}
/*
void Init_BC_8_Set(Mesh & showHexMesh, std::vector<std::vector<BC_8>> v_cell_vector)
{
	std::set<BaseComplex, compare_BaseComplex> set_temp;
	//填充
	for (std::vector<std::vector<BC_8>>::iterator bc8_v_it = v_cell_vector.begin(); bc8_v_it != v_cell_vector.end(); ++bc8_v_it) {
		//填充面即可
		BaseComplex temp_basecomplex;
		for (std::vector<BC_8>::iterator bc8_iter = (*bc8_v_it).begin(); bc8_iter != (*bc8_v_it).end(); ++bc8_iter) {
			for((*bc8_iter))
			temp_basecomplex.insertHalfFace();
		}
	}
	init_basecomplexset(showHexMesh,set_temp);
}*/
//填充base_complex集合
void init_basecomplexset(Mesh &m, std::set<BaseComplex, compare_BaseComplex> &baseComplexSet) {
	//TODO：这是错的 未共用同址 等会改过来
	mesh = m;
	glEnable(GL_DEPTH_TEST);
	Vector3d pos(0.0, 0.0, 100.0);
	Vector3d target(0.0, 0.0, 0.0);
	Vector3d up(0.0, 1.0, 0.0);
	cam = new GLCamera(pos, target, up);
	glClearColor(0.7f, 0.7f, 0.7f, 0.5f);

	setupBaseComplexSet(baseComplexSet); 
}


void reshape(int w, int h)
{
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	//gluPerspective(75.0f, (float)w / h, 1.0f, 1000.0f);
	cam->setShape(45.0, (GLfloat)w / (GLfloat)h, 0.1, 1000.0);
	glMatrixMode(GL_MODELVIEW);
}
