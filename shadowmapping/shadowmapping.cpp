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

#include "../glcommon/glaux.h"
#include "../glcommon/loadshaders.h"
#include "../glcommon/load_m.h"
#include "../glcommon/loadbitmap.h"
using namespace glcommon;

Texture *mytexture = NULL;
GLuint texture_id = 0;

GLuint vbo, vao, ibo;
shader_handles shader_main;
// GLuint attrib_modelview_matrix = 0;
GLuint attrib_pointlight_matrix = 0;
GLuint attrib_perspective_matrix = 0;

vec3f material_rgb = vec3f(0.969f, 0.969f, 0.999f);

// 0, 1, 2 - coordinates;  3, 4, 5 - normal;   6, 7 - uv
vector<pos_normal_uv> vertex_data;

// each triple for a triangle
vector<unsigned int> vertex_indices;

GLuint shadowmap_fbo = 0;
GLuint shadowmap_id = 0;
const GLuint SHADOWMAP_WIDTH = 2048;
const GLuint SHADOWMAP_HEIGHT = 2048;

void load_texture(string texturefile)
{
    if (texture_id > 0)
        glDeleteTextures(1, &texture_id);
    if (mytexture)
        delete mytexture;
    texture_id = 0;
    mytexture = NULL;

    texturefile = trim(texturefile);
    if (texturefile.size() <= 0)
        return;

    string ext = LowerString(GetExtNamePart(texturefile));
    if (ext == "bmp")
        mytexture = new Bitmap(texturefile.c_str());
    else
        return;
    texture_id = gentexture(mytexture, GL_TEXTURE0);
}

void setup_rc(string texture_filename)
{
    load_texture(texture_filename);

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // ibo defines the index format
    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, vertex_indices.size()*sizeof(GLuint),
        &vertex_indices[0], GL_STATIC_DRAW);

    //----------------------------------------------------------------
    // vbo store the raw data
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertex_data.size()*sizeof(pos_normal_uv),
        &vertex_data[0], GL_STATIC_DRAW);

    // input parameter0 for the program - vertex positions
    shader_main.do_attrib("position", VERTEX_POS_SIZE, GL_FLOAT, GL_FALSE,
        VERTICES_STRIDE*sizeof(GL_FLOAT), (GLvoid*)(VERTEX_POS_OFFSET*sizeof(GL_FLOAT)));

    // input parameter1 for the program - vertex normals
    shader_main.do_attrib("normal", VERTEX_NORMAL_SIZE, GL_FLOAT, GL_FALSE,
        VERTICES_STRIDE*sizeof(GL_FLOAT), (GLvoid*)(VERTEX_NORMAL_OFFSET*sizeof(GL_FLOAT)));

    // input parameter2 for the program - vertex uv
    shader_main.do_attrib("uv", VERTEX_UV_SIZE, GL_FLOAT, GL_FALSE,
        VERTICES_STRIDE*sizeof(GL_FLOAT), (GLvoid*)(VERTEX_UV_OFFSET*sizeof(GL_FLOAT)));
    //----------------------------------------------------------------

    // unbind the VBO and VAO
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_VERTEX_ARRAY, 0);
    //----- notice the unbinding sequence: first vao, then ibo -----
    glBindVertexArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    //----- ----- ----- ----- ----- ----- ----- ----- ----- ----- -----
}

void draw_me(GLenum mode, GLsizei index_count, GLuint textureid)
{
    if (rendermode == RENDER_MODE_HYBIRD)
        glLineWidth(1.09f);
    else
        glLineWidth(0.3f);

    glPolygonMode(GL_FRONT_AND_BACK, mode);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureid);

    GLuint use_texture = glGetUniformLocation(shader_main.handler_program, "use_texture");
    glUniform1i(use_texture, 0);
    if (textureid > 0)
    {
        glUniform1i(use_texture, 1);
    }

    glDrawElements(GL_TRIANGLES, index_count, GL_UNSIGNED_INT, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

bool generate_shadowframe()
{
    glGenTextures(1, &shadowmap_id);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, shadowmap_id);

    // No need to force GL_DEPTH_COMPONENT24, drivers usually give you the max precision if available
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOWMAP_WIDTH, SHADOWMAP_HEIGHT, 0,
        GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

    // GL_LINEAR does not make sense for depth texture. However, next tutorial shows usage of GL_LINEAR and PCF
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);   // GL_NEAREST
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);   // GL_NEAREST

    // Remove artefact on the edges of the shadowmap
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);    // GL_CLAMP_TO_EDGE
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);    // GL_CLAMP_TO_EDGE

    // comparison mode of the shadow map
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);

    // create a framebuffer
    glGenFramebuffers(1, &shadowmap_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, shadowmap_fbo);

    // attach the texture to FBO depth attachment point
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowmap_id, 0);

    // Instruct openGL that we won't bind a color texture with the currently binded FBO
    glDrawBuffer(GL_NONE);
