#ifdef QT_CORE_LIB
#include <QCoreApplication>
#endif

#ifdef _MSC_FULL_VER
#include <tchar.h>
#include <windows.h>
#endif

#include <iostream>
#include <stdexcept>
#include <iostream>
#include <cmath>

#include "../commoninclude/commoninclude.h"
#include "../commoninclude/stringfuncs.h"
using namespace ziken;
using namespace std;

#ifdef __GNUC__
#include <GL/glew.h>
#include <GL/freeglut.h>
#endif

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
#include "../glcommon/matrix4f.h"
#include "../glcommon/loadshaders.h"

using namespace glcommon;

GLuint vbo_screen, ibo_screen;
shader_handles shader_main;
GLuint attrib_modelview_matrix = -1;
GLuint attrib_perspective_matrix = -1;
matrix4f perspective_matrix;

GLfloat camera_pos_x, camera_pos_y, camera_pos_z;
GLfloat camera_target_x, camera_target_y, camera_target_z;
GLfloat camera_up_x, camera_up_y, camera_up_z;
GLfloat fovy;

vec3f model_center;
GLfloat translate_x, translate_y, translate_z;
GLfloat scale_x, scale_y, scale_z;
GLfloat rotate_y, rotate_x, rotate_z;

GLfloat pointlight_radius = 1.9f;

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

DirLight dirlight0;
PointLight pointlight1;
GLfloat material_shininess;
GLfloat material_alpha;

vec3f material_rgb;
GLfloat attenuate_factor;
GLfloat pointlight_hitradius;
GLfloat hit_tolerance;

#define PI (3.14159265)
#define RAD (PI / 180.0f)
#define VERTEX_POS_SIZE         3 // xyz
#define VERTEX_NORMAL_SIZE      3 // xyz
#define VERTEX_POS_OFFSET       0
#define VERTEX_NORMAL_OFFSET    3
const GLsizei VERTICES_STRIDE = VERTEX_POS_SIZE + VERTEX_NORMAL_SIZE;

static GLfloat screen_data[] = 
{
    -1.f, 1.f, 0.f,    -1.28f, 0.98f, -1.0f, // top_left
    -1.f, -1.f, 0.f,   -1.280f, -0.98f, -1.0f, // bottom_left
    1.f, -1.f, 0.f,    1.280f, -0.980f, -1.f, // bottom_right
    1.f, 1.f, 0.f,     1.280f, 0.98f, -1.f, // top_right
};

static GLuint screen_indices[] = { 0, 1, 2, 0, 2, 3 };

vec3f sphere0_center;
GLfloat sphere0_radius;
vec3f sphere1_center;
GLfloat sphere1_radius;
vec3f sphere2_center;
GLfloat sphere2_radius;
vec3f plane_point;
vec3f plane_normal;

bool scene_update = false;
bool leftbotton = false;
bool rightbotton = false;
bool middlebotton = false;
int oldmx, oldmy;

static bool rc_set = false;
GLfloat enable_pointlight = 1.f;
GLfloat enable_directionallight = 0.f;


void update_corner_rays(float aspect)
{
    vec3f u0(0, 1, 0), u1(0, 1, 0), u2(0, 1, 0), u3(0, 1, 0);
    vec3f v0(1, 0, 0), v1(1, 0, 0), v2(1, 0, 0), v3(1, 0, 0);
    vec3f f0(0, 0, -1), f1(0, 0, -1), f2(0, 0, -1), f3(0, 0, -1);
    vec3f r[4];
    for (int i = 0; i < 3; i++)
    {
        r[0][i] = f0[i] - v0[i] * aspect* tan(fovy*RAD / 2) + u0[i] * tan(fovy*RAD / 2);// top left
        r[1][i] = f1[i] - v1[i] * aspect* tan(fovy*RAD / 2) - u1[i] * tan(fovy*RAD / 2);//bottom left
        r[2][i] = f2[i] + v2[i] * aspect* tan(fovy*RAD / 2) - u2[i] * tan(fovy*RAD / 2);//bottom right
        r[3][i] = f3[i] + v3[i] * aspect* tan(fovy*RAD / 2) + u3[i] * tan(fovy*RAD / 2);//top right
    }

    for (int j = 0; j < 4; j++)
    {// which ray
        for (int i = 0; i < 3; i++)
        {// which component
            screen_data[VERTICES_STRIDE*j + VERTEX_POS_SIZE + i] = r[j][i];
        }
    }

    glBindBuffer(GL_ARRAY_BUFFER, vbo_screen);
    glBufferData(GL_ARRAY_BUFFER, 4*6*sizeof(float), screen_data, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void setup_rc()
{
    if (rc_set)
    {
        cout << "u should not setup_rc twice" << endl;
        return;
    }

    // pass the modelview/projection matrix to uniform variable
    attrib_modelview_matrix = glGetUniformLocation(shader_main.handler_program, "modelview_matrix");
    attrib_perspective_matrix = glGetUniformLocation(shader_main.handler_program, "perspective_matrix");

    glGenBuffers(1, &ibo_screen);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_screen);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6*sizeof(GLuint),
        screen_indices, GL_STATIC_DRAW);

    glGenBuffers(1, &vbo_screen);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_screen);
