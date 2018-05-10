#include "matrix4f.h"
#include <memory.h>
#include <math.h>

namespace glcommon
{

const float PI = 3.14159f;  // (acos(-1.0))

float v_dot3f(float *x, float *y)
{
    return x[0] * y[0] + x[1] * y[1] + x[2] * y[2];
}

float v_length3f(float *x)
{
    return (float)sqrt(v_dot3f(x, x));
}

void v_normalize3f(float &x, float &y, float &z)
{
    float module = (float)sqrt(x * x + y * y + z * z);
    x = x / module;
    y = y / module;
    z = z / module;
}

void v_normalize3f(float *x)
{
    float module = v_length3f(x);
    x[0] = x[0] / module;
    x[1] = x[1] / module;
    x[2] = x[2] / module;
}

void v_normalize3f(float *v, float *x)
{
    memcpy(v, x, sizeof(float)*3);
    v_normalize3f(v);
}

void v_add3f(float *v, float *v1, float *v2)
{
    v[0] = v1[0] + v2[0];
    v[1] = v1[1] + v2[1];
    v[2] = v1[2] + v2[2];
}

void v_substract3f(float *v, float *v1, float *v2)
{
    v[0] = v1[0] - v2[0];
    v[1] = v1[1] - v2[1];
    v[2] = v1[2] - v2[2];
}

void v_cross3f(float *v, float *x, float *y)
{
    v[0] = x[1] * y[2] - x[2] * y[1];
    v[1] = x[2] * y[0] - x[0] * y[2];
    v[2] = x[0] * y[1] - x[1] * y[0];
}

void v_negate(float *v)
{
    v[0] = -v[0];
    v[1] = -v[1];
    v[2] = -v[2];
}

matrix4f::matrix4f()
{
    memset(_data, 0, sizeof(_data));
    return;
}

matrix4f::matrix4f(const matrix4f& m)
{
    memcpy(_data, m._data, sizeof(_data));
}

matrix4f::matrix4f(float r0c0, float r1c0, float r2c0, float r3c0,
    float r0c1, float r1c1, float r2c1, float r3c1,
    float r0c2, float r1c2, float r2c2, float r3c2,
    float r0c3, float r1c3, float r2c3, float r3c3)
{
    this->update(r0c0, r1c0, r2c0, r3c0, r0c1, r1c1, r2c1, r3c1,
        r0c2, r1c2, r2c2, r3c2, r0c3, r1c3, r2c3, r3c3);
}

void matrix4f::update(float r0c0, float r1c0, float r2c0, float r3c0,
    float r0c1, float r1c1, float r2c1, float r3c1,
    float r0c2, float r1c2, float r2c2, float r3c2,
    float r0c3, float r1c3, float r2c3, float r3c3)
{
    //    c  r     r c
    _data[0][0] = r0c0;
    _data[0][1] = r1c0;
    _data[0][2] = r2c0;
    _data[0][3] = r3c0;
    _data[1][0] = r0c1;
    _data[1][1] = r1c1;
    _data[1][2] = r2c1;
    _data[1][3] = r3c1;
    _data[2][0] = r0c2;
    _data[2][1] = r1c2;
    _data[2][2] = r2c2;
    _data[2][3] = r3c2;
    _data[3][0] = r0c3;
    _data[3][1] = r1c3;
    _data[3][2] = r2c3;
    _data[3][3] = r3c3;
}

void matrix4f::loadidentity()
{
    memset(_data, 0, sizeof(_data));
    _data[0][0] = 1.0f;
    _data[1][1] = 1.0f;
    _data[2][2] = 1.0f;
    _data[3][3] = 1.0f;
}

void matrix4f::diagonalize(float a)
{
    //    r  c
    _data[0][0] = a;
    _data[1][1] = a;
    _data[2][2] = a;
    _data[3][3] = a;
}

void matrix4f::multiply(const matrix4f &m)
{
    m4f_multiply(*this, *this, m);
}

void matrix4f::multiply_from(const matrix4f &m)
{
    m4f_multiply(*this, m, *this);
}

void matrix4f::copyto(matrix4f& m)
{
    memcpy(m._data, _data, sizeof(_data));
}

const float* matrix4f::pointer() const
{
    return &_data[0][0];
}

float vec3f_dot(vec3f &v1, vec3f &v2)
{
    return v1.x*v2.x + v1.y*v2.y + v1.z*v2.z;
}

float vec3f_length(vec3f &v)
{
    return sqrt(vec3f_dot(v, v));
}

void vec3f_normalize(vec3f &v)
{
    float module = vec3f_length(v);
    v.x = v.x / module;
    v.y = v.y / module;
    v.z = v.z / module;
}

void vec3f_add(vec3f &v, vec3f &v1, vec3f &v2)
{
    v.x = v1.x + v2.x;
    v.y = v1.y + v2.y;
    v.z = v1.z + v2.z;
}

void vec3f_substract(vec3f &v, vec3f &v1, vec3f &v2)
{
    v.x = v1.x - v2.x;
    v.y = v1.y - v2.y;
    v.z = v1.z - v2.z;
}

void vec3f_cross(vec3f &v, vec3f &v1, vec3f &v2)
{
    v.x = v1.y * v2.z - v1.z * v2.y;
    v.y = v1.z * v2.x - v1.x * v2.z;
    v.z = v1.x * v2.y - v1.y * v2.x;
}

vec4f::vec4f(float x, float y, float z, float w)
{
    this->_data[0] = x;
    this->_data[1] = y;
    this->_data[2] = z;
    this->_data[3] = w;
}

void vec4f::transform_me(const matrix4f &m)
{
    vec4f v;
    m4f_multiply_v4f(v, m, *this);
    *this = v;
}

void updatev(vec3f &v, float arr[])
{
    v._data[0] = arr[0];
    v._data[1] = arr[1];
    v._data[2] = arr[2];
}

void updatev(vec4f &v, float arr[])
{
    v._data[0] = arr[0];
    v._data[1] = arr[1];
    v._data[2] = arr[2];
    v._data[3] = arr[3];
}

void updatev(vec3f &v, float v0, float v1, float v2)
{
    v._data[0] = v0;
    v._data[1] = v1;
    v._data[2] = v2;
}

void updatev(vec4f &v, float v0, float v1, float v2, float v3)
{
    v._data[0] = v0;
    v._data[1] = v1;
    v._data[2] = v2;
    v._data[3] = v3;
}

void readv(const vec3f &v, float arr[])
{
    arr[0] = v._data[0];
    arr[1] = v._data[1];
    arr[2] = v._data[2];
}

void readv(const vec4f &v, float arr[])
{
    arr[0] = v._data[0];
    arr[1] = v._data[1];
    arr[2] = v._data[2];
    arr[3] = v._data[3];
}

void m4f_multiply_v4f(vec4f &result, const matrix4f &m, const vec4f &v)
{
    vec4f t;
    for (int i = 0; i<4; ++i)
    {
        t._data[i] = 0;
        for (int j = 0; j<4; ++j)
        {
            t._data[i] += m._data[j][i] * v._data[j];
        }
    }

    result = t;
}

// B = A.transpose
inline void transpose(float *B, float *A, int dim)
{
    for (int i=0; i<dim; i++)
    {
        for (int j=0; j<dim; j++)
        {
            //  matrix[i][j]第i行第j列，  偏移为： i整行 + j零头,  行长度为dim  => i*dim + j
            // B[i][j] = A[j][i]
            *(B + i*dim + j) = *(A + j*dim + i);
        }
    }
}

//   row_major_order!!!              C<m,q>  =  A<m,n>  *  B<n,q>
inline void matmul_row_major_order(float *C, float *A, float *B, int m, int n, int q)
{
    memset(C, 0, m*q*sizeof(float));

    for (int i=0; i<m; i++)
    {
        for (int j=0; j<q; j++)
        {
            for (int k=0; k<n; k++)
            {
                // 计算的是偏移：  !!! 注意数组从0开始计数 !!!
                //  C[i][j]第i行第j列，  偏移为： i整行 + j零头,  行长度为q  => i*q + j
                //  A[i][k]第i行第k列，  偏移为： i整行 + k零头,  行长度为n  => i*n + k
                //  B[k][j]第k行第j列，  偏移为： k整行 + j零头,  行长度为q  => k*q + j

                //  R[i][j]    +=     A[i][k]    *     B[k][j];
                *(C + i*q + j) += *(A + i*n + k) * *(B + k*q + j);
            }
        }
    }
}

// column major order
// notice that c might share the same address of a or b !!!!!
void m4f_multiply(matrix4f &c, const matrix4f &a, const matrix4f &b)
{
/*
    matrix4f t;

    for (int i = 0; i<4; ++i)
    {
        for (int k = 0; k<4; ++k)
        {
            float r = a._data[k][i];
            for (int j = 0; j<4; ++j)
            {
                t._data[j][i] += r*b._data[j][k];
            }
        }
    }

    c = t;
*/

    matrix4f at, bt, ct;
    transpose((float*)&at, (float*)&a, 4);
    transpose((float*)&bt, (float*)&b, 4);
    matmul_row_major_order((float*)&ct, (float*)&at, (float*)&bt, 4, 4, 4);
    transpose((float*)&c, (float*)&ct, 4);
}

void m4f_rotate(matrix4f &m, float degree, float x, float y,
    float z)
{
    double a = degree / 180 * 3.1415926;
    double s = sin(a);
    double c = cos(a);
    double t = 1.0f - c;

    m._data[0][0] = (float)(t * x * x + c);
    m._data[0][1] = (float)(t * x * y + s * z);
    m._data[0][2] = (float)(t * x * z - s * y);
    m._data[1][0] = (float)(t * x * y - s * z);
    m._data[1][1] = (float)(t * y * y + c);
    m._data[1][2] = (float)(t * y * z + s * x);
    m._data[2][0] = (float)(t * x * z + s * y);
    m._data[2][1] = (float)(t * y * z - s * x);
    m._data[2][2] = (float)(t * z * z + c);
    m._data[0][3] = m._data[1][3] = m._data[2][3] = 0.0f;
    m._data[3][0] = m._data[3][1] = m._data[3][2] = 0.0f;
    m._data[3][3] = 1.0f;
}

void multipy_rotate(matrix4f &m, float degree, float x, float y,
    float z)
{
    matrix4f tm;
    m4f_rotate(tm, degree, x, y, z);
    m.multiply(tm);
}

/*
void glrotate(float angle, float x, float y, float z)
{
    matrix4f m;
    m4f_rotate(m, angle, x, y, z);
    glMultMatrixf(&m._data[0][0]);
}

void gltranslate(float translate_x, float translate_y, float translate_z)
{
    matrix4f m;
    m4f_translate(m, translate_x, translate_y, translate_z);
    glMultMatrixf(&m._data[0][0]);
}
*/

void m4f_translate(matrix4f &m, float translate_x, float translate_y,
    float translate_z)
{
    m._data[0][0] = m._data[1][1] = m._data[2][2] = m._data[3][3] = 1.0f;
    m._data[0][1] = m._data[0][2] = m._data[0][3] = 0.0f;
    m._data[1][0] = m._data[1][2] = m._data[1][3] = 0.0f;
    m._data[2][0] = m._data[2][1] = m._data[2][3] = 0.0f;
    m._data[3][0] = translate_x;
    m._data[3][1] = translate_y;
    m._data[3][2] = translate_z;
}

void multipy_translate(matrix4f &m, float translate_x, float translate_y,
    float translate_z)
{
    matrix4f tm;
    m4f_translate(tm, translate_x, translate_y, translate_z);
    m.multiply(tm);
}

void m4f_scale(matrix4f &m, float scale_x, float scale_y, float scale_z)
{
    m._data[0][0] = scale_x;
    m._data[1][1] = scale_y;
    m._data[2][2] = scale_z;
    m._data[0][1] = m._data[0][2] = m._data[0][3] = 0.0f;
    m._data[1][0] = m._data[1][2] = m._data[1][3] = 0.0f;
    m._data[2][0] = m._data[2][1] = m._data[2][3] = 0.0f;
    m._data[3][0] = m._data[3][1] = m._data[3][2] = 0.0f;
    m._data[3][3] = 1.0f;
}

void multipy_scale(matrix4f &m, float scale_x, float scale_y, float scale_z)
{
    matrix4f tm;
    m4f_scale(tm, scale_x, scale_y, scale_z);
    m.multiply(tm);
}

void m4f_lookat(matrix4f &m, float camera_x, float camera_y, float camera_z,
    float target_x, float target_y, float target_z, float up_x, float up_y, float up_z)
{
    float camera[3];
    camera[0] = camera_x;
    camera[1] = camera_y;
    camera[2] = camera_z;

    float forward[3];
    forward[0] = target_x - camera_x;
    forward[1] = target_y - camera_y;
    forward[2] = target_z - camera_z;
    v_normalize3f(forward);

    float up[3];
    up[0] = up_x;
    up[1] = up_y;
    up[2] = up_z;

    float right[3];
    v_cross3f(right, forward, up);
    v_normalize3f(right);
    v_cross3f(up, right, forward);

    matrix4f r(
        right[0], up[0], -forward[0], 0,
        right[1], up[1], -forward[1], 0,
        right[2], up[2], -forward[2], 0,
            0,      0,      0,        1);

    matrix4f t(
        1,          0,          0,          0,  //col0
        0,          1,          0,          0,  //col1
        0,          0,          1,          0,  //col2
        -camera[0], -camera[1], -camera[2], 1); //col3

    m4f_multiply(m, r, t);
}

void multipy_lookat(matrix4f &m, float camera_x, float camera_y, float camera_z,
    float target_x, float target_y, float target_z, float up_x, float up_y, float up_z)
{
    matrix4f tm;
    m4f_lookat(tm, camera_x, camera_y, camera_z, target_x, target_y, target_z,
        up_x, up_y, up_z);
    m.multiply(tm);
}

/*
void gllookat(float camera_x, float camera_y, float camera_z,
    float target_x, float target_y, float target_z,
    float up_x, float up_y, float up_z)
{
    matrix4f m;
    m4f_lookat(m, camera_x, camera_y, camera_z, target_x, target_y, target_z,
        up_x, up_y, up_z);
    glLoadMatrixf(&m._data[0][0]);
}
*/

void m4f_perspective(matrix4f &m, float fovy, float aspect, float znear, float zfar)
{
    float c = float(PI / 180.0f);
    float d = float(1 / tan(fovy*c / 2));
    float a = -(zfar + znear) / (zfar - znear);
    float b = -2 * zfar*znear / (zfar - znear);
    m.update(d/aspect,  0,  0,  0,
                0,      d,  0,  0,
                0,      0,  a,  -1,
                0,      0,  b,  0);
}

/*
void glperspective(float fovy, float aspect, float znear, float zfar)
{
    matrix4f r;
    m4f_perspective(r, fovy, aspect, znear, zfar);
    glLoadMatrixf(&r._data[0][0]);
}
*/

vec3f screen2sphere(float x, float y)
{
    vec3f result;
    float length = x*x + y*y;

    if (length > 1.0f)
    {
        float norm = 1.0f / sqrtf(length);
        result.x = x * norm;
        result.y = y * norm;
        result.z = 0.0f;
    }
    else
    {
        result.x = x;
        result.y = y;
        result.z = sqrtf(1.0f - length);
    }

    vec3f_normalize(result);
    return result;
}

void quaternion(const vec3f &n, float theta, matrix4f &r)
{
    float c = PI / 180.0f;
    float q0 = cos(c*theta / 2);
    float q1 = sin(c*theta / 2)*n.x;
    float q2 = sin(c*theta / 2)*n.y;
    float q3 = sin(c*theta / 2)*n.z;

    r._data[0][0] = q0*q0 + q1*q1 - q2*q2 - q3*q3;
    r._data[0][1] = 2.0f*q1*q2 - 2.0f*q0*q3;;
    r._data[0][2] = 2.0f*q1*q3 + 2.0f*q0*q2;
    r._data[0][3] = 0.0f;

    r._data[1][0] = 2.0f*q1*q2 + 2.0f*q0*q3;
    r._data[1][1] = q0*q0 - q1*q1 + q2*q2 - q3*q3;
    r._data[1][2] = 2.0f*q2*q3 - 2.0f*q0*q1;
    r._data[1][3] = 0.0f;

    r._data[2][0] = 2.0f*q1*q3 - 2.0f*q0*q2;
    r._data[2][1] = 2.0f*q2*q3 + 2.0f*q0*q1;
    r._data[2][2] = q0*q0 - q1*q1 - q2*q2 + q3*q3;
    r._data[2][3] = 0.0f;

    r._data[3][0] = 0.0f;
    r._data[3][1] = 0.0f;
    r._data[3][2] = 0.0f;
    r._data[3][3] = 1.0f;
}

}
