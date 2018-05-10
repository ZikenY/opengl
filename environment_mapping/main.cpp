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
#include "../glcommon/glaux_store.h"
#include "../glcommon/loadshaders.h"
#include "../glcommon/load_m.h"
#include "../glcommon/loadbitmap.h"
using namespace glcommon;

// describ the whole scene including view field, lights, model position.... everything
string scene_filename;
bool init_ok = false;
bool skybox_ok = false;

shader_handles shader_main;
GLuint attrib_perspective_matrix = 0;
GLuint attrib_texture_color = 0;

vec3f material_rgb = vec3f(0.96f, 0.94f, 0.97f);

struct vxo
{
    GLuint vao, vbo, ibo;
    Texture *texture;
    GLuint texture_id;
    vxo()
    {
        texture = NULL;
        texture_id = -1;
    }
};
vector<vxo> vxos;

// 0, 1, 2 - coordinates;  3, 4, 5 - normal;   6, 7 - uv
vector<vector<pos_normal_uv> > vertex_data_array;

// each triple for a triangle
vector<vector<unsigned int> > vertex_indices_array;

void load_texture(string texturefile, int modelindex)
{
    if (vxos[modelindex].texture)
    {
        glDeleteTextures(1, &vxos[modelindex].texture_id);
        delete vxos[modelindex].texture;
    }
    vxos[modelindex].texture_id = 0;
    vxos[modelindex].texture = NULL;

    texturefile = trim(texturefile);
    if (texturefile.size() <= 0)
        return;

    string ext = LowerString(GetExtNamePart(texturefile));
    if (ext == "bmp")
        vxos[modelindex].texture = new Bitmap(texturefile.c_str());
    else
        return;

    vxos[modelindex].texture_id = gentexture(vxos[modelindex].texture, GL_TEXTURE0);
}

