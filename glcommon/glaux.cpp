#include "glaux.h"
#include "loadbitmap.h"
#include <iostream>
#include <stdexcept>
#include <string.h>
#include <math.h>

namespace glcommon
{

static GLfloat skybox_data[] = {
    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f, 1.0f,
    -1.0f, 1.0f, 1.0f,
    -1.0f, 1.0f, -1.0f,
    1.0f, 1.0f, -1.0f,
    1.0f, -1.0f, -1.0f,
    1.0f, 1.0f, 1.0f,
    1.0f, -1.0f, 1.0f
};

static GLuint skybox_incices[] = {0, 1, 2, 0, 2, 3, 4, 0, 3, 0, 4, 5, 4, 5, 6, 5, 6, 7,
    1, 2, 6, 1, 6, 7, 2, 3, 4, 2, 4, 6, 0, 1, 7, 0, 5, 7};
GLuint vao_skybox = 0;
GLuint vbo_skybox = 0;
GLuint ibo_skybox = 0;
GLuint textureid_skybox = 0;
shader_handles shader_skybox;
GLuint attrib_matrix_skybox = 0;
GLuint attrib_skyboxTex = 0;

GLfloat max_r;
int oldmx, oldmy;

matrix4f perspective_matrix;
// for each object's matrix
vector<transform_params> model_transforms;

// -1 means the matrix for the all model, not the element in model_transforms
int select_object = -1;

float data_bound_x_max;
float data_bound_x_min;
float data_bound_y_max;
float data_bound_y_min;
float data_bound_z_max;
float data_bound_z_min;

matrix4f trackball;

GLfloat fovy;
float target_phi, target_sita;    // control the camera target vector
GLfloat camera_pos_x, camera_pos_y, camera_pos_z;
GLfloat camera_up_x, camera_up_y, camera_up_z;
GLfloat rotate_x, rotate_y, rotate_z;
GLfloat translate_x, translate_y, translate_z;
GLfloat scale_x, scale_y, scale_z;
GLfloat material_shininess, material_alpha; // 0~1
GLfloat pointlight1_radius, pointlight1_phi, pointlight1_sita;
DirLight dirlight0;
PointLight pointlight1;

InitialAux initaux_saved;
bool rotate_dirlight0 = true;
bool scene_update, leftbotton, rightbotton, middlebotton;
int camera_move_longitute = 0;
int camera_move_lateral = 0;

// 0 - ambient; 1 - diffuse; 2 - specular; 3 - shininess
int current_tune_bar = 0;

int rendermode = RENDER_MODE_SKELETON;
void glutWindowResize(GLsizei w, GLsizei h);
void(*p_windowresize)(GLsizei, GLsizei) = NULL;
bool(*p_userkey)(unsigned char key) = NULL;
void(*p_timer)(int);
void(*p_storescene)() = NULL;
void(*p_loadscene)() = NULL;

// draw the lines for xz-plane
vector<vec3f> planexz;
GLuint vbo_planexz, vao_planexz;
shader_handles shader_planexz;
GLuint attrib_matrix_planexz = 0;
vec4f wireframe_color = vec4f(0.57, 0.53, 0.64, 1.0);

GLuint texture_floor_id = 0;
GLuint vao_floor, vbo_floor, ibo_floor;

#ifndef max
template <typename T>
inline T max(const T& a, const T& b)
{
    return a < b ? b : a;
}
#endif

#ifndef min
template <typename T>
inline T min(const T& a, const T& b)
{
    return a < b ? a : b;
}
#endif

transform_params::transform_params()
{
    this->reset();
}

void transform_params::reset()
{
    rotate_x = rotate_y = rotate_z = 0;
    translate_x = translate_y = translate_z = 0;
    scale_x = scale_y = scale_z = 1;
    trackball.loadidentity();
}

InitialAux::InitialAux()
{
    fovy = 44.f;
    target_phi = -90.0f;
    target_sita = 90.0f;
    camera_up_x = 0;
    camera_up_y = 1.0f;
    camera_up_z = 0;
    rotate_x = rotate_y = rotate_z = 0;
    translate_x = translate_y = translate_z = 0;
    scale_x = scale_y = scale_z = 1.0f;

    GLfloat light_position[] = {-1, -1, -1};
    GLfloat dirlight0_ambient[] = {0.1124f, 0.1124f, 0.1124f};
    GLfloat dirlight0_diffuse[] = {0.03135f, 0.03135f, 0.03135f};
    GLfloat dirlight0_specular[] = {0.0169f, 0.0169f, 0.0169f};
    updatev(dirlight0.ambient, dirlight0_ambient);
    updatev(dirlight0.diffuse, dirlight0_diffuse);
    updatev(dirlight0.specular, dirlight0_specular);
    updatev(dirlight0.direction, light_position);

    GLfloat pointlight1_ambient[] = {0.02f, 0.03f, 0.06f, 1.0f};
    GLfloat pointlight1_diffuse[] = {0.635f, 0.635f, 0.635f, 1.0f};
    GLfloat pointlight1_specular[] = {0.49f, 0.49f, 0.49f, 1.0f};
    updatev(pointlight1.ambient, pointlight1_ambient);
    updatev(pointlight1.diffuse, pointlight1_diffuse);
    updatev(pointlight1.specular, pointlight1_specular);
    updatev(pointlight1.position, light_position);
    updatev(pointlight1.attenuation, 0, 0.0019, 0.000269);
    material_shininess = 0.6f;
    material_alpha = 1.f;
    pointlight1_radius = -1;
    pointlight1_phi = 327.f;
    pointlight1_sita = -49.f;
    trackball.loadidentity();
}

void update_lights()
{
    /*
    GLfloat dirlight0_ambient[] = {0.f, 0.f, 0.f, 1.f};
    GLfloat dirlight0_diffuse[] = {0.f, 0.f, 0.f, 1.f};
    GLfloat dirlight0_specular[] = {0.f, 0.f, 0.f, 1.f};
    GLfloat dirlight0_direction[] = {0.f, 0.f, 0.f, 0.f};
    readv(dirlight0.ambient, dirlight0_ambient);
    readv(dirlight0.diffuse, dirlight0_diffuse);
    readv(dirlight0.specular, dirlight0_specular);
    readv(dirlight0.direction, dirlight0_direction);
    glLightfv(GL_LIGHT0, GL_AMBIENT, dirlight0_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, dirlight0_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, dirlight0_specular);
    glLightfv(GL_LIGHT0, GL_POSITION, dirlight0_direction);
    glEnable(GL_LIGHT0);
    */

    GLfloat pointlight1_ambient[] = {0.f, 0.f, 0.f, 1.f};
    GLfloat pointlight1_diffuse[] = {0.f, 0.f, 0.f, 1.f};
    GLfloat pointlight1_specular[] = {0.f, 0.f, 0.f, 1.f};
    GLfloat pointlight1_position[] = {0.f, 0.f, 0.f, 1.f};
    readv(pointlight1.ambient, pointlight1_ambient);
    readv(pointlight1.diffuse, pointlight1_diffuse);
    readv(pointlight1.specular, pointlight1_specular);
    readv(pointlight1.position, pointlight1_position);
    glLightfv(GL_LIGHT1, GL_AMBIENT, pointlight1_ambient);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, pointlight1_diffuse);
    glLightfv(GL_LIGHT1, GL_SPECULAR, pointlight1_specular);
    glLightfv(GL_LIGHT1, GL_POSITION, pointlight1_position);
    glEnable(GL_LIGHT1);