//    glBufferData(GL_ARRAY_BUFFER, 4*sizeof(pos_normal), screen_data, GL_STATIC_DRAW);

    shader_main.do_attrib("screen", VERTEX_POS_SIZE, GL_FLOAT, GL_FALSE, VERTICES_STRIDE*sizeof(GL_FLOAT),
        (GLvoid*)(VERTEX_POS_OFFSET*sizeof(GL_FLOAT)));
    shader_main.do_attrib("corner_rays", VERTEX_NORMAL_SIZE, GL_FLOAT, GL_FALSE,
        VERTICES_STRIDE*sizeof(GL_FLOAT), (GLvoid*)(VERTEX_NORMAL_OFFSET*sizeof(GL_FLOAT)));

    // unbind the VBO and IBO
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    
    rc_set = true;
}

void model_matrix(matrix4f &m, vec3f model_center)
{
    // step 4
    // apply translate
    multipy_translate(m, translate_x, translate_y, translate_z);

    //step 3
    // --------- restore the original model center ------------
    multipy_translate(m, model_center[0], model_center[1], model_center[2]);

    // setp 2
    // apply rotation & scale
    multipy_scale(m, scale_x, scale_y, scale_z);
    multipy_rotate(m, rotate_x, 1.0f, 0.0f, 0.0f);
    multipy_rotate(m, rotate_y, 0.0f, 1.0f, 0.0f);
    multipy_rotate(m, rotate_z, 0.0f, 0.0f, 1.0f);

    //step 1
    // ---------- move the model center to the center of world ----------
    multipy_translate(m, -model_center[0], -model_center[1], -model_center[2]);
}