//    glReadBuffer(GL_NONE);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        cout << "GL_FRAMEBUFFER_COMPLETE failed, CANNOT use FBO" << endl;
        return false;
    }

    // switch back to window-system-provided framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return true;
}

static float fov_view_pointlight = 45.f;
void perspective_pointlight(matrix4f &m, GLuint w, GLuint h, GLfloat fov)
{
    m.loadidentity();
    GLfloat aspect = (GLfloat)w / (GLfloat)h;
    m4f_perspective(m, fov, aspect, 0.1f, 100000.0f);
}

void render_shadowmap()
{
    // activate offset for polygons
    glEnable(GL_POLYGON_OFFSET_FILL);
    // offset by two units equal to smallest value of change in the shadow map
    // and offset by two units depending on the slope of the polygon
    glPolygonOffset(2.0f, 2.0f);

    glBindFramebuffer(GL_FRAMEBUFFER, shadowmap_fbo);

    glViewport(0, 0, SHADOWMAP_WIDTH, SHADOWMAP_HEIGHT);
    glClearDepth(1.0);
    glClear(GL_DEPTH_BUFFER_BIT);

    matrix4f m;
    perspective_pointlight(m, SHADOWMAP_WIDTH, SHADOWMAP_HEIGHT, fov_view_pointlight);
    GLuint gluniform_id = glGetUniformLocation(shader_main.handler_program, "perspective_matrix");
    glUniformMatrix4fv(gluniform_id, 1, GL_FALSE, m.pointer());

    m.loadidentity();
    model_matrix(m);
    gluniform_id = glGetUniformLocation(shader_main.handler_program, "model_matrix");
    glUniformMatrix4fv(gluniform_id, 1, GL_FALSE, m.pointer());

    m.loadidentity();
    multipy_lookat(m, pointlight1.position.x, pointlight1.position.y, pointlight1.position.z,
        0, 0, 0, 0, 1, 0);
    gluniform_id = glGetUniformLocation(shader_main.handler_program, "camera_matrix");
    glUniformMatrix4fv(gluniform_id, 1, GL_FALSE, m.pointer());

    // render the mesh
    glBindVertexArray(vao);
    draw_me(GL_FILL, vertex_indices.size(), 0);
    // render the floor
    render_floor(shader_main);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glDisable(GL_POLYGON_OFFSET_FILL);

    // restore the viewport and perspective for window rendering
    int window_width = glutGet(GLUT_WINDOW_WIDTH);
    int window_height = glutGet(GLUT_WINDOW_HEIGHT);
    glViewport(0, 0, window_width, window_height);
    float aspect = (GLfloat)window_width / (GLfloat)window_height;
    m4f_perspective(perspective_matrix, fovy, aspect, 0.1f, 100000.0f);
    glUniformMatrix4fv(attrib_perspective_matrix, 1, GL_FALSE, perspective_matrix.pointer());
}

void renderscene()
{
    shader_main.use_me();
    render_shadowmap();

//    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    matrix4f pointlight_matrix;
    pointlight_matrix.loadidentity();
    multipy_lookat(pointlight_matrix, pointlight1.position.x, pointlight1.position.y, pointlight1.position.z,
        0, 0, 0, 0, 1, 0);
    glUniformMatrix4fv(attrib_pointlight_matrix, 1, GL_FALSE, pointlight_matrix.pointer());

    matrix4f lookat;
    lookat.loadidentity();
    lookat_matrix(lookat);
    GLuint gluniform_id = glGetUniformLocation(shader_main.handler_program, "camera_matrix");
    glUniformMatrix4fv(gluniform_id, 1, GL_FALSE, lookat.pointer());

    matrix4f modelmatrix;
    modelmatrix.loadidentity();
    model_matrix(modelmatrix);
    gluniform_id = glGetUniformLocation(shader_main.handler_program, "model_matrix");
    glUniformMatrix4fv(gluniform_id, 1, GL_FALSE, modelmatrix.pointer());

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
    vec4f position4f = vec3f_to_4f(dirlight0.direction, 1);
    position4f.transform_me(lookat);
    gluniform_id = glGetUniformLocation(shader_main.handler_program, "dirlight_direction");
    glUniform3fv(gluniform_id, 1, (GLfloat*)&position4f);

    // deal with the point light
    gluniform_id = glGetUniformLocation(shader_main.handler_program, "pointlight_ambient");
    glUniform3fv(gluniform_id, 1, (GLfloat*)&pointlight1.ambient);
    gluniform_id = glGetUniformLocation(shader_main.handler_program, "pointlight_diffuse");
    glUniform3fv(gluniform_id, 1, (GLfloat*)&pointlight1.diffuse);
    gluniform_id = glGetUniformLocation(shader_main.handler_program, "pointlight_specular");
    glUniform3fv(gluniform_id, 1, (GLfloat*)&pointlight1.specular);
    gluniform_id = glGetUniformLocation(shader_main.handler_program, "pointlight_attenuation");
    glUniform3fv(gluniform_id, 1, (GLfloat*)&pointlight1.attenuation);
    position4f = vec3f_to_4f(pointlight1.position, 1);
    position4f.transform_me(lookat);
    gluniform_id = glGetUniformLocation(shader_main.handler_program, "pointlight_position");
    glUniform3fv(gluniform_id, 1, (GLfloat*)&position4f);

    // draw this vao
    glBindVertexArray(vao);
    // draw surface or hybird
    draw_me(GL_FILL, vertex_indices.size(), texture_id);
    glBindVertexArray(0);

    // switch vao to draw the floor
    render_floor(shader_main);

    shader_main.do_not_use_me();
    print_control_hint();
    glutSwapBuffers();
}