    glEnable(GL_LIGHTING);
}

void set_pointlight1(GLfloat _radius, GLfloat _phi, GLfloat _sita)
{
    GLfloat position[4];
    position[0] = _radius*sin(RAD*_phi)*cos(RAD*_sita);
    position[1] = _radius*cos(RAD*_phi);
    position[2] = _radius*sin(RAD*_phi)*sin(RAD*_sita);
    position[3] = 1.0f;
    updatev(pointlight1.position, position);
    pointlight1_radius = _radius;
    pointlight1_phi = _phi;
    pointlight1_sita = _sita;
}

void reset_space(InitialAux &initaux, bool reset_cam_pos)
{
    initaux_saved = initaux;
    trackball = initaux.trackball;
    fovy = initaux.fovy;
    target_phi = initaux.target_phi;
    target_sita = initaux.target_sita;
    camera_pos_x = initaux.camera_pos_x;
    camera_pos_y = initaux.camera_pos_y;
    camera_pos_z = initaux.camera_pos_z;
    camera_up_x = initaux.camera_up_x;
    camera_up_y = initaux.camera_up_y;
    camera_up_z = initaux.camera_up_z;
    rotate_x = initaux.rotate_x;
    rotate_y = initaux.rotate_y;
    rotate_z = initaux.rotate_z;
    translate_x = initaux.translate_x;
    translate_y = initaux.translate_y;
    translate_z = initaux.translate_z;
    scale_x = initaux.scale_x;
    scale_y = initaux.scale_y;
    scale_z = initaux.scale_z;
    dirlight0 = initaux.dirlight0;
    pointlight1 = initaux.pointlight1;
    update_lights();
    material_shininess = initaux.material_shininess;
    material_alpha = initaux.material_alpha;
    if (reset_cam_pos)
    {
        // initial camera position
        initaux.camera_pos_x = 2 * (2 * max_r)*sin(-RAD * 90)*cos(-RAD * 90);
        initaux.camera_pos_y = max_r * 1.7f + 2 * (2 * max_r)*cos(-RAD * 90);
        initaux.camera_pos_z = 2 * (2 * max_r)*sin(-RAD * 90)*sin(-RAD * 90);
        camera_pos_x = initaux.camera_pos_x;
        camera_pos_y = initaux.camera_pos_y;
        camera_pos_z = initaux.camera_pos_z;
        target_phi = initaux.target_phi = -99.0f;
        target_sita = initaux.target_sita = 90.0f;

        // move the objects up a little bit
        if (fabs(translate_y) < 0.00001)
            translate_y = initaux.translate_y = max_r / 0.89f;
    }

    if (initaux.pointlight1_radius < 0)
    {
        initaux.pointlight1_radius = max_r + 69.f;
        pointlight1_radius = initaux.pointlight1_radius;
    }
    else
    {
        pointlight1_radius = initaux.pointlight1_radius;
    }
    pointlight1_phi = initaux.pointlight1_phi;
    pointlight1_sita = initaux.pointlight1_sita;
    set_pointlight1(pointlight1_radius, pointlight1_phi, pointlight1_sita);

    if (initaux.dirlight0.direction[0] < 0)
    {
        GLfloat position[4];
        position[0] = (2*max_r)*sin(-RAD*69)*cos(-RAD*69);
        position[1] = (2*max_r)*cos(-RAD*99);
        position[2] = (2*max_r)*sin(-RAD*90)*sin(-RAD*90);
        position[3] = 1.0f;
        updatev(initaux.dirlight0.direction, position);
    }
    dirlight0.direction = initaux.dirlight0.direction;

    update_lights();

    int window_width = glutGet(GLUT_WINDOW_WIDTH);
    int window_height = glutGet(GLUT_WINDOW_HEIGHT);
    glutWindowResize(window_width, window_height);

    leftbotton = false;
    rightbotton = false;
    middlebotton = false;
    scene_update = true;
}

vec3f getTargetVector()
{
    vec3f v3f;
    v3f.x = 696969 * (2 * max_r)*sin(RAD*target_phi)*cos(RAD*target_sita);
    v3f.y = 696969 * (2 * max_r)*cos(RAD*target_phi);
    v3f.z = 696969 * (2 * max_r)*sin(RAD*target_phi)*sin(RAD*target_sita);
    return v3f;
}

vec3f getAzimuthVector()
{
    vec3f target = getTargetVector();
    vec3f a;
    a.x = target.x - camera_pos_x;
    a.y = target.y - camera_pos_y;
    a.z = target.z - camera_pos_z;
    return a;
}

vec4f getLateralVector()
{
    vec3f a = getAzimuthVector();
    vec4f v(a);
    matrix4f m;
    m.loadidentity();
    m4f_rotate(m, 90, camera_up_x, camera_up_y, camera_up_z);
    vec4f vv;
    m4f_multiply_v4f(vv, m, v);
    return vv;
}

void model_matrix(matrix4f &m)
{
    // apply translate
    multipy_translate(m, translate_x, translate_y, translate_z);

    // apply scale
    multipy_scale(m, scale_x, scale_y, scale_z);

    m.multiply(trackball);

    // apply rotation from keyboard
    multipy_rotate(m, rotate_x, 1.0f, 0.0f, 0.0f);
    multipy_rotate(m, rotate_y, 0.0f, 1.0f, 0.0f);
    multipy_rotate(m, rotate_z, 0.0f, 0.0f, 1.0f);
}

void model_matrix(matrix4f &m, vec3f &model_center)
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

    m.multiply(trackball);

    multipy_rotate(m, rotate_x, 1.0f, 0.0f, 0.0f);
    multipy_rotate(m, rotate_y, 0.0f, 1.0f, 0.0f);
    multipy_rotate(m, rotate_z, 0.0f, 0.0f, 1.0f);

    //step 1
    // ---------- move the model center to the center of world ----------
    multipy_translate(m, -model_center[0], -model_center[1], -model_center[2]);
}