void renderscene()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    shader_main.use_me();
    // invoke fake draw screen
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_screen);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_screen);

    matrix4f modelmatrix;
    modelmatrix.loadidentity();
    model_matrix(modelmatrix, model_center);
    GLuint gluniform_id = glGetUniformLocation(shader_main.handler_program, "model_matrix");
    glUniformMatrix4fv(gluniform_id, 1, GL_FALSE, modelmatrix.pointer());

    matrix4f lookatmatrix;
    lookatmatrix.loadidentity();
    multipy_lookat(lookatmatrix, camera_pos_x, camera_pos_y, camera_pos_z,
        camera_target_x, camera_target_y, camera_target_z,
        camera_up_x, camera_up_y, camera_up_z);
    gluniform_id = glGetUniformLocation(shader_main.handler_program, "lookat_matrix");
    glUniformMatrix4fv(gluniform_id, 1, GL_FALSE, lookatmatrix.pointer());

    matrix4f modelview;
    modelview.loadidentity();
    m4f_multiply(modelview, lookatmatrix, modelmatrix);   ////    c = a * b 
    glUniformMatrix4fv(attrib_modelview_matrix, 1, GL_FALSE, modelview.pointer());

    // 3 sphere with a plane
    gluniform_id = glGetUniformLocation(shader_main.handler_program, "_sphere0_center");
    glUniform3fv(gluniform_id, 1, (GLfloat*)sphere0_center._data);
    gluniform_id = glGetUniformLocation(shader_main.handler_program, "sphere0_radius");
    glUniform1f(gluniform_id, sphere0_radius);
    gluniform_id = glGetUniformLocation(shader_main.handler_program, "_sphere1_center");
    glUniform3fv(gluniform_id, 1, (GLfloat*)sphere1_center._data);
    gluniform_id = glGetUniformLocation(shader_main.handler_program, "sphere1_radius");
    glUniform1f(gluniform_id, sphere1_radius);
    gluniform_id = glGetUniformLocation(shader_main.handler_program, "_sphere2_center");
    glUniform3fv(gluniform_id, 1, (GLfloat*)sphere2_center._data);
    gluniform_id = glGetUniformLocation(shader_main.handler_program, "sphere2_radius");
    glUniform1f(gluniform_id, sphere2_radius);
    gluniform_id = glGetUniformLocation(shader_main.handler_program, "_plane_point");
    glUniform3fv(gluniform_id, 1, (GLfloat*)plane_point._data);
    gluniform_id = glGetUniformLocation(shader_main.handler_program, "_plane_normal");
    glUniform3fv(gluniform_id, 1, (GLfloat*)plane_normal._data);
    gluniform_id = glGetUniformLocation(shader_main.handler_program, "attenuate_factor");
    glUniform1f(gluniform_id, attenuate_factor);
    gluniform_id = glGetUniformLocation(shader_main.handler_program, "hit_tolerance");
    glUniform1f(gluniform_id, hit_tolerance);
    gluniform_id = glGetUniformLocation(shader_main.handler_program, "pointlight_hitradius");
    glUniform1f(gluniform_id, pointlight_hitradius);

    // deal with material
    gluniform_id = glGetUniformLocation(shader_main.handler_program, "material_rgb");
    glUniform3fv(gluniform_id, 1, (GLfloat*)material_rgb._data);
    GLfloat specularpower = material_shininess * 1.f;
    gluniform_id = glGetUniformLocation(shader_main.handler_program, "material_specularpower");
    glUniform1f(gluniform_id, specularpower);
    GLfloat transparency = material_alpha;
    gluniform_id = glGetUniformLocation(shader_main.handler_program, "material_alpha");
    glUniform1f(gluniform_id, transparency);

    // deal with the directional light
    gluniform_id = glGetUniformLocation(shader_main.handler_program, "dirlight_ambient");
    glUniform3fv(gluniform_id, 1, (GLfloat*)&dirlight0.ambient);
    gluniform_id = glGetUniformLocation(shader_main.handler_program, "dirlight_diffuse");
    glUniform3fv(gluniform_id, 1, (GLfloat*)&dirlight0.diffuse);
    gluniform_id = glGetUniformLocation(shader_main.handler_program, "dirlight_specular");
    glUniform3fv(gluniform_id, 1, (GLfloat*)&dirlight0.specular);
    vec3f direction3f = dirlight0.direction;
    vec3f_normalize(direction3f);
    gluniform_id = glGetUniformLocation(shader_main.handler_program, "dirlight_direction");
    glUniform3fv(gluniform_id, 1, (GLfloat*)&direction3f);

    // deal with the point light
    gluniform_id = glGetUniformLocation(shader_main.handler_program, "pointlight_ambient");
    glUniform3fv(gluniform_id, 1, (GLfloat*)&pointlight1.ambient);
    gluniform_id = glGetUniformLocation(shader_main.handler_program, "pointlight_diffuse");
    glUniform3fv(gluniform_id, 1, (GLfloat*)&pointlight1.diffuse);
    gluniform_id = glGetUniformLocation(shader_main.handler_program, "pointlight_specular");
    glUniform3fv(gluniform_id, 1, (GLfloat*)&pointlight1.specular);
    gluniform_id = glGetUniformLocation(shader_main.handler_program, "pointlight_attenuation");
    glUniform3fv(gluniform_id, 1, (GLfloat*)&pointlight1.attenuation);
    gluniform_id = glGetUniformLocation(shader_main.handler_program, "pointlight_position");
    glUniform3fv(gluniform_id, 1, (GLfloat*)&pointlight1.position);

    gluniform_id = glGetUniformLocation(shader_main.handler_program, "enable_pointlight");
    glUniform1f(gluniform_id, enable_pointlight);
    gluniform_id = glGetUniformLocation(shader_main.handler_program, "enable_directionallight");
    glUniform1f(gluniform_id, enable_directionallight);
    
    glEnable(GL_BLEND);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDrawElements(GL_TRIANGLES, (GLsizei)6, GL_UNSIGNED_INT, 0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    // finished drawing
    shader_main.do_not_use_me();
    glutSwapBuffers();
}

void glutwindowresize(GLsizei w, GLsizei h)
{
    glViewport(0, 0, w, h);
    GLfloat aspect = (GLfloat)w / (GLfloat)h;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    matrix4f m;
    m4f_perspective(m, fovy, aspect, 1.f, 100000.0f);
    glLoadMatrixf(&m._data[0][0]);

    if (rc_set)
        update_corner_rays(aspect);

    // my home-made perspective matrix
    m4f_perspective(perspective_matrix, fovy, aspect, 0.3f, 100000.0f);

    // pass the perspective matrix to the shader
    shader_main.use_me();
    glUniformMatrix4fv(attrib_perspective_matrix, 1, GL_FALSE, perspective_matrix.pointer());
    shader_main.do_not_use_me();
}