void windowresize(GLsizei w, GLsizei h)
{
    // pass the perspective matrix to the shader
    shader_main.use_me();
    glUniformMatrix4fv(attrib_perspective_matrix, 1, GL_FALSE, perspective_matrix.pointer());
    shader_main.do_not_use_me();
}

void load_model(string modelfile)
{
    mesh_bound bound;
    load_m(modelfile, vertex_data, vertex_indices, bound);
    data_bound_x_max = bound.mesh_bound_x_max; data_bound_x_min = bound.mesh_bound_x_min;
    data_bound_y_max = bound.mesh_bound_y_max; data_bound_y_min = bound.mesh_bound_y_min;
    data_bound_z_max = bound.mesh_bound_z_max; data_bound_z_min = bound.mesh_bound_z_min;
    init_center();
    recenter_polygons(vertex_data, bound);
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

    if (work_dir.size() > 0 && work_dir[work_dir.size() - 1] != _T('/') && work_dir[work_dir.size() - 1] != _T('\\'))
        work_dir = work_dir + _T('/');

    WIN32_FIND_DATA FindFileData;
    HANDLE hFind;
    tstring fileregulator = work_dir + filename;
    hFind = FindFirstFile(fileregulator.c_str(), &FindFileData);
    if (hFind == INVALID_HANDLE_VALUE)
        return false;

    FindClose(hFind);
    return true;
}

tstring get_first_model(tstring model_dir, tstring model_ext)
{
    WIN32_FIND_DATA FindFileData;
    HANDLE hFind;

    tstring fileregulator = model_dir + _T("\\*.") + model_ext;
    hFind = FindFirstFile(fileregulator.c_str(), &FindFileData);
    if (hFind == INVALID_HANDLE_VALUE)
        return _T("");

    FindClose(hFind);
    return model_dir + _T("\\") + FindFileData.cFileName;
}
#endif

