#version 130   
in vec3 screen;
in vec3 corner_rays;    
out vec3 ray_direction; 

out vec3 sphere0_center;
out vec3 sphere1_center;
out vec3 sphere2_center;
out vec3 plane_point;   
out vec3 plane_normal;  

uniform vec3 _sphere0_center;
uniform vec3 _sphere1_center;
uniform vec3 _sphere2_center;
uniform vec3 _plane_point;   
uniform vec3 _plane_normal;  

uniform mat4 model_matrix;
uniform mat4 lookat_matrix;
uniform mat4 modelview_matrix;
uniform mat4 perspective_matrix;

vec3 model_transform(in vec3 v)
{
    return vec3(model_matrix * vec4(v, 1));
}

void main()
{
    gl_Position = vec4(screen, 1.0);
    ray_direction = vec3(lookat_matrix * vec4(corner_rays, 0));

    sphere0_center = model_transform(_sphere0_center);
    sphere1_center = model_transform(_sphere1_center);
    sphere2_center = model_transform(_sphere2_center);

    // fix the plane
    plane_point = _plane_point;;
    plane_normal = normalize(_plane_normal);
}