bool isMouseWheel(int button, int state)
{
    // It's a wheel event
    if ((button == 3) || (button == 4))
    {
        // Each wheel event reports like a button click, GLUT_DOWN then GLUT_UP
        if (state == GLUT_UP)
            return true; // Disregard redundant GLUT_UP events

        if (button == 3)
        {
            pointlight1.position.y += 0.06f;
        }
        else
        {
            pointlight1.position.y -= 0.06f;
        }

        scene_update = true;
        glutPostRedisplay();
        return true;
    }

    return false;
}

void glutMouseBotton(int button, int state, int x, int y)
{
    if (isMouseWheel(button, state))
        return;

    if (button == GLUT_LEFT_BUTTON
        || button == GLUT_RIGHT_BUTTON
        || button == GLUT_MIDDLE_BUTTON)
    {
        scene_update = true;
    }

    if (button == GLUT_LEFT_BUTTON)
    {
        if (state == GLUT_DOWN) {
            leftbotton = true;
            oldmx = x;
            oldmy = y;
        }else{
            leftbotton = false;
        }
    }

    if (button == GLUT_RIGHT_BUTTON)
    {
        if (state == GLUT_DOWN) {
            rightbotton = true;
            oldmx = x;
            oldmy = y;
        }
        else{
            rightbotton = false;
        }
    }
}

void glutMouseMove(int x, int y)
{
    if (leftbotton)
    {
        rotate_y += x - oldmx;
        rotate_x += y - oldmy;
    }

    oldmx = x;
    oldmy = y;

    glutPostRedisplay();
}

void glutKeyboard(unsigned char key, int x, int y)
{
    scene_update = true;
    switch (key)
    {
    case 27:
#ifdef __GNUC__
        glutLeaveMainLoop();
#endif
        break;

    case 61:    // +
        pointlight_radius += 0.1;
        break;

    case 45:    // -
        pointlight_radius -= 0.1;
        break;

    case 112:   // p
        enable_pointlight = enable_pointlight > 0 ? -1 : 1;
        break;

    case 100:   // d
        enable_directionallight = enable_directionallight > 0 ? -1 : 1;
        break;

    default: {}
    }
    glutPostRedisplay();
}

void init_scene()
{
    translate_x = translate_y = translate_z = 0.f;
    scale_x = scale_y = scale_z = 0.69f;
    rotate_x = 68.f;
    rotate_y = -116.f;
    rotate_z = 0.f;

    camera_pos_x = camera_pos_y = camera_pos_z = 0.f;
    camera_target_x = 0;
    camera_target_y = 0;
    camera_target_z = -1;
    camera_up_x = 0;
    camera_up_y = 1;
    camera_up_z = 0;
    fovy = 69;

    hit_tolerance = 0.0001f;
    pointlight_hitradius = 0.07f;
    attenuate_factor = 16.9f;
    material_rgb = vec3f(0.69f, 0.69f, 0.89f);

    GLfloat light_direction[] = { 1.3, 2.0, 1.3 };
    GLfloat dirlight0_ambient[] = { 0.01124f, 0.01124f, 0.01124f };
    GLfloat dirlight0_diffuse[] = { 0.12135f, 0.1335f, 0.115f };
    GLfloat dirlight0_specular[] = { 0.1269f, 0.109f, 0.121f };
    updatev(dirlight0.ambient, dirlight0_ambient);
    updatev(dirlight0.diffuse, dirlight0_diffuse);
    updatev(dirlight0.specular, dirlight0_specular);
    updatev(dirlight0.direction, light_direction);

    GLfloat light_position[] = { -0.5, 2.2, -1.5 };
    GLfloat pointlight1_ambient[] = { 0.02f, 0.03f, 0.03f, 1.0f };
    GLfloat pointlight1_diffuse[] = { 0.195f, 0.215f, 0.2075f, 1.0f };
    GLfloat pointlight1_specular[] = { 0.42f, 0.47f, 0.52f, 1.0f };
    updatev(pointlight1.ambient, pointlight1_ambient);
    updatev(pointlight1.diffuse, pointlight1_diffuse);
    updatev(pointlight1.specular, pointlight1_specular);
    updatev(pointlight1.position, light_position);
    updatev(pointlight1.attenuation, 0.1, 0.19, 0.0269);
    material_shininess = 0.87f;
    material_alpha = 1.f;

    model_center._data[0] = 0.f;
    model_center._data[1] = 0.3;
    model_center._data[2] = -1.2f;

    sphere0_center.x = -0.1;
    sphere0_center.y = 0;
    sphere0_center.z = -1;
    sphere0_radius = 0.1;
    sphere1_center.x = 0;
    sphere1_center.y = 0.3;
    sphere1_center.z = -1.5;
    sphere1_radius = 0.2;
    sphere2_center.x = 0.4;
    sphere2_center.y = 0;
    sphere2_center.z = -1.7;
    sphere2_radius = 0.1;
    plane_point.x = plane_point.z = 0;
    plane_point.y = -0.3;
    plane_normal.x = plane_normal.z = 0;
    plane_normal.y = 1;
}