void setup_rc(vector<pos_normal_uv> &vertex_data, vector<unsigned int> &vertex_indices, string texture_filename)
{
    vxo vxo0;
    vxos.push_back(vxo0);
    int vao_index = vxos.size()-1;
    load_texture(texture_filename, vao_index);

    glGenVertexArrays(1, &vxos[vao_index].vao);
    glBindVertexArray(vxos[vao_index].vao);

    // ibo defines the index format
    glGenBuffers(1, &vxos[vao_index].ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vxos[vao_index].ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, vertex_indices.size()*sizeof(GLuint),
        &vertex_indices[0], GL_STATIC_DRAW);

    //----------------------------------------------------------------
    // vbo store the raw data
    glGenBuffers(1, &vxos[vao_index].vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vxos[vao_index].vbo);
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

void draw_me(int vao_index, GLenum mode)
{
    glPolygonMode(GL_FRONT_AND_BACK, mode);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, vxos[vao_index].texture_id);

    GLuint use_texture = glGetUniformLocation(shader_main.handler_program, "use_texture");
    glUniform1i(use_texture, 0);
    if (vxos[vao_index].texture)
    {
        glUniform1i(use_texture, 1);
    }

    glDrawElements(GL_TRIANGLES, (GLsizei)vertex_indices_array[vao_index].size(), GL_UNSIGNED_INT, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void renderscene()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    shader_main.use_me();

    matrix4f lookat;
    lookat.loadidentity();
    lookat_matrix(lookat);
    GLuint gluniform_id = glGetUniformLocation(shader_main.handler_program, "camera_matrix");
    glUniformMatrix4fv(gluniform_id, 1, GL_FALSE, lookat.pointer());

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

    for (int i=0; i < vxos.size(); i++)
    {
        matrix4f modelmatrix;
        modelmatrix.loadidentity();
        model_matrix(modelmatrix);

        // transform this specific object
        model_matrix(modelmatrix, model_transforms[i]);

        gluniform_id = glGetUniformLocation(shader_main.handler_program, "model_matrix");
        glUniformMatrix4fv(gluniform_id, 1, GL_FALSE, modelmatrix.pointer());

        // draw this vao
        glBindVertexArray(vxos[i].vao);

        // draw surface or wired frame
        if (rendermode == RENDER_MODE_SURFACE)
            draw_me(i, GL_FILL);
        else
            draw_me(i, GL_LINE);

        // finished drawing this vao
        glBindVertexArray(0);
    }

    // draw floor
    render_floor(shader_main);
    shader_main.do_not_use_me();

    if (skybox_ok)
        render_skybox();
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

void load_models(string workdir, vector<transform_params> &transforms)
{
    if (workdir.size() > 0 && workdir[workdir.size() - 1] != '/' && workdir[workdir.size() - 1] != '\\')
        workdir = workdir + '/';

    int delta = model_transforms.size();

    for (int i = 0; i < transforms.size(); i++)
    {
        model_transforms.push_back(transforms[i]);

        vector<pos_normal_uv> vertex_data0;
        vertex_data_array.push_back(vertex_data0);
        vector<unsigned int> vertex_indices0;
        vertex_indices_array.push_back(vertex_indices0);
        mesh_bound bound;
        load_m(workdir + model_transforms[delta + i].model_name, vertex_data_array[delta + i], vertex_indices_array[delta + i], bound);
        recenter_polygons(vertex_data_array[delta + i], bound);

        if (delta == 0 && i == 0)
        {
            data_bound_x_max = bound.mesh_bound_x_max; data_bound_x_min = bound.mesh_bound_x_min;
            data_bound_y_max = bound.mesh_bound_y_max; data_bound_y_min = bound.mesh_bound_y_min;
            data_bound_z_max = bound.mesh_bound_z_max; data_bound_z_min = bound.mesh_bound_z_min;
        }
        else if (delta == 0)
        {
            data_bound_x_max = max(data_bound_x_max, bound.mesh_bound_x_max);
            data_bound_x_min = min(data_bound_x_min, bound.mesh_bound_x_min);
            data_bound_y_max = max(data_bound_y_max, bound.mesh_bound_y_max);
            data_bound_y_min = min(data_bound_y_min, bound.mesh_bound_y_min);
            data_bound_z_max = max(data_bound_z_max, bound.mesh_bound_z_max);
            data_bound_z_min = min(data_bound_z_min, bound.mesh_bound_z_min);
        }

        string texturefile = model_transforms[delta + i].texture_name;
        if (texturefile.size() > 0)
            texturefile = workdir + texturefile;
        setup_rc(vertex_data_array[delta + i], vertex_indices_array[delta + i], texturefile);
    }

    if (delta == 0)
        init_center();
}

// load_model(GetDirectoryPart(xx), RemoveDirectoryPart(xx), "xxx.bmp");
void load_model(string workdir, string modelname, string texturename)
{
    transform_params transform;
    transform.model_name = modelname;
    transform.texture_name = texturename;
    vector<transform_params> transforms;
    transforms.push_back(transform);
    load_models(workdir, transforms);
}

bool userkey(unsigned char key)
{
    switch (key)
    {
    case 13:    // enter
        {
            vector<pos_normal_uv> cube_data;
            vector<unsigned int> cube_indices;
            cube(cube_data, cube_indices);
            setup_rc(cube_data, cube_indices, "");

            vertex_data_array.push_back(cube_data);
            vertex_indices_array.push_back(cube_indices);

            transform_params transform;
            transform.model_name = "cube";
            model_transforms.push_back(transform);

            glutPostRedisplay();
        }
        break;

    default:
        return false;
    }

    return true;
}

void init_aux(string workdir, vector<transform_params> &transforms)
{
    load_models(workdir, transforms);

    InitialAux initaux;
    initaux.fovy = 39.f;
    initaux.target_phi = -79.0f;
    initaux.translate_y = max_r / 0.69f;
    initaux.pointlight1_radius = 60.f;
    reset_space(initaux, true);
    glLineWidth(0.3f);
    init_ok = true;
}

void loadscene()
{
    string workdir = GetDirectoryPart(scene_filename);
    vector<transform_params> transforms;
    string s;
    if (File2String(scene_filename, s))
    {
        Strings ss;
        ss.SetText(s);

        ss.GetLine(0, s);
        ss.DeleteLine(0);
        int count = StrToInt(s);
        for (int i = 0; i < count; i++)
        {
            // filename and matrix the object
            int delta = 0;
            if (init_ok)
            {
                load_transform_params(ss, delta, model_transforms[i]);
            }
            else
            {
                transform_params transform;
                transforms.push_back(transform);
                load_transform_params(ss, delta, transforms[i]);
            }
            for (int j = 0; j < delta; j++)
            {
                ss.DeleteLine(0);
            }
        }

        if (!init_ok)
        {
            init_aux(workdir, transforms);
        }

        s = ss.GetText();
        load_scene(s);
        glutPostRedisplay();
    }
    else
    {
        reset_space(initaux_saved, false);
    }
}

void storescene()
{
    // number of models
    string s = IntToStr(model_transforms.size()) + "\n";
    for (int i = 0; i < model_transforms.size(); i++)
    {
        // filename and matrix of .m
        Strings ss;
        store_transform_params(model_transforms[i], ss);
        s += ss.GetText() + '\n';
    }

    s += store_scene();
    String2File(s, scene_filename);

    loadscene(); // update the InitialAux
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

string get_first_ext(string work_dir, string file_ext)
{
    if (work_dir.size() > 0 && work_dir[work_dir.size() - 1] != '/' && work_dir[work_dir.size() - 1] != '\\')
        work_dir = work_dir + '/';

    DIR *d = opendir(work_dir.c_str());
    if (d)
    {
        struct dirent *dir;
        while ((dir = readdir(d)) != NULL)
        {
            string filename = dir->d_name;
            string ext = LowerString(GetExtNamePart(filename));
            if (ext.compare(file_ext) == 0)
            {
                closedir(d);
                return work_dir + filename;
            }
        }
        closedir(d);
    }
    return ""; 
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

string get_first_ext(string s_work_dir, string s_ext)
{
    tstring work_dir = s2ws(s_work_dir);
    tstring ext = s2ws(s_ext);

    if (work_dir.size() > 0 && work_dir[work_dir.size() - 1] != _T('/') && work_dir[work_dir.size() - 1] != _T('\\'))
        work_dir = work_dir + _T('/');

    WIN32_FIND_DATA FindFileData;
    HANDLE hFind;

    tstring fileregulator = work_dir + _T("*.") + ext;
    hFind = FindFirstFile(fileregulator.c_str(), &FindFileData);
    if (hFind == INVALID_HANDLE_VALUE)
        return "";

    FindClose(hFind);
    return ws2s(work_dir + FindFileData.cFileName);
}
#endif

#ifdef __GNUC__
int main(int argc, char *argv[])
{
    vector<string> filelist;
    for (int i = 1; i < argc; i++)
    {
        filelist.push_back((argv[i]));
    }
#endif
#ifdef _MSC_FULL_VER
int _tmain(int argc, _TCHAR* argv[])
{
    vector<string> filelist;
    for (int i = 1; i < argc; i++)
    {
        filelist.push_back(ws2s(argv[i]));
    }
#endif

    string workdir = "./";
    bool is_scene3d = false;
    if (filelist.size() == 0)
    {
        // find the first .scene3d
        string scene3d = get_first_ext(workdir, "scene3d");
        while (scene3d.size() == 0)
        {
            workdir += "../";
            if (workdir.size() > 200)
            {
                cout << "give me a .scene3d or .m please" << endl;
                return 1;
            }

            scene3d = get_first_ext(workdir, "scene3d");
        }

        filelist.push_back(RemoveDirectoryPart(scene3d));
        is_scene3d = true;
    }
    else
    {
        workdir = GetDirectoryPart(filelist[0]);
        if (workdir.size() > 0 && workdir[workdir.size() - 1] != '/' && workdir[workdir.size() - 1] != '\\')
            workdir = workdir + "/";

        for (int i = 0; i < filelist.size(); i++)
        {
            filelist[i] = RemoveDirectoryPart(filelist[i]);
        }

        string ext = GetExtNamePart(filelist[0]);
        if (ext == "scene3d")
        {
            is_scene3d = true;
        }
    }

    if (workdir == "")
    {
        workdir = "./";
    }

    glut_glew_init("oops", renderscene, windowresize, userkey, NULL, storescene, loadscene);
    shader_main = LoadShaders(vertex_shader_matrics, fragment_shader_phong);
    attrib_perspective_matrix = glGetUniformLocation(shader_main.handler_program, "perspective_matrix");
    attrib_texture_color = glGetUniformLocation(shader_main.handler_program, "texture_color");
    GLuint attrib_uniform_color_id = glGetUniformLocation(shader_main.handler_program, "uniform_color");

    shader_main.use_me();
    // use the texture unit #0
    glUniform1i(attrib_texture_color, 0);

    // do not use the uniform_color
    vec4f uniform_color = vec4f(0,0,0,0);
    glUniform4fv(attrib_uniform_color_id, 1, (GLfloat*)&uniform_color);
    shader_main.do_not_use_me();

    if (is_scene3d)
    {
        // load a .scene3d
        if (!is_file_existed(workdir, filelist[0]))
        {
            cout << "file no find: " << workdir + filelist[0] << endl;
            return 1;
        }

        scene_filename = workdir + filelist[0];
        loadscene();
    }
    else
    {
        // load bunch of .m files
        while (!is_file_existed(workdir, filelist[0]))
        {
            workdir += "../";
            if (workdir.size() > 200)
            {
                cout << "file no find: " << filelist[0] << endl;
                return 1;
            }
        }

        vector<transform_params> transforms;
        for (int i = 0; i < filelist.size(); i++)
        {
            transform_params transform;
            transform.model_name = filelist[i];
            transform.texture_name = "";
            if (is_file_existed(workdir, RemoveExtNamePart(transform.model_name) + ".bmp"))
                transform.texture_name = RemoveExtNamePart(transform.model_name) + ".bmp";
            transforms.push_back(transform);
        }

        init_aux(workdir, transforms);
        scene_filename = workdir + "what_is_in_your_mind.scene3d";
    }

    rendermode = RENDER_MODE_SURFACE;

    string floor_texture = "";
    if (is_file_existed(workdir, "starry.bmp"))
        floor_texture = workdir + "starry.bmp";
    create_floor(shader_main, floor_texture);

    if (is_file_existed(workdir, "negx.bmp"))
    {
        create_skybox(
            workdir + "negx.bmp",
            workdir + "negy.bmp",
            workdir + "negz.bmp",
            workdir + "posx.bmp",
            workdir + "posy.bmp",
            workdir + "posz.bmp"
            );
        skybox_ok = true;
    }

    glutMainLoop();
    shader_main.UnloadAll();
    glBindVertexArray(0);
    for (int i = 0; i<vxos.size(); i++)
    {
        glDeleteBuffers(1, &vxos[i].vbo);
        glDeleteBuffers(1, &vxos[i].ibo);
        glDeleteBuffers(1, &vxos[i].vao);

        if (vxos[i].texture)
        {
            glDeleteTextures(1, &vxos[i].texture_id);
            delete vxos[i].texture;
        }
    }

    release_floor();
    if (skybox_ok)
        release_skybox();
    return 0;
}
