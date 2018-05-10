#version 130                                          
in vec2 _uv;                                          
in vec3 position_world;                               
in vec3 position_incam;                               
in vec3 normal_incam;                                 
in vec4 shadow_coord;                                 
out vec4 fragshader_color;                            
uniform vec3 dirlight_ambient;                        
uniform vec3 dirlight_diffuse;                        
uniform vec3 dirlight_specular;                       
uniform vec3 dirlight_direction;                      
uniform vec3 pointlight_ambient;                      
uniform vec3 pointlight_diffuse;                      
uniform vec3 pointlight_specular;                     
uniform vec3 pointlight_position;                     
uniform vec3 pointlight_attenuation;                  
uniform vec3 material_rgb;                            
uniform float material_specularpower; //shininess     
uniform float material_alpha;         //transparency  
uniform sampler2D shadowmap;                          
uniform sampler2D colortexture;                       
uniform int use_texture;                              
uniform vec4 uniform_color;                           
void main()                                           
{                                                     
    vec3 normal = normalize(normal_incam);            
    vec3 view_direction = -normalize(position_incam);            
    // --- directional light source ---               
    vec3 light_direction = normalize(dirlight_direction - position_incam);                      
    vec3 reflect_direction = normalize(-reflect(light_direction, normal));                      
    vec3 eye_normal = normalize(view_direction + light_direction);                              
    float diff = max(dot(normal, light_direction), 0.0);                                        
    float spec = pow(max(dot(view_direction, reflect_direction), 0.0), material_specularpower); 
    vec3 ambient = dirlight_ambient * material_rgb;                                             
    vec3 diffuse = dirlight_diffuse * diff * material_rgb;                                      
    vec3 specular = dirlight_specular * spec * material_rgb;                                    
    vec3 light_dir = ambient + diffuse + specular;                                              
    // --- point light source ---                                                               
    light_direction = normalize(pointlight_position - position_incam);                          
    reflect_direction = normalize(-reflect(light_direction, normal));                           
    eye_normal = normalize(view_direction + light_direction);                                   
    diff = max(dot(normal, light_direction), 0.0);                                              
    spec = pow(max(dot(view_direction, reflect_direction), 0.0), material_specularpower);       
    float distance = length(pointlight_position - position_incam);                              
    float attenuation = 1.0f / (pointlight_attenuation[0] + pointlight_attenuation[1]*distance +
        pointlight_attenuation[2]*(distance * distance));                                       
    ambient = attenuation * pointlight_ambient * material_rgb;                                  
    diffuse = attenuation * pointlight_diffuse * material_rgb *                                 
        max(0, dot(normal, light_direction));                                                   
    specular = attenuation * pointlight_specular * material_rgb *                               
        pow(max(0, dot(reflect_direction, eye_normal)), material_specularpower);

    vec3 light_pnt = ambient;                                              

    // shadowmapping apply to the diffuse component on point light
    vec3 ProjCoords = shadow_coord.xyz / shadow_coord.w;
    float shadowdepth = texture(shadowmap, ProjCoords.xy).x;                      
    if (shadowdepth < ProjCoords.z - 0.00001)        // && dot(normal, light_direction) > 0.00001
    {
        light_pnt += diffuse * 0.1;
    }
    else
    {
        light_pnt += diffuse + specular;
    }


    // --- combine the 2 lights ---     
    vec3 texture_rgb = vec3(1, 1, 1);                                                            
    if (use_texture > 0.1)                                                                      
    {
        texture_rgb = texture2D(colortexture, _uv).rgb;
    }                                                                                           

    fragshader_color = vec4((light_dir + light_pnt) * texture_rgb, material_alpha);
}                                                                                     
