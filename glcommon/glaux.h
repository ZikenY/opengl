#if !defined(GLAUX_INCLUDED_)
#define GLAUX_INCLUDED_

//LIBS += /usr/lib/x86_64-linux-gnu/mesa/libGL.so
//LIBS += -L/usr/local/lib -lGLEW -lglfw -lglut -lGLU -lSDL2 -lSDL2_image

#include <vector>
#include <string>
using namespace std;

#ifdef __GNUC__
#include <GL/glew.h>
#include <GL/freeglut.h>

#endif

#include "loadshaders.h"

#ifdef _MSC_FULL_VER
#pragma comment(lib, "opengl32.lib")  
#pragma comment(lib, "glu32.lib")
#pragma comment(lib, "c:\\sdks\\freeglut\\lib\\freeglut.lib")
#pragma comment(lib, "c:\\sdks\\glew\\lib\\Release\\Win32\\glew32.lib")

//#define GLEW_STATIC
#include "c:\\sdks\\glew\\include\\GL\\glew.h"
#include "c:\\sdks\\freeglut\\include\\GL\\glut.h"
#endif

#include <GL/gl.h>
#include <GL/glu.h>
#include "matrix4f.h"

namespace glcommon
{

#define PI (3.14159265)
#define RAD (PI / 180.0f)

struct Light
{
    vec3f ambient;
    vec3f diffuse;
    vec3f specular;
};

struct DirLight : public Light
{
    vec3f direction;
};

struct PointLight : public Light
{
    vec3f position;
    vec3f attenuation;
};

struct transform_params
{
    string model_name;
    string texture_name;
    GLfloat rotate_x, rotate_y, rotate_z;
    GLfloat translate_x, translate_y, translate_z;
    GLfloat scale_x, scale_y, scale_z;
    matrix4f trackball;
    transform_params();
    void reset();
};

struct InitialAux
{
    GLfloat fovy;
    float target_phi, target_sita;
    GLfloat camera_pos_x, camera_pos_y, camera_pos_z;
    GLfloat camera_up_x, camera_up_y, camera_up_z;
    GLfloat rotate_x, rotate_y, rotate_z;
    GLfloat translate_x, translate_y, translate_z;
    GLfloat scale_x, scale_y, scale_z;
    GLfloat material_shininess, material_alpha;
    GLfloat pointlight1_radius, pointlight1_phi, pointlight1_sita;
    DirLight dirlight0;
    PointLight pointlight1;
    matrix4f trackball;
    InitialAux();
};

extern matrix4f perspective_matrix;
extern vector<transform_params> model_transforms;

extern float data_bound_x_max;
extern float data_bound_x_min;
extern float data_bound_y_max;
extern float data_bound_y_min;
extern float data_bound_z_max;
extern float data_bound_z_min;
extern GLfloat max_r;
extern GLfloat fovy;

extern matrix4f trackball;

extern float target_phi, target_sita;
extern GLfloat camera_pos_x, camera_pos_y, camera_pos_z;
extern GLfloat camera_up_x, camera_up_y, camera_up_z;
extern GLfloat rotate_x, rotate_y, rotate_z;
extern GLfloat translate_x, translate_y, translate_z;
extern GLfloat scale_x, scale_y, scale_z;

extern GLfloat pointlight1_radius, pointlight1_phi, pointlight1_sita;
extern DirLight dirlight0;
extern PointLight pointlight1;
extern float material_shininess;
extern float material_alpha;

extern GLuint texture_floor_id;
extern GLuint vao_floor, vbo_floor, ibo_floor;

extern vector<vec3f> planexz;
extern bool _with_material;

#define VERTEX_POS_SIZE         3   // xyz
#define VERTEX_NORMAL_SIZE      3   // xyz
#define VERTEX_UV_SIZE          2   // uv
#define VERTEX_POS_OFFSET       0
#define VERTEX_NORMAL_OFFSET    3
#define VERTEX_UV_OFFSET        6
const GLsizei VERTICES_STRIDE = VERTEX_POS_SIZE + VERTEX_NORMAL_SIZE
    + VERTEX_UV_SIZE;

#define RENDER_MODE_SKELETON    0
#define RENDER_MODE_SURFACE     1
#define RENDER_MODE_HYBIRD      2
extern int rendermode;

extern InitialAux initaux_saved;
extern bool trackball_enable;

extern void glutWindowResize(GLsizei w, GLsizei h);
extern void CameraPositionUpdate();

extern void reset_space(InitialAux &initaux, bool reset_cam_pos);
extern void glut_glew_init(string title, void(*renderscene)(void), void(*resize)(int, int),
    bool(*userkey)(unsigned char key), void(*timer)(int), void(*storescene)(), void(*loadscene)());

extern void model_matrix(matrix4f &m);
extern void model_matrix(matrix4f &m, vec3f &model_center);
extern void model_matrix(matrix4f &m, transform_params &transform);
extern void lookat_matrix(matrix4f &m);
extern void pointlight_matrix(matrix4f &m);
extern void modelview_matrix(matrix4f &m);

extern void init_center();
extern void recenter_polygons(vector<pos_normal_uv> &vertex_data, mesh_bound &bound);
extern void recenter_polygons(vector<vector<float> > &_polygons);

extern void create_planexz();
extern void draw_planexz(GLfloat r, GLfloat g, GLfloat b);
extern void initshader_planexz();
extern void shade_planexz();
extern void releaseshader_planexz();

extern GLuint create_texture(string filename, GLenum activetexture = GL_TEXTURE0);

extern void render_floor(shader_handles &shader_main);
extern void create_floor(shader_handles &shader_main, string floor_texturefile);
extern void release_floor();

extern void render_skybox();
extern void create_skybox(string negx, string negy, string negz, string posx, string posy, string posz);
extern void release_skybox();

extern void cube(vector<pos_normal_uv> &cube_data, vector<unsigned int> &cube_indices);

extern void gl_print(const char *text, int x, int y, bool from_top = true, bool from_left = true,
    float r = 0.37f, float g = 0.89, float b = 0.25, void* font = GLUT_BITMAP_9_BY_15);
extern void print_control_hint();

}
#endif