void model_matrix(matrix4f &m, transform_params &transform)
{
    // apply translate
    multipy_translate(m, transform.translate_x, transform.translate_y, transform.translate_z);

    // apply scale
    multipy_scale(m, transform.scale_x, transform.scale_y, transform.scale_z);

    // apply rotation from trackball
    m.multiply(transform.trackball);

    // apply rotation along axises
    multipy_rotate(m, transform.rotate_x, 1.0f, 0.0f, 0.0f);
    multipy_rotate(m, transform.rotate_y, 0.0f, 1.0f, 0.0f);
    multipy_rotate(m, transform.rotate_z, 0.0f, 0.0f, 1.0f);
}

void lookat_matrix(matrix4f &m)
{
    /*
    // make sure the correct "up" vector to the camera
    //------------------------------------------------
    double _phi = (int)phi % 360;
    if (_phi > 0 && _phi <= 180)
        camera_up_y = -fabs(camera_up_y);
    else if (_phi > 180 && _phi <= 360)
        camera_up_y = fabs(camera_up_y);
    else if (_phi > -360 && _phi <= -180)
        camera_up_y = -fabs(camera_up_y);
    else
        camera_up_y = fabs(camera_up_y);
    //------------------------------------------------
    */

    // move camera at last. so first of all, it multipy by lookat()
    vec3f v3f = getTargetVector();
    multipy_lookat(m, camera_pos_x, camera_pos_y, camera_pos_z,
        v3f.x, v3f.y, v3f.z, camera_up_x, camera_up_y, camera_up_z);
}

vec3f NormalizeScreenPoint(int x, int y)
{
    int window_width = glutGet(GLUT_WINDOW_WIDTH);
    int window_height = glutGet(GLUT_WINDOW_HEIGHT);
    vec3f result;
    result.x = x / ((window_width - 1.0f) * 0.5f) - 1.0f;
    result.y = 1.0f - (y / ((window_height - 1.0f) * 0.5f));
    result.z = 0;
    return result;
}

bool isMouseWheel(int button, int state)
{
    // It's a wheel event
    if ((button == 3) || (button == 4))
    {
        // Each wheel event reports like a button click, GLUT_DOWN then GLUT_UP
        if (state == GLUT_UP)
            return true; // Disregard redundant GLUT_UP events

        if (!middlebotton)
        {
            if (select_object < 0)
            {
                if (button == 3)
                {
                    scale_x = scale_x - scale_x*0.03;
                    scale_z = scale_z - scale_z*0.03;
                    scale_y = scale_y - scale_y*0.03;
                }
                else
                {
                    scale_x = scale_x + scale_x*0.03;
                    scale_z = scale_z + scale_z*0.03;
                    scale_y = scale_y + scale_y*0.03;
                }
            }
            else
            {
                if (button == 3)
                {
                    model_transforms[select_object].scale_x = model_transforms[select_object].scale_x - model_transforms[select_object].scale_x*0.03;
                    model_transforms[select_object].scale_z = model_transforms[select_object].scale_z - model_transforms[select_object].scale_z*0.03;
                    model_transforms[select_object].scale_y = model_transforms[select_object].scale_y - model_transforms[select_object].scale_y*0.03;
                }
                else
                {
                    model_transforms[select_object].scale_x = model_transforms[select_object].scale_x + model_transforms[select_object].scale_x*0.03;
                    model_transforms[select_object].scale_z = model_transforms[select_object].scale_z + model_transforms[select_object].scale_z*0.03;
                    model_transforms[select_object].scale_y = model_transforms[select_object].scale_y + model_transforms[select_object].scale_y*0.03;
                }
            }
        }
        else
        {
            if (button == 3)
            {
                set_pointlight1(pointlight1_radius + 1.f, pointlight1_phi, pointlight1_sita);
            }else{
                set_pointlight1(pointlight1_radius - 1.f, pointlight1_phi, pointlight1_sita);
            }
            update_lights();
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
//        || button == GLUT_WHEEL_UP
//        || button == GLUT_WHEEL_DOWN)
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
        }else{
            rightbotton = false;
        }
    }

    if (button == GLUT_MIDDLE_BUTTON)
    {
        if (state == GLUT_DOWN){
            middlebotton = true;
            oldmx = x;
            oldmy = y;
        }else{
            middlebotton = false;
        }
    }
}

void trackball_me(int x, int y, matrix4f &m)
{
    vec3f old_pos = NormalizeScreenPoint(oldmx, oldmy);
    vec3f new_pos = NormalizeScreenPoint(x, y);
    vec3f from = screen2sphere(old_pos.x, old_pos.y);
    vec3f to = screen2sphere(new_pos.x, new_pos.y);
    vec3f n, d;
    vec3f_cross(n, to, from);
    vec3f_normalize(n);
    vec3f_substract(d, to, from);
    float theta = 180 * asin(sqrt(d.x*d.x + d.y*d.y + d.z*d.z));
    if (sqrtf(n.x*n.x + n.y*n.y + n.z*n.z) > 1e-5)
    {
        matrix4f q;
        quaternion(n, theta, q);
        m.multiply_from(q);
    }
}

void glutMouseMove(int x, int y)
{
    if (leftbotton)
    {
        if (select_object < 0)
            trackball_me(x, y, trackball);
        else
            trackball_me(x, y, model_transforms[select_object].trackball);
    }

    if (rightbotton)
    {
        target_sita += x - oldmx;
        if (fabs(target_phi + y - oldmy + 90.f) < 90.f)
            target_phi += y - oldmy;    }

    if (middlebotton)
    {
         set_pointlight1(pointlight1_radius, pointlight1_phi - (y - oldmy), pointlight1_sita - (x - oldmx));
         update_lights();
    }

    oldmx = x;
    oldmy = y;

    glutPostRedisplay();
}

