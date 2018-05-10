#if !defined(GLLOOKAT_INCLUDED_)
#define GLLOOKAT_INCLUDED_

namespace glcommon
{

extern float v_dot3f(float *x, float *y);
extern float v_length3f(float *x);
extern void v_normalize3f(float &x, float &y, float &z);
extern void v_normalize3f(float *x);
extern void v_normalize3f(float *v, float *x);
extern void v_add3f(float *v, float *v1, float *v2);
extern void v_substract3f(float *v, float *v1, float *v2);
extern void v_cross3f(float *v, float *x, float *y);

struct matrix4f
{
    //          c  r
    float _data[4][4];
    // stored in column-major order:
    // a11,a21,a31,a41
    // a12,a22,a32,a42
    // a13,a23,a33,a43
    // a14,a24,a34,a44

    matrix4f();
    matrix4f(const matrix4f &m);
    matrix4f(float r0c0, float r1c0, float r2c0, float r3c0,
        float r0c1, float r1c1, float r2c1, float r3c1,
        float r0c2, float r1c2, float r2c2, float r3c2,
        float r0c3 = 0, float r1c3 = 0, float r2c3 = 0, float r3c3 = 1);
    void update(float r0c0, float r1c0, float r2c0, float r3c0,
        float r0c1, float r1c1, float r2c1, float r3c1,
        float r0c2, float r1c2, float r2c2, float r3c2,
        float r0c3, float r1c3, float r2c3, float r3c3);
    void loadidentity();
    void diagonalize(float a);
    void multiply(const matrix4f &m); // this = this * m
    void multiply_from(const matrix4f &m);  // this = m * this
    void copyto(matrix4f &m);
    const float* pointer() const;
};

struct vec3f
{
    union
    {
        float _data[3];
        struct
        {
            union{float x; float r;};
            union{float y; float g;};
            union{float z; float b;};
        };
    };
    float &operator[](int i) {if (i==0) return x; if (i==1) return y; /*if (i==2)*/ return z;}
    vec3f(float _x, float _y, float _z):x(_x), y(_y), z(_z) {}
    vec3f() {}
};

extern float vec3f_dot(vec3f &v1, vec3f &v2);
extern float vec3f_length(vec3f &v);
extern void vec3f_normalize(vec3f &v);
extern void vec3f_add(vec3f &v, vec3f &v1, vec3f &v2);
extern void vec3f_substract(vec3f &v, vec3f &v1, vec3f &v2);
extern void vec3f_cross(vec3f &v, vec3f &v1, vec3f &v2);

struct vec4f
{
    union
    {
        float _data[4];
        struct
        {
            union{float x; float r;};
            union{float y; float g;};
            union{float z; float b;};
            union{float w; float a;};
        };
    };

    float &operator[](int i) {return *(_data + i);}
    vec4f(){}
    vec4f(vec3f v3f){_data[0] = v3f.x; _data[1] = v3f.y; _data[2] = v3f.z; _data[3] = 0;}
    vec4f(float x, float y, float z, float w);
    void transform_me(const matrix4f &m);
};

static inline vec3f vec4f_to_3f(const vec4f &v4f)
{
    return vec3f(v4f._data[0], v4f._data[1], v4f._data[2]);
}

static inline vec4f vec3f_to_4f(const vec3f &v3f, const float p)
{
    vec4f v4f(v3f);
    v4f._data[3] = p;
    return v4f;
}

extern void updatev(vec3f &v, float arr[]);
extern void updatev(vec4f &v, float arr[]);
extern void updatev(vec3f &v, float v0, float v1, float v2);
extern void updatev(vec4f &v, float v0, float v1, float v2, float v3);
extern void readv(const vec3f &v, float arr[]);
extern void readv(const vec4f &v, float arr[]);

// result = m*v
extern void m4f_multiply_v4f(vec4f &result, const matrix4f &m, const vec4f &v);
// c = a*b
extern void m4f_multiply(matrix4f &c, const matrix4f &a, const matrix4f &b);

extern void m4f_rotate(matrix4f &m, float degree, float x, float y, float z);
// apply rotate to a existed matrix
extern void multipy_rotate(matrix4f &m, float degree, float x, float y, float z);

extern void m4f_translate(matrix4f &m, float translate_x, float translate_y, float translate_z);
extern void multipy_translate(matrix4f &m, float translate_x, float translate_y, float translate_z);

extern void m4f_scale(matrix4f &m, float scale_x, float scale_y, float scale_z);
extern void multipy_scale(matrix4f &m, float scale_x, float scale_y, float scale_z);

extern void m4f_lookat(matrix4f &m, float camera_x, float camera_y, float camera_z,
    float target_x, float target_y, float target_z, float up_x, float up_y, float up_z);
extern void multipy_lookat(matrix4f &m, float camera_x, float camera_y, float camera_z,
    float target_x, float target_y, float target_z, float up_x, float up_y, float up_z);

extern void m4f_perspective(matrix4f &m, float fovy, float aspect, float znear, float zfar);

extern vec3f screen2sphere(float x, float y);
extern void quaternion(const vec3f &n, float theta, matrix4f &r);

/*
void glrotate(float angle, float x, float y, float z);
void gllookat(float camera_x, float camera_y, float camera_z,
   float target_x, float target_y, float target_z,
   float up_x, float up_y, float up_z);
void glperspective(float fovy, float aspect, float znear, float zfar);
*/

struct pos_normal_uv
{
    vec3f pos;
    vec3f normal;
    float u, v;
    pos_normal_uv() {}
};

struct mesh_bound
{
    float mesh_bound_x_max;
    float mesh_bound_x_min;
    float mesh_bound_y_max;
    float mesh_bound_y_min;
    float mesh_bound_z_max;
    float mesh_bound_z_min;
};

}
#endif