void glutTimer(int value)
{
    if (value != 0)
        return;

    static float theta = 0.f;
    pointlight1.position.x = cos(theta) * pointlight_radius;
    pointlight1.position.z = sin(theta) * pointlight_radius - 1;
    dirlight0.direction.x = sin(theta / 3) * pointlight_radius;
    dirlight0.direction.z = cos(theta / 3) * pointlight_radius;

    rotate_y += 2.3;
    rotate_x += 1.6;

    theta += 0.03;

    glutPostRedisplay();
    glutTimerFunc(9, glutTimer, 0);
}

void glut_glew_init()
{
    char fakeParam[] = "oh boy";
    char *fakeargv[] = { fakeParam, NULL };
    int fakeargc = 1;
    glutInit(&fakeargc, fakeargv);
    glutInitWindowPosition(200, 200);
    glutInitWindowSize(1024, 600);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutCreateWindow(fakeParam);
    glutDisplayFunc(renderscene);
    glutMouseFunc(glutMouseBotton);
    glutMotionFunc(glutMouseMove);
    glutKeyboardFunc(glutKeyboard);
    glutReshapeFunc(glutwindowresize);
    glutTimerFunc(1, glutTimer, 0);

    glEnable(GL_DEPTH_TEST);
//    glEnable(GL_CULL_FACE);
    glShadeModel(GL_SMOOTH);

    if (glewInit() != GLEW_OK)
        throw runtime_error("glewInit failed");

    // print out some info about the graphics drivers
    cout << "OpenGL version: " << glGetString(GL_VERSION) << endl;
    cout << "GLSL version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
    cout << "Vendor: " << glGetString(GL_VENDOR) << endl;
    cout << "Renderer: " << glGetString(GL_RENDERER) << endl;

    if (GLEW_ARB_vertex_shader && GLEW_ARB_fragment_shader)
        printf("Ready for GLSL\n");
    else
        printf("No GLSL support\n");
}

#ifdef __GNUC__
bool is_file_existed(string work_dir, string file_name)
{
    DIR *d = opendir(work_dir.c_str());
    if (d)
    {
        struct dirent *dir;
        while ((dir = readdir(d)) != NULL)
        {
            string filename = dir->d_name;
            if (filename.compare(file_name) == 0)
            {
                closedir(d);
                return true;
            }
        }
        closedir(d);
    }
    return false;
}
#endif

#ifdef _MSC_FULL_VER
typedef std::basic_string<TCHAR> tstring;
bool is_file_existed(string s_work_dir, string s_filename)
{
    tstring work_dir = s2ws(s_work_dir);
    tstring filename = s2ws(s_filename);

    WIN32_FIND_DATA FindFileData;
    HANDLE hFind;

    tstring fileregulator = work_dir + filename;
    hFind = FindFirstFile(fileregulator.c_str(), &FindFileData);
    if (hFind == INVALID_HANDLE_VALUE)
        return false;

    FindClose(hFind);
    return true;
}
#endif

#ifdef __GNUC__
int main(int argc, char *argv[])
#endif
#ifdef _MSC_FULL_VER
int _tmain(int argc, _TCHAR* argv[])
#endif
{
    glut_glew_init();
    // load the shaders
    string workdir = "./";
    string vs_file = "vertex_shader_raytrace.glsl";
    string fs_file = "fragment_shader_raytrace.glsl";
    while (!is_file_existed(workdir, vs_file))
    {
        workdir += "../";
        if (workdir.size() > 200)
            return 1;
    }

    shader_main = LoadShaders(file2string(workdir + vs_file), file2string(workdir + fs_file));
    setup_rc();
    init_scene();
    glutMainLoop();
    shader_main.UnloadAll();
    glBindVertexArray(0);
    glDeleteBuffers(1, &vbo_screen);
    glDeleteBuffers(1, &ibo_screen);
    return 0;
}