void keyprocess_model_transform(unsigned char key)
{
    if (select_object < 0)
    {
        switch(key)
        {
        case 122:   // z
            translate_y += max_r / 10.0f;
            break;
        case 120:   // x
            translate_y -= max_r / 10.0f;
            break;
        case 99:  //c
            translate_x -= max_r / 10.0f;
            break;
        case 118:  //v
            translate_x += max_r / 10.0f;
            break;
        case 98:  //b
            translate_z += max_r / 10.0f;
            break;
        case 110:  //n
            translate_z -= max_r / 10.0f;
            break;
        case 91:   // [
            rotate_y += 10.0f;
            break;
        case 93:   // ]
            rotate_y -= 10.0f;
            break;
        case 59:   // ;
            rotate_z += 10.0f;
            break;
        case 39:   // '
            rotate_z -= 10.0f;
            break;
        case 44:   // <
            rotate_x += 10.0f;
            break;
        case 46:   // >
            rotate_x -= 10.0f;
            break;
        case 61:   // +
            //  (2*max_r) = (2*max_r) + max_r/5;
            scale_x = scale_x + scale_x*0.03;
            scale_z = scale_z + scale_z*0.03;
            scale_y = scale_y + scale_y*0.03;
            break;
        case 45:   // -
            scale_x = scale_x - scale_x*0.03;
            scale_z = scale_z - scale_z*0.03;
            scale_y = scale_y - scale_y*0.03;
            break;
        default:{}
        }
    }
    else
    {
        switch(key)
        {
        case 122:   // z
            model_transforms[select_object].translate_y += max_r / 10.0f;
            break;
        case 120:   // x
            model_transforms[select_object].translate_y -= max_r / 10.0f;
            break;
        case 99:  //c
            model_transforms[select_object].translate_x -= max_r / 10.0f;
            break;
        case 118:  //v
            model_transforms[select_object].translate_x += max_r / 10.0f;
            break;
        case 98:  //b
            model_transforms[select_object].translate_z += max_r / 10.0f;
            break;
        case 110:  //n
            model_transforms[select_object].translate_z -= max_r / 10.0f;
            break;
        case 91:   // [
            model_transforms[select_object].rotate_y += 10.0f;
            break;
        case 93:   // ]
            model_transforms[select_object].rotate_y -= 10.0f;
            break;
        case 59:   // ;
            model_transforms[select_object].rotate_z += 10.0f;
            break;
        case 39:   // '
            model_transforms[select_object].rotate_z -= 10.0f;
            break;
        case 44:   // <
            model_transforms[select_object].rotate_x += 10.0f;
            break;
        case 46:   // >
            model_transforms[select_object].rotate_x -= 10.0f;
            break;
        case 61:   // +
            model_transforms[select_object].scale_x = model_transforms[select_object].scale_x + model_transforms[select_object].scale_x*0.03;
            model_transforms[select_object].scale_z = model_transforms[select_object].scale_z + model_transforms[select_object].scale_z*0.03;
            model_transforms[select_object].scale_y = model_transforms[select_object].scale_y + model_transforms[select_object].scale_y*0.03;
            break;
        case 45:   // -
            model_transforms[select_object].scale_x = model_transforms[select_object].scale_x - model_transforms[select_object].scale_x*0.03;
            model_transforms[select_object].scale_z = model_transforms[select_object].scale_z - model_transforms[select_object].scale_z*0.03;
            model_transforms[select_object].scale_y = model_transforms[select_object].scale_y - model_transforms[select_object].scale_y*0.03;
            break;
        default:{}
        }
    }
}

void glutKeyboard(unsigned char key, int x, int y)
{
    if (p_userkey)
    {
        bool result = p_userkey(key);
        if (result)
            return;
    }

    scene_update = true;
    int window_width = glutGet(GLUT_WINDOW_WIDTH);
    int window_height = glutGet(GLUT_WINDOW_HEIGHT);
    switch(key)
    {
    case 27:
#ifdef __GNUC__
        glutLeaveMainLoop();
#endif
        break;

    case 8:     // backspace
        reset_space(initaux_saved, true);
        break;

    case 127:   // delete
        set_pointlight1(pointlight1_radius - max_r/5.f, pointlight1_phi, pointlight1_sita);
        update_lights();
        break;

    case 9:     // tab
        if (select_object < int(model_transforms.size()-1))
            select_object++;
        else
            select_object = -1;
        break;

    case 32:    // space
        if (rendermode == RENDER_MODE_SKELETON)
            rendermode = RENDER_MODE_SURFACE;
        else if (rendermode == RENDER_MODE_SURFACE)
            rendermode = RENDER_MODE_HYBIRD;
        else
            rendermode = RENDER_MODE_SKELETON;
        break;

    case 48:    // 0
        break;

    case 112:   // p
        rotate_dirlight0 = !rotate_dirlight0;
        break;

    case 119:   // w
        camera_move_longitute = 100;
        break;

    case 115:   // s
        camera_move_longitute = -100;
        break;

    case 97:   // a
        camera_move_lateral = 100;
        break;

    case 100:   // d
        camera_move_lateral = -100;
        break;

    case 114:   // r
        camera_pos_y += max_r / 19.f;
        break;

    case 102:   // f
        camera_pos_y -= max_r / 19.f;
        break;

    case 113:   // q
        if (camera_up_x < 1.f)
        {
            camera_up_x += 0.1;
            camera_up_y = 1 - fabs(camera_up_x);
        }
        break;

    case 101:   // e
        if (camera_up_x > -1.f)
        {
            camera_up_x -= 0.1;
            camera_up_y = 1 - fabs(camera_up_x);
        }
        break;

    case 122:   // z
    case 120:   // x
    case 99:    // c
    case 118:   // v
    case 98:    // b
    case 110:   // n
    case 91:    // [
    case 93:    // ]
    case 59:    // ;
    case 39:    // '
    case 44:    // <
    case 46:    // >
    case 61:   // +
    case 45:   // -
        keyprocess_model_transform(key);
        break;

    default:
        {}
    }
    CameraPositionUpdate();
    glutPostRedisplay();
}

void tune_light(int up_down, int left_right)
{
    if (current_tune_bar == 0 && up_down < 0)
        current_tune_bar = 4;
    else if (current_tune_bar == 4 && up_down > 0)
        current_tune_bar = 0;
    else if (up_down < 0)
        current_tune_bar--;
    else if (up_down > 0)
        current_tune_bar++;

    float delta = 0.f;
    if (left_right < 0)
        delta = -0.1;
    else if (left_right > 0)
        delta = 0.1;
    float ambient = min(1.f, (pointlight1.ambient.r + pointlight1.ambient.g + pointlight1.ambient.b)/3.f);
    float diffuse = min(1.f, (pointlight1.diffuse.r + pointlight1.diffuse.g + pointlight1.diffuse.b)/3.f);
    float specular = min(1.f, (pointlight1.specular.r + pointlight1.specular.g + pointlight1.specular.b)/3.f);
    float shininess = min(1.f, max(0.f, (material_shininess + delta)));
    float alpha = min(1.f, max(0.f, (material_alpha + delta)));
    ambient = min(1.f, max(0.f, (ambient + delta)));
    diffuse = min(1.f, max(0.f, (diffuse + delta)));
    specular = min(1.f, max(0.f, (specular + delta)));

    if (left_right != 0)
    {
        switch(current_tune_bar) // 0 - ambient; 1 - diffuse; 2 - specular; 3 - shininess
        {
        case 0:
            pointlight1.ambient.r = pointlight1.ambient.g = pointlight1.ambient.b = ambient;
            break;
        case 1:
            pointlight1.diffuse.r = pointlight1.diffuse.g = pointlight1.diffuse.b = diffuse;
            break;
        case 2:
            pointlight1.specular.r = pointlight1.specular.g = pointlight1.specular.b = specular;
            break;
        case 3:
            material_shininess = shininess;
            break;
        case 4:
            material_alpha = alpha;
            break;
        default:
            {}
        }
        update_lights();
    }
}

