#ifndef LOADSHADERS_H
#define LOADSHADERS_H

#include <string>
#include <fstream>
#include <sstream>
using namespace std;

#ifdef __GNUC__
#include <GL/gl.h>
#endif

#ifdef _MSC_FULL_VER
#include "c:\\sdks\\glew\\include\\GL\\glew.h"
#endif

namespace glcommon
{

struct shader_handles
{
    GLuint handler_vertexshader = 0;
    GLuint handler_fragmentshader = 0;
    GLuint handler_program = 0;
    shader_handles():handler_vertexshader(0), handler_fragmentshader(0), handler_program(0){}

    void use_me() {
        glUseProgram(this->handler_program);
    }

    void do_not_use_me() {
        glUseProgram(0);
    }

    GLuint do_attrib(char* attrib, GLint size, GLenum type, GLboolean normalized,
        GLsizei stride, const GLvoid *pointer)
    {
        GLuint attrib_location = glGetAttribLocation(this->handler_program, attrib);
        glEnableVertexAttribArray(attrib_location);
        glVertexAttribPointer(attrib_location, size, type, normalized, stride, pointer);
        return attrib_location;
    }

    void UnloadAll(){
        if (handler_program) {
            glDeleteProgram(handler_program);
            handler_program = 0;
        }

        if (handler_vertexshader) {
            glDeleteShader(handler_vertexshader);
            handler_vertexshader = 0;
        }

        if (handler_fragmentshader) {
            glDeleteShader(handler_fragmentshader);
            handler_fragmentshader = 0;
        }
    }
};

static shader_handles LoadShaders(string vertex_shader, string fragment_shader)
{
    shader_handles result;
    const char *vv = vertex_shader.c_str();
    const char *ff = fragment_shader.c_str();

    result.handler_vertexshader = glCreateShader(GL_VERTEX_SHADER);
    result.handler_fragmentshader = glCreateShader(GL_FRAGMENT_SHADER);

    glShaderSource(result.handler_vertexshader, 1, &vv, NULL);
    glShaderSource(result.handler_fragmentshader, 1, &ff, NULL);

    glCompileShader(result.handler_vertexshader);
    glCompileShader(result.handler_fragmentshader);

    result.handler_program = glCreateProgram();

    glAttachShader(result.handler_program, result.handler_vertexshader);
    glAttachShader(result.handler_program, result.handler_fragmentshader);

    glLinkProgram(result.handler_program);

    glDetachShader(result.handler_program, result.handler_vertexshader);
    glDetachShader(result.handler_program, result.handler_fragmentshader);

    GLint status;
    glGetProgramiv(result.handler_program, GL_LINK_STATUS, &status);
    if (status == GL_FALSE) {
        string msg("shader program linking failure: ");

        GLint infoLogLength;
        glGetProgramiv(result.handler_program, GL_INFO_LOG_LENGTH, &infoLogLength);
        char* strInfoLog = new char[infoLogLength + 1];
        glGetProgramInfoLog(result.handler_program, infoLogLength, NULL, strInfoLog);
        msg += strInfoLog;
        delete[] strInfoLog;

        glDeleteProgram(result.handler_program);
        result.handler_program = 0;
        throw runtime_error(msg);
    }

    return result;
}

static string file2string(const string& filename)
{
    ifstream ifs(filename.c_str(), ios_base::in);
    if (!ifs.is_open())
    {
        return "";
    }

    ostringstream buf;
    char c;
    while (buf && ifs.get(c))
    {
        buf.put(c);
    }

    string text = buf.str();
    return text;
}

static string simple_vertex_shader = "\
#version 130                                                                \n\
in vec3 position;                                                           \n\
uniform mat4 matrix;                                                        \n\
void main()                                                                 \n\
{                                                                           \n\
    gl_Position = matrix * vec4(position, 1.0);                             \n\
}                                                                           \n";

static string simple_fragment_shader = "\
#version 130                                                                \n\
out vec4 fragshader_color;                                                  \n\
uniform vec4 wireframe_color;                                               \n\
void main()                                                                 \n\
{                                                                           \n\
    fragshader_color = wireframe_color;                                     \n\
}                                                                           \n";

static string vertex_shader_skybox = "\
#version 130                                                                                    \n\
in vec3 position;                                                                               \n\
out vec3 textcoord;                                                                             \n\
uniform mat4 matrix;                                                                            \n\
void main()                                                                                     \n\
{                                                                                               \n\
    vec4 pos_pvm = matrix * vec4(position, 1.0);                                                \n\
    gl_Position = pos_pvm.xyww;                                                                 \n\
	textcoord = position;                                                                       \n\
}                                                                                               \n";

static string fragment_shader_skybox =                                                          "\
#version 130                                                                                    \n\
in vec3 textcoord;                                                                              \n\
out vec4 FragColor;                                                                             \n\
uniform samplerCube skyboxTex;                                                                  \n\
void main()                                                                                     \n\
{                                                                                               \n\
    FragColor = texture(skyboxTex, textcoord);                                                  \n\
}                                                                                               \n";

static string vertex_shader_matrics = "\
#version 130                                                                \n\
in vec3 position;                                                           \n\
in vec3 normal;                                                             \n\
in vec2 uv;                                                                 \n\
out vec2 _uv;                                                               \n\
out vec3 position_world;                                                    \n\
out vec3 position_incam;                                                    \n\
out vec3 normal_incam;                                                      \n\
uniform mat4 model_matrix;                                                  \n\
uniform mat4 camera_matrix;                                                 \n\
uniform mat4 perspective_matrix;                                            \n\
void main()                                                                 \n\
{                                                                           \n\
    _uv = uv;                                                               \n\
    mat4 modelview_matrix = camera_matrix * model_matrix;                   \n\
    mat4 camera_PVM = perspective_matrix * modelview_matrix;                \n\
    position_world = vec3(model_matrix * vec4(position, 1.0));              \n\
    position_incam = vec3(modelview_matrix * vec4(position, 1.0));          \n\
    normal_incam = vec3(modelview_matrix * vec4(normal, 0));                \n\
    gl_Position = camera_PVM * vec4(position, 1.0);                         \n\
}                                                                           \n";

static string fragment_shader_phong = "\
#version 130                                                                \n\
in vec2 _uv;                                                                \n\
in vec3 position_world;                                                     \n\
in vec3 position_incam;                                                     \n\
in vec3 normal_incam;                                                       \n\
out vec4 fragshader_color;                                                  \n\
uniform vec3 dirlight_ambient;                                              \n\
uniform vec3 dirlight_diffuse;                                              \n\
uniform vec3 dirlight_specular;                                             \n\
uniform vec3 dirlight_direction;                                            \n\
uniform vec3 pointlight_ambient;                                            \n\
uniform vec3 pointlight_diffuse;                                            \n\
uniform vec3 pointlight_specular;                                           \n\
uniform vec3 pointlight_position;                                           \n\
uniform vec3 pointlight_attenuation;                                        \n\
uniform vec3 material_rgb;                                                  \n\
uniform float material_specularpower; //shininess                           \n\
uniform float material_alpha;         //transparency                        \n\
uniform sampler2D colortexture;                                             \n\
uniform int use_texture;                                                    \n\
uniform vec4 uniform_color;                                                 \n\
void main()                                                                 \n\
{                                                                           \n\
    vec3 normal = normalize(normal_incam);                                                      \n\
    vec3 view_direction = -normalize(position_incam);                                           \n\
    // --- directional light source ---                                                         \n\
    vec3 light_direction = normalize(dirlight_direction - position_incam);                      \n\
    vec3 reflect_direction = normalize(-reflect(light_direction, normal));                      \n\
    vec3 eye_normal = normalize(view_direction + light_direction);                              \n\
    float diff = max(dot(normal, light_direction), 0.0);                                        \n\
    float spec = pow(max(dot(view_direction, reflect_direction), 0.0), material_specularpower); \n\
    vec3 ambient = dirlight_ambient * material_rgb;                                             \n\
    vec3 diffuse = dirlight_diffuse * diff * material_rgb;                                      \n\
    vec3 specular = dirlight_specular * spec * material_rgb;                                    \n\
    vec3 light_dir = ambient + diffuse + specular;                                              \n\
    // --- point light source ---                                                               \n\
    light_direction = normalize(pointlight_position - position_incam);                          \n\
    reflect_direction = normalize(-reflect(light_direction, normal));                           \n\
    eye_normal = normalize(view_direction + light_direction);                                   \n\
    diff = max(dot(normal, light_direction), 0.0);                                              \n\
    spec = pow(max(dot(view_direction, reflect_direction), 0.0), material_specularpower);       \n\
    float distance = length(pointlight_position - position_incam);                              \n\
    float attenuation = 1.0f / (pointlight_attenuation[0] + pointlight_attenuation[1]*distance +\n\
        pointlight_attenuation[2]*(distance * distance));                                       \n\
    ambient = attenuation * pointlight_ambient * material_rgb;                                  \n\
    diffuse = attenuation * pointlight_diffuse * material_rgb *                                 \n\
        max(0, dot(normal, light_direction));                                                   \n\
    specular = attenuation * pointlight_specular * material_rgb *                               \n\
        pow(max(0, dot(reflect_direction, eye_normal)), material_specularpower);                \n\
    vec3 light_pnt = ambient + diffuse + specular;                                              \n\
                                                                                                \n\
    // --- combine the 2 lights ---                                                             \n\
    vec3 texture_rgb = vec3(1, 1, 1);                                                           \n\
    if (use_texture > 0.1)                                                                      \n\
    {                                                                                           \n\
        texture_rgb = texture2D(colortexture, _uv).rgb;                                         \n\
    }                                                                                           \n\
    fragshader_color = vec4((light_dir + light_pnt) * texture_rgb, material_alpha);             \n\
                                                                                                \n\
    if (uniform_color.a > 0.999)                                                                \n\
    {                                                                                           \n\
        fragshader_color = uniform_color;                                                       \n\
    }                                                                                           \n\
}                                                                                               \n";

}

#endif // LOADSHADERS_H

/*
datafile = open('glsl.txt', 'r')
output = ''
lines = datafile.readlines(1000000)
for line in lines:
    line = line[0:-1]
    if len(line) < 90:
        for i in range(0, 90 - len(line)):
            line += ' '
    line += '\\n\\' + '\n'
    output += line
datafile = open('xxx.txt', 'w')
datafile.write(output)
datafile.close()
*/