void sub_main(string modelfile, string texturefile)
{
    modelfile = trim(modelfile);
    texturefile = trim(texturefile);
    glut_glew_init(modelfile, renderscene, windowresize, NULL, NULL, NULL, NULL);


    string workdir = "./";
    string vs_file = "vertex_shader_shadowmap.glsl";
    string fs_file = "fragment_shader_shadowmap.glsl";
    while (!is_file_existed(workdir, vs_file))
    {
        workdir += "../";
        if (workdir.size() > 200)
            return;
    }

    shader_main = LoadShaders(file2string(workdir + vs_file), file2string(workdir + fs_file));

    attrib_perspective_matrix = glGetUniformLocation(shader_main.handler_program, "perspective_matrix");
    attrib_pointlight_matrix = glGetUniformLocation(shader_main.handler_program, "pointlight_matrix");
    GLuint uniform_color_id = glGetUniformLocation(shader_main.handler_program, "uniform_color");
    GLuint attrib_colortexture = glGetUniformLocation(shader_main.handler_program, "colortexture");
    GLuint attrib_shadowmap = glGetUniformLocation(shader_main.handler_program, "shadowmap");
    GLuint attrib_bias_matrix = glGetUniformLocation(shader_main.handler_program, "bias_matrix");

    shader_main.use_me();

    // perspective matrix for point light viewer in shadow mapping
    matrix4f m;
    perspective_pointlight(m, SHADOWMAP_WIDTH, SHADOWMAP_HEIGHT, fov_view_pointlight);
    GLuint gluniform_id = glGetUniformLocation(shader_main.handler_program, "perspective_pointlight");
    glUniformMatrix4fv(gluniform_id, 1, GL_FALSE, m.pointer());

    // do not use the uniform_color
    vec4f uniform_color = vec4f(0, 0, 0, 0);
    glUniform4fv(uniform_color_id, 1, (GLfloat*)&uniform_color);

    // use the texture unit0 for normal texture mapping
    glUniform1i(attrib_colortexture, 0);
    // use the texture unit1 for the shadowmapping
    glUniform1i(attrib_shadowmap, 1);

    matrix4f bias_matrix;
    bias_matrix.loadidentity();
    bias_matrix._data[0][0] = 0.5f;
    bias_matrix._data[1][1] = 0.5f;
    bias_matrix._data[2][2] = 0.5f;
    bias_matrix._data[3][3] = 1.f;
    bias_matrix._data[3][0] = 0.5f;
    bias_matrix._data[3][1] = 0.5f;
    bias_matrix._data[3][2] = 0.5f;

    glUniformMatrix4fv(attrib_bias_matrix, 1, GL_FALSE, bias_matrix.pointer());
    shader_main.do_not_use_me();

    load_model(modelfile);
    InitialAux initaux;
    initaux.pointlight1.ambient.r = 0.15;
    initaux.pointlight1.ambient.g = 0.15;
    initaux.pointlight1.ambient.b = 0.2;
    initaux.pointlight1.diffuse.r = 0.7;
    initaux.pointlight1.diffuse.g = 0.7;
    initaux.pointlight1.diffuse.b = 0.73;
    initaux.pointlight1.specular.r = 0.4;
    initaux.pointlight1.specular.g = 0.4;
    initaux.pointlight1.specular.b = 0.4;
    initaux.material_shininess = 0.9;
    initaux.pointlight1_sita = -29.f;
    reset_space(initaux, true);

    if (texturefile == "")
    {
        string workdir = GetDirectoryPart(modelfile);
        if (is_file_existed(workdir, RemoveDirectoryPart(RemoveExtNamePart(modelfile)) + ".bmp"))
            texturefile = RemoveExtNamePart(modelfile) + ".bmp";
    }

    if (!generate_shadowframe())
    {
        cout << "generate_shadowframe() not ok" << endl;
        return;
    }

    workdir = "./";
    while (!is_file_existed(workdir, "mao.bmp"))
    {
        workdir += "../";
        if (workdir.size() > 200)
            workdir = "";
    }

    string floor_texture = "";
    if (workdir != "")
    {
        floor_texture = workdir + "mao.bmp";
    }

    create_floor(shader_main, floor_texture);

    setup_rc(texturefile);

    rendermode = RENDER_MODE_SURFACE;
    glutMainLoop();

    shader_main.UnloadAll();
    glBindVertexArray(0);
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ibo);
    glDeleteBuffers(1, &vao);
    if (texture_id > 0)
        glDeleteTextures(1, &texture_id);
    if (mytexture)
        delete mytexture;

    release_floor();
}

#ifdef __GNUC__
int main(int argc, char *argv[])
{
    string texturefile = "";
    string modelfile = get_first_model(".", "m");
    if (modelfile.size() == 0)
        modelfile = get_first_model("..", "m");
    if (modelfile.size() == 0)
        modelfile = get_first_model("../..", "m");
    if (modelfile.size() == 0)
        modelfile = get_first_model("../../..", "m");

    if (argc > 1)
        modelfile = argv[1];
    if (argc > 2)
        texturefile = argv[2];

    if (modelfile.size() == 0)
    {
        cout << "give '.m' to me\n i'm worth it" << endl;
        fflush(stdout);
        return 1;
    }

    sub_main(modelfile, texturefile);
    cout << "love kitty" << endl;
    fflush(stdout);
    return 0;
}
#endif

#ifdef _MSC_FULL_VER
int _tmain(int argc, _TCHAR* argv[])
{
    tstring ttexturefile = _T("");
    tstring tmodelfile = get_first_model(_T("."), _T("m"));
    if (tmodelfile.size() == 0)
        tmodelfile = get_first_model(_T(".."), _T("m"));
    if (tmodelfile.size() == 0)
        tmodelfile = get_first_model(_T("../.."), _T("m"));
    if (tmodelfile.size() == 0)
        tmodelfile = get_first_model(_T("../../.."), _T("m"));

    if (argc > 1)
        tmodelfile = argv[1];
    if (argc > 2)
        ttexturefile = argv[2];

    if (tmodelfile.size() == 0)
    {
        cout << "give '.m' to me\n i'm worth it" << endl;
        fflush(stdout);
        return 1;
    }

    sub_main(ws2s(tmodelfile), ws2s(ttexturefile));

    cout << "love kitty" << endl;
    fflush(stdout);
    return 0;
}
#endif