void glutSpecialKey(int key, int x, int y)
{
    int window_width = glutGet(GLUT_WINDOW_WIDTH);
    int window_height = glutGet(GLUT_WINDOW_HEIGHT);
    switch (key)
    {
    case GLUT_KEY_LEFT:
        tune_light(0, -1);
        break;
    case GLUT_KEY_UP:
        tune_light(-1, 0);
        break;
    case GLUT_KEY_RIGHT:
        tune_light(0, 1);
        break;
    case GLUT_KEY_DOWN:
        tune_light(1, 0);
        break;
    case GLUT_KEY_PAGE_UP:
         set_pointlight1(pointlight1_radius, pointlight1_phi+5, pointlight1_sita);
         update_lights();
        break;
    case GLUT_KEY_PAGE_DOWN:
        set_pointlight1(pointlight1_radius, pointlight1_phi-5, pointlight1_sita);
        update_lights();
        break;
    case GLUT_KEY_HOME:
        set_pointlight1(pointlight1_radius, pointlight1_phi, pointlight1_sita+5);
        update_lights();
        break;
    case GLUT_KEY_END:
        set_pointlight1(pointlight1_radius, pointlight1_phi, pointlight1_sita-5);
        update_lights();
        break;
    case GLUT_KEY_INSERT:
        set_pointlight1(pointlight1_radius + max_r/5.f, pointlight1_phi, pointlight1_sita);
        update_lights();
        break;
    case GLUT_KEY_F1:
        fovy -= 5.f;
        glutWindowResize(window_width, window_height);
        break;
    case GLUT_KEY_F2:
        fovy += 5.f;
        glutWindowResize(window_width, window_height);
        break;
    case GLUT_KEY_F5:
        if (p_storescene)
            p_storescene();
        break;
    case GLUT_KEY_F6:
        if (p_loadscene)
            p_loadscene();
        break;

    default:
        {}
    }
    glutPostRedisplay();
}

void glutWindowResize(GLsizei w, GLsizei h)
{
    glViewport(0, 0, w, h);
    GLfloat aspect = (GLfloat)w / (GLfloat)h;
    m4f_perspective(perspective_matrix, fovy, aspect, 0.1f, 100000.0f);

    // only for the non-shader version
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glLoadMatrixf(&perspective_matrix._data[0][0]);

    if (p_windowresize)
    {
        p_windowresize(w, h);
    }
}

void CameraPositionUpdate()
{
    vec3f a = getAzimuthVector();
    vec4f vv = getLateralVector();

    if (camera_move_longitute > 0)
    {
        camera_pos_x += a.x / (6969*3000);
        camera_pos_z += a.z / (6969*3000);
    }
    else if (camera_move_longitute < 0)
    {
        camera_pos_x -= a.x / (6969*3000);
        camera_pos_z -= a.z / (6969*3000);
    }

    if (camera_move_lateral > 0)
    {
        camera_pos_x += vv._data[0] / (6969*3000);
        camera_pos_z += vv._data[2] / (6969*3000);
    }
    else if (camera_move_lateral < 0)
    {
        camera_pos_x -= vv._data[0] / (6969*3000);
        camera_pos_z -= vv._data[2] / (6969*3000);
    }

    camera_move_longitute = camera_move_lateral = 0.f;
}

void glutTimer(int value)
{
    if (value != 0)
        return;

    if (rotate_dirlight0)
    {
        // compute the coordinates of the rotating light
        static GLfloat light2_phi = 0;
        static GLfloat light2_sita = 0;
        light2_phi += 1.9f;
        light2_sita += 1.0f;

        // compute the cartesian coordinates of light position based on radius and the 2 angles
        GLfloat position[4];
        position[0] = 3*(2*max_r)*sin(RAD*light2_phi)*cos(RAD*light2_sita);
        position[1] = 3*(2*max_r)*cos(RAD*light2_phi);
        position[2] = 3*(2*max_r)*sin(RAD*light2_phi)*sin(RAD*light2_sita);
        position[3] = 1.0f;
        updatev(dirlight0.direction, position);
        // updatev(pointlight1.position, position);
        update_lights();
    }

    if (p_timer)
        p_timer(value);

    glutPostRedisplay();
    glutTimerFunc(9, glutTimer, 0);
}

