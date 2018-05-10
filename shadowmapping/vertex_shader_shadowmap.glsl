#version 130                                                      
in vec3 position;                                                 
in vec3 normal;                                                   
in vec2 uv;                                                       
out vec2 _uv;                                                     
out vec3 position_world;                                          
out vec3 position_incam;                                          
out vec3 normal_incam;                                            
out vec4 shadow_coord;                                            
uniform mat4 model_matrix;                                        
uniform mat4 camera_matrix;                                       
uniform mat4 perspective_matrix;
uniform mat4 perspective_pointlight;                                  
uniform mat4 pointlight_matrix;                                   
uniform mat4 bias_matrix;                                         
void main()                                                       
{                                                                 
    _uv = uv;                                                     
    mat4 modelview_matrix = camera_matrix * model_matrix;         
    mat4 camera_PVM = perspective_matrix * modelview_matrix;      
    position_world = vec3(model_matrix * vec4(position, 1.0));      
    position_incam = vec3(modelview_matrix * vec4(position, 1.0));
    normal_incam = vec3(modelview_matrix * vec4(normal, 0));      
    gl_Position = camera_PVM * vec4(position, 1.0);               
    shadow_coord = bias_matrix * perspective_pointlight * pointlight_matrix * vec4(position_world, 1.0);
}