void glut_glew_init(string title, void (*renderscene)(void), void (*resize)(int, int),
    bool(*userkey)(unsigned char key), void(*timer)(int), void(*storescene)(), void(*loadscene)())
{
    p_windowresize = resize;
    p_userkey = userkey;
    p_timer = timer;
    p_storescene = storescene;
    p_loadscene = loadscene;

    char fakeParam[] = "oh boy";
    char *fakeargv[] = { fakeParam, NULL };
    int fakeargc = 1;
    glutInit(&fakeargc, fakeargv);
    glutInitWindowPosition(100, 100);
    glutInitWindowSize(1024, 600);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutCreateWindow(title.c_str());
    glutDisplayFunc(renderscene);
    glutReshapeFunc(glutWindowResize);
    glutMouseFunc(glutMouseBotton);
    glutMotionFunc(glutMouseMove);
    glutKeyboardFunc(glutKeyboard);
    glutSpecialFunc(glutSpecialKey);
    glutTimerFunc(1, glutTimer, 0);

    trackball.loadidentity();

    glDisable(GL_CULL_FACE);
    glClearColor(.0f, .0f, .0f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glShadeModel(GL_SMOOTH);
//    glEnable(GL_LINE_SMOOTH);
//    glHint(GL_LINE_SMOOTH_HINT, GL_FASTEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);

    if (glewInit() != GLEW_OK)
        throw runtime_error("glewInit failed");

    // print out some info about the graphics drivers
    cout << "OpenGL version: " << glGetString(GL_VERSION) << endl;
    cout << "GLSL version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
    cout << "Vendor: " << glGetString(GL_VENDOR) << endl;
    cout << "Renderer: " << glGetString(GL_RENDERER) << endl;

    if (!GLEW_VERSION_3_3)
        cout << "OpenGL 3.3 API is not available." << endl;
//        throw std::runtime_error("OpenGL 3.3 API is not available.");

    if(GLEW_ARB_vertex_shader && GLEW_ARB_fragment_shader)
        printf("Ready for GLSL\n");
    else
        printf("No GLSL support\n");
}

void init_center()
{
    float radius = fabs(data_bound_x_max);
    if (radius < fabs(data_bound_x_min))
        radius = fabs(data_bound_x_min);

    if (radius < fabs(data_bound_y_max))
        radius = fabs(data_bound_y_max);

    if (radius < fabs(data_bound_y_min))
        radius = fabs(data_bound_y_min);

    if (radius < fabs(data_bound_z_max))
        radius = fabs(data_bound_z_max);

    if (radius < fabs(data_bound_x_min))
        radius = fabs(data_bound_z_min);

    radius *= (float)1.7f;
    max_r = radius / 2;
}

void recenter_polygons(vector<pos_normal_uv> &vertex_data, mesh_bound &bound)
{
    vec3f model_center;
    model_center.x = (bound.mesh_bound_x_max + bound.mesh_bound_x_min) / 2.0f;
    model_center.y = (bound.mesh_bound_y_max + bound.mesh_bound_y_min) / 2.0f;
    model_center.z = (bound.mesh_bound_z_max + bound.mesh_bound_z_min) / 2.0f;

    vector<pos_normal_uv>::iterator it_pos_normal = vertex_data.begin();
    while (it_pos_normal != vertex_data.end())
    {
        (*it_pos_normal).pos[0] = (*it_pos_normal).pos[0] - model_center.x;
        (*it_pos_normal).pos[1] = (*it_pos_normal).pos[1] - model_center.y;
        (*it_pos_normal).pos[2] = (*it_pos_normal).pos[2] - model_center.z;
        it_pos_normal++;
    }
}

void recenter_polygons(vector<vector<float> > &_polygons)
{
    vec3f model_center;
    model_center.x = (data_bound_x_max + data_bound_x_min) / 2.0f;
    model_center.y = (data_bound_y_max + data_bound_y_min) / 2.0f;
    model_center.z = (data_bound_z_max + data_bound_z_min) / 2.0f;

    vector<vector<float> >::iterator it_polygons = _polygons.begin();
    while (it_polygons != _polygons.end())
    {
        vector<float> coords = *it_polygons;
        vector<float>::iterator it_coords = coords.begin();
        while (it_coords != coords.end())
        {
            *it_coords = *it_coords - model_center.x;
            *(it_coords+1) = *(it_coords+1) - model_center.y;
            *(it_coords+2) = *(it_coords+2) - model_center.z;
            it_coords += 3;
        }

        *it_polygons = coords;
        it_polygons++;
    }

    data_bound_x_min = data_bound_x_min - model_center.x;
    data_bound_y_min = data_bound_y_min - model_center.y;
    data_bound_z_min = data_bound_z_min - model_center.z;
}

void create_planexz()
{
    planexz.clear();
    float center_x = (data_bound_x_max + data_bound_x_min) / 2.f;
    float center_z = (data_bound_z_max + data_bound_z_min) / 2.f;
    float base_length = (data_bound_x_max - data_bound_x_min);
    float plane_length = base_length * 35.f;
    float min_x = center_x - plane_length / 2.f;
    float max_x = center_x + plane_length / 2.f;
    float min_z = center_z - plane_length / 2.f;
    float max_z = center_z + plane_length / 2.f;
    int line_count = 26;
    float cell_length = plane_length / line_count;
    for (int i=0; i<line_count+1; i++)
    {
        vec3f v0, v1;
        v0.x = min_x;
        v0.z = min_z + cell_length*i;
        v0.y = 0;
        v1.x = max_x;
        v1.z = min_z + cell_length*i;
        v1.y = 0;
        planexz.push_back(v0);
        planexz.push_back(v1);
        planexz.push_back(v0);
    }

    vec3f v0;
    v0.x = min_x;
    v0.z = min_z;
    v0.y = 0;
    planexz.push_back(v0);
    for (int i=0; i<line_count+1; i++)
    {
        vec3f v0, v1;
        v0.x = min_x + cell_length*i;
        v0.z = min_z;
        v0.y = 0;
        v1.x = min_x + cell_length*i;
        v1.z = max_z;
        v1.y = 0;
        planexz.push_back(v0);
        planexz.push_back(v1);
        planexz.push_back(v0);
    }
}

void draw_planexz(GLfloat r, GLfloat g, GLfloat b)
{
    GLboolean color_material;
    glGetBooleanv(GL_COLOR_MATERIAL, &color_material);
    glEnable(GL_COLOR_MATERIAL);
    glColor3f(r, g, b);

    glEnableClientState(GL_VERTEX_ARRAY);
    matrix4f lookatview;
    lookatview.loadidentity();
    lookat_matrix(lookatview);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadMatrixf(lookatview.pointer());
    glVertexPointer(3, GL_FLOAT, 0, &planexz[0]);
    glDrawArrays(GL_LINE_STRIP, 0, planexz.size());
    glPopMatrix();
    glDisableClientState(GL_VERTEX_ARRAY);

    if (!color_material)
        glDisable(GL_COLOR_MATERIAL);
}

void initshader_planexz()
{
    shader_planexz = LoadShaders(simple_vertex_shader, simple_fragment_shader);
    attrib_matrix_planexz = glGetUniformLocation(shader_planexz.handler_program, "matrix");

    glGenVertexArrays(1, &vao_planexz);
    glBindVertexArray(vao_planexz);

    glGenBuffers(1, &vbo_planexz);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_planexz);
    glBufferData(GL_ARRAY_BUFFER, planexz.size()*sizeof(vec3f), &planexz[0], GL_STATIC_DRAW);
    shader_planexz.do_attrib("position", 3, GL_FLOAT, GL_FALSE, 3*sizeof(GL_FLOAT), 0);
                                                                                // (GLvoid*)(0*sizeof(GL_FLOAT))
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void shade_planexz()
{
    shader_planexz.use_me();
    glBindVertexArray(vao_planexz);

    matrix4f matrix;
    matrix.loadidentity();
    lookat_matrix(matrix);
    matrix.multiply_from(perspective_matrix);
    glUniformMatrix4fv(attrib_matrix_planexz, 1, GL_FALSE, matrix.pointer());

    GLuint wireframe_color_id = glGetUniformLocation(shader_planexz.handler_program, "wireframe_color");
    glUniform4fv(wireframe_color_id, 1, (GLfloat*)&wireframe_color);

    glDrawArrays(GL_LINE_STRIP, 0, planexz.size());

    glBindVertexArray(0);
    shader_planexz.do_not_use_me();
}

void releaseshader_planexz()
{
    glDeleteBuffers(1, &vbo_planexz);
    glDeleteBuffers(1, &vao_planexz);
    shader_planexz.UnloadAll();
}

Texture* create_texture_warpper(string filename)
{
    Texture *texture = NULL;
    if (filename.length() > 0)
    {
        string ext = filename.substr(filename.length() - 4, 4);
        if (ext == ".bmp")
            texture = new Bitmap(filename.c_str());
        else if (ext == ".tga")
            texture = new Tga(filename.c_str());
    }

    return texture;
}

GLuint create_texture(string filename, GLenum activetexture)
{
    Texture *texture = create_texture_warpper(filename);
    GLuint texture_id = 0;
    if (texture)
    {
        texture_id = gentexture(texture, activetexture);
        delete texture;
    }
    return texture_id;
}

void render_floor(shader_handles &shader_main)
{
    glBindVertexArray(vao_floor);
    matrix4f modelmatrix;
    modelmatrix.loadidentity();     // you just restored the modelmatrix to identity
    GLuint gluniform_id = glGetUniformLocation(shader_main.handler_program, "model_matrix");
    glUniformMatrix4fv(gluniform_id, 1, GL_FALSE, modelmatrix.pointer());

    GLuint use_texture = glGetUniformLocation(shader_main.handler_program, "use_texture");
    glUniform1i(use_texture, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    if (texture_floor_id > 0)
    {
        glUniform1i(use_texture, 1);
        GLuint attrib_texture_color = glGetUniformLocation(shader_main.handler_program, "texture_color");
        glUniform1i(attrib_texture_color, 0);
        glBindTexture(GL_TEXTURE_2D, texture_floor_id);
    }
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindVertexArray(0);
}

void create_floor(shader_handles &shader_main, string floor_texturefile)
{
    texture_floor_id = create_texture(floor_texturefile, GL_TEXTURE0);

    float center_x = (data_bound_x_max + data_bound_x_min) / 2.f;
    float center_z = (data_bound_z_max + data_bound_z_min) / 2.f;
    float base_length = (data_bound_x_max - data_bound_x_min);
    float plane_length = base_length * 3.76f;
    float min_x = center_x - plane_length / 2.f;
    float max_x = center_x + plane_length / 2.f;
    float min_z = center_z - plane_length / 2.f;
    float max_z = center_z + plane_length / 2.f;

    static GLfloat floor_data[] =
    {
        0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 1.f, 0.f, // 0
        0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 1.f, 1.f, // 1
        0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 1.f, // 2
        0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f  // 3
    };

    static GLuint floor_indices[] = { 0, 1, 2, 0, 2, 3 };

    floor_data[0] = min_x;
    floor_data[2] = min_z;
    floor_data[VERTICES_STRIDE + 0] = min_x;
    floor_data[VERTICES_STRIDE + 2] = max_z;
    floor_data[2 * VERTICES_STRIDE + 0] = max_x;
    floor_data[2 * VERTICES_STRIDE + 2] = max_z;
    floor_data[3 * VERTICES_STRIDE + 0] = max_x;
    floor_data[3 * VERTICES_STRIDE + 2] = min_z;

    glGenVertexArrays(1, &vao_floor);
    glBindVertexArray(vao_floor);

    // ibo defines the index format
    glGenBuffers(1, &ibo_floor);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_floor);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6*sizeof(GLuint), floor_indices, GL_STATIC_DRAW);

    // vbo store the raw data
    glGenBuffers(1, &vbo_floor);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_floor);
    glBufferData(GL_ARRAY_BUFFER, 4*sizeof(pos_normal_uv), floor_data, GL_STATIC_DRAW);

    // input parameter0 for the program - vertex positions
    shader_main.do_attrib("position", VERTEX_POS_SIZE, GL_FLOAT, GL_FALSE,
        VERTICES_STRIDE*sizeof(GL_FLOAT), (GLvoid*)(VERTEX_POS_OFFSET*sizeof(GL_FLOAT)));

    // input parameter1 for the program - vertex normals
    shader_main.do_attrib("normal", VERTEX_NORMAL_SIZE, GL_FLOAT, GL_FALSE,
        VERTICES_STRIDE*sizeof(GL_FLOAT), (GLvoid*)(VERTEX_NORMAL_OFFSET*sizeof(GL_FLOAT)));

    // input parameter2 for the program - vertex uv
    shader_main.do_attrib("uv", VERTEX_UV_SIZE, GL_FLOAT, GL_FALSE,
        VERTICES_STRIDE*sizeof(GL_FLOAT), (GLvoid*)(VERTEX_UV_OFFSET*sizeof(GL_FLOAT)));

    // unbind the VBO and VAO
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_VERTEX_ARRAY, 0);
    //----- notice the unbinding sequence: first vao, then ibo -----
    glBindVertexArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    //----- ----- ----- ----- ----- ----- ----- ----- ----- ----- -----
}

void release_floor()
{
    glDeleteBuffers(1, &vbo_floor);
    glDeleteBuffers(1, &ibo_floor);
    glDeleteBuffers(1, &vao_floor);
    if (texture_floor_id > 0)
        glDeleteTextures(1, &texture_floor_id);
}


void render_skybox()
{
    GLint OldCullFaceMode, OldDepthFuncMode;
    glGetIntegerv(GL_CULL_FACE_MODE, &OldCullFaceMode);
    glGetIntegerv(GL_DEPTH_FUNC, &OldDepthFuncMode);
    glCullFace(GL_FRONT);
    glDepthFunc(GL_LEQUAL);

    shader_skybox.use_me();
    glBindVertexArray(vao_skybox);

    matrix4f matrix;
    matrix.loadidentity();
    lookat_matrix(matrix);
    multipy_translate(matrix, camera_pos_x, camera_pos_y, camera_pos_z);
    matrix.multiply_from(perspective_matrix);
    glUniformMatrix4fv(attrib_matrix_skybox, 1, GL_FALSE, matrix.pointer());
    glUniform1i(attrib_skyboxTex, 0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureid_skybox);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

    glBindVertexArray(0);
    shader_skybox.do_not_use_me();

    glCullFace(OldCullFaceMode);
    glDepthFunc(OldDepthFuncMode);
}

void create_skybox(string negx, string negy, string negz, string posx, string posy, string posz)
{
    shader_skybox = LoadShaders(vertex_shader_skybox, fragment_shader_skybox);
    attrib_matrix_skybox = glGetUniformLocation(shader_skybox.handler_program, "matrix");
    attrib_skyboxTex = glGetUniformLocation(shader_skybox.handler_program, "skyboxTex");

    Texture* warpper_posx = create_texture_warpper(negx);   // posx?
    Texture* warpper_negx = create_texture_warpper(posx);   // negx?
    Texture* warpper_posz = create_texture_warpper(posz);
    Texture* warpper_negz = create_texture_warpper(negz);
    Texture* warpper_posy = create_texture_warpper(posy);
    Texture* warpper_negy = create_texture_warpper(negy);

    glGenTextures(1, &textureid_skybox);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureid_skybox);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGB, warpper_posx->width, warpper_posx->height, 0, GL_RGB, GL_UNSIGNED_BYTE, &warpper_posx->Pixels[0]);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGB, warpper_negx->width, warpper_negx->height, 0, GL_RGB, GL_UNSIGNED_BYTE, &warpper_negx->Pixels[0]);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGB, warpper_posz->width, warpper_posz->height, 0, GL_RGB, GL_UNSIGNED_BYTE, &warpper_posz->Pixels[0]);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGB, warpper_negz->width, warpper_negz->height, 0, GL_RGB, GL_UNSIGNED_BYTE, &warpper_negz->Pixels[0]);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGB, warpper_posy->width, warpper_posy->height, 0, GL_RGB, GL_UNSIGNED_BYTE, &warpper_posy->Pixels[0]);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGB, warpper_negy->width, warpper_negy->height, 0, GL_RGB, GL_UNSIGNED_BYTE, &warpper_negy->Pixels[0]);
    glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

    glGenVertexArrays(1, &vao_skybox);
    glBindVertexArray(vao_skybox);

    glGenBuffers(1, &ibo_skybox);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_skybox);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(skybox_incices), skybox_incices, GL_STATIC_DRAW);

    glGenBuffers(1, &vbo_skybox);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_skybox);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skybox_data), skybox_data, GL_STATIC_DRAW);
    shader_skybox.do_attrib("position", 3, GL_FLOAT, GL_FALSE, 0, 0);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void release_skybox()
{
    if (textureid_skybox > 0)
        glDeleteTextures(1, &textureid_skybox);

    glDeleteBuffers(1, &vbo_skybox);
    glDeleteBuffers(1, &ibo_skybox);
    glDeleteBuffers(1, &vao_skybox);
    shader_skybox.UnloadAll();
}

void gl_print(const char *text, int x, int y, bool from_top, bool from_left,
    float r, float g, float b, void* font)
{
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    int window_width = glutGet(GLUT_WINDOW_WIDTH);
    int window_height = glutGet(GLUT_WINDOW_HEIGHT);
    gluOrtho2D(0, window_width, 0, window_height);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    GLboolean lighting = glIsEnabled(GL_LIGHTING);
    glDisable(GL_LIGHTING);
    glColor3f(r, g, b);
    if (from_top)
        y = window_height - y;
    if (!from_left)
        x = window_width - x;
    glRasterPos2f(x, y);
    int len = (int)strlen(text);
    for (int i=0; i<len; i++) {
        glutBitmapCharacter(font, text[i]);
    }

    if (lighting)
        glEnable(GL_LIGHTING);

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
}

void cube(vector<pos_normal_uv> &cube_data, vector<unsigned int> &cube_indices)
{
    static GLfloat data[] =
    {
        -0.5, 0.5, 0.5,     -0.5, 0.5, 0.5,     0, 0,
        -0.5, -0.5, 0.5,    -0.5, -0.5, 0.5,    0, 1,
        0.5, -0.5, 0.5,     0.5, -0.5, 0.5,     1, 1,
        0.5, 0.5, 0.5,      0.5, 0.5, 0.5,      1, 0,

        -0.5, 0.5, -0.5,    -0.5, 0.5, -0.5,    0, 0,
        -0.5, -0.5, -0.5,   -0.5, -0.5, -0.5,   0, 1,
        0.5, -0.5, -0.5,    0.5, -0.5, -0.5,    1, 1,
        0.5, 0.5, -0.5,     0.5, 0.5, -0.5,     1, 0
    };

    static GLuint indices[] =
    {
        0, 1, 2, 0, 2, 3,
        3, 2, 6, 3, 6, 7,
        7, 6, 5, 7, 5, 4,
        4, 5, 1, 4, 1, 0,
        4, 0, 3, 4, 3, 7,
        1, 5, 6, 1, 6, 2
    };

    cube_data.resize(8);
    memcpy(&cube_data[0], data, sizeof(data));

    cube_indices.resize(sizeof(indices)/sizeof(GLuint));
    memcpy(&cube_indices[0], indices, sizeof(indices));
}

static char* float2str(const double f)
{
    static char cBuff[100];
    ::sprintf(cBuff, "%f", f);
    return cBuff;
}

string int2str(const long i)
{
    stringstream ss;
    ss << i;
    return ss.str();
}

long str2int(const string& s)
{
    return ::atoi(s.c_str());
}

string tune_bar(string title, float value)
{
    int v = min(9, max(0, int(value * 10.f)));
    string bar = " [ .......... ]";
    bar[3+v] = '0' + v;
    return title + bar;
}

void print_control_hint()
{
    string curobj = "all objects";
    if (select_object >= 0)
    {
        curobj = model_transforms[select_object].model_name;
    }

    gl_print("transform object: ", 9, 30, true, true);
    gl_print(curobj.c_str(), 169, 30, true, true, 0.98, 0.69, 0.79);

    char *fov = float2str(fovy);
    gl_print("current fov:", 220, 30, true, false);
    gl_print(fov, 100, 30, true, false);

    gl_print("mouse wheel        - scale object", 400, 95, false, false);
    gl_print("mouse left drag    - rotate object", 400, 80, false, false);
    gl_print("mouse right drag   - free look at", 400, 65, false, false);
    gl_print("mouse middle drag  - move point light", 400, 50, false, false);
    gl_print("cursor arrows      - tune point light", 400, 35, false, false);
    gl_print("w, s, a, d         - move on x-z plane", 400, 20, false, false);
    gl_print("<Tab>              - select objects", 400, 5, false, false);

    string bar = tune_bar("ambient", (pointlight1.ambient.r + pointlight1.ambient.g + pointlight1.ambient.b)/3.f);
    gl_print(bar.c_str(), 270, 90, true, false, 0.58, 0.89, 0.69);
    bar = tune_bar("diffuse", (pointlight1.diffuse.r + pointlight1.diffuse.g + pointlight1.diffuse.b)/3.f);
    gl_print(bar.c_str(), 270, 110, true, false, 0.58, 0.89, 0.69);
    bar = tune_bar("specular", (pointlight1.specular.r + pointlight1.specular.g + pointlight1.specular.b)/3.f);
    gl_print(bar.c_str(), 279, 130, true, false, 0.58, 0.89, 0.69);
    bar = tune_bar("shininess", material_shininess);
    gl_print(bar.c_str(), 288, 150, true, false, 0.58, 0.89, 0.69);
    bar = tune_bar("alpha", material_alpha);
    gl_print(bar.c_str(), 252, 170, true, false, 0.58, 0.89, 0.69);

    gl_print("<--", 69, 90 + current_tune_bar*20, true, false, 0.98, 0.69, 0.19);
}

}
