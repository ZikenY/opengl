#version 130                                                              

in vec3 ray_direction;                                                    
out vec4 fragshader_color;                                                

in vec3 sphere0_center;
in vec3 sphere1_center;                                            
in vec3 sphere2_center;                                            
in vec3 plane_point;                                               
in vec3 plane_normal;

uniform float sphere0_radius;                                             
uniform float sphere1_radius;                                             
uniform float sphere2_radius;                                             
uniform float attenuate_factor;
uniform float hit_tolerance;
uniform float pointlight_hitradius;

uniform vec3 dirlight_ambient;                                            
uniform vec3 dirlight_diffuse;                                            
uniform vec3 dirlight_specular;                                           
uniform vec3 dirlight_direction;    // must be normalized
uniform vec3 pointlight_ambient;                                          
uniform vec3 pointlight_diffuse;                                          
uniform vec3 pointlight_specular;                                         
uniform vec3 pointlight_position;                                         
uniform vec3 pointlight_attenuation;                                      
uniform vec3 material_rgb;                                                
uniform float material_specularpower; //shininess                         
uniform float material_alpha;         //transparency     
uniform float enable_pointlight;          
uniform float enable_directionallight;

float shadow_dist_factor;
float medium_attenuate_factor;

float sqr(in float x)
{
    return x*x;
}

float quadratic(in vec3 ray_start, in vec3 ray_dir, in vec3 sphere_center, float sphere_radius,
    out float a, out float b, out float c)
{
    vec3 n = sphere_center - ray_start;
    a = dot(ray_dir, ray_dir);
    b = dot(ray_dir, 2.0 * (-n));
    c = dot(n, n) - sqr(sphere_radius);
    return sqr(b) - 4.0 * a * c;
}

// check whether the two hit points are valid or not
void cross_sphere(vec3 ray_start, vec3 ray_dir, vec3 sphere_center, float sphere_radius,
    out vec3 hit_point0, out vec3 hit_point1, out bool hit_on_0, out bool hit_on_1)
{
    hit_on_0 = hit_on_1 = false;
    float a, b, c;
    float d = quadratic(ray_start, ray_dir, sphere_center, sphere_radius, a, b, c);
    if (d <= hit_tolerance)
        return;

    float t1 = (-b + sqrt(d)) / (2*a);
    float t2 = (-b - sqrt(d)) / (2*a);
    hit_on_0 = t1 >= 0 ? true:false;
    hit_on_1 = t2 >= 0 ? true:false;

    hit_point0 = ray_start + t1 * ray_dir;
    hit_point1 = ray_start + t2 * ray_dir;
}

// return the distance between ray_start and hit_point.  (-1 means no hit)
float hit_sphere(vec3 ray_start, vec3 ray_dir, vec3 sphere_center, float sphere_radius,
    out vec3 hit_point)
{
    float dist = -1;
    float a, b, c;
    float d = quadratic(ray_start, ray_dir, sphere_center, sphere_radius, a, b, c);
    if (d <= hit_tolerance)
        return -1;

    float t1 = (-b + sqrt(d)) / (2*a);
    float t2 = (-b - sqrt(d)) / (2*a);
    if (t1 > hit_tolerance)
        dist = t1;
    if (t2 > hit_tolerance && t2 < t1)
        dist = t2;

    hit_point = ray_start + dist * ray_dir;
    return dist;
}

// return the distance between ray_start and hit_point.  (-1 means no hit)
float hit_plane(vec3 ray_start, vec3 ray_dir, vec3 plane_point, vec3 plane_normal, out vec3 hit_point)
{
    if (dot(ray_dir, plane_normal) == 0)
        return -1.0f;

    float dist = dot(plane_point - ray_start, plane_normal) / dot(ray_dir, plane_normal);
    if (dist < hit_tolerance)
        return -1.0f;

    hit_point = ray_start + dist * ray_dir;
    return dist;
}

float shadow_distance(in vec3 hit_point, in vec3 light_direction, out float medium_attenuate)
{
    bool hit_s0_0, hit_s0_1, hit_s1_0, hit_s1_1, hit_s2_0, hit_s2_1;
    vec3 hit_s0_point0, hit_s0_point1, hit_s1_point0, hit_s1_point1, hit_s2_point0, hit_s2_point1, hit_the_plane;

    // check if this point is blocked for the directional light
    cross_sphere(hit_point, light_direction, sphere0_center, sphere0_radius,
        hit_s0_point0, hit_s0_point1, hit_s0_0, hit_s0_1);
    cross_sphere(hit_point, light_direction, sphere1_center, sphere1_radius,
        hit_s1_point0, hit_s1_point1, hit_s1_0, hit_s1_1);
    cross_sphere(hit_point, light_direction, sphere2_center, sphere2_radius,
        hit_s2_point0, hit_s2_point1, hit_s2_0, hit_s2_1);
    float dist_plane = hit_plane(hit_point, light_direction, plane_point, plane_normal, hit_the_plane);

    medium_attenuate = -1;
    float dist = -1;
    float d = distance(hit_s0_point0, hit_point);
    if (hit_s0_0 && d > hit_tolerance)
        dist = d;

    d = distance(hit_s0_point1, hit_point);
    if (hit_s0_1 && d > hit_tolerance && (dist < 0 || d < dist))
        dist = d;

    d = distance(hit_s1_point0, hit_point);
    if (hit_s1_0 && d > hit_tolerance && (dist < 0 || d < dist))
        dist = d;

    d = distance(hit_s1_point1, hit_point);
    if (hit_s1_1 && d > hit_tolerance && (dist < 0 || d < dist))
        dist = d;

    d = distance(hit_s2_point0, hit_point);
    if (hit_s2_0 && d > hit_tolerance && (dist < 0 || d < dist))
        dist = d;

    d = distance(hit_s2_point1, hit_point);
    if (hit_s2_1 && d > hit_tolerance && (dist < 0 || d < dist))
        dist = d;

    d = distance(hit_the_plane, hit_point);
    if (dist_plane > 0 && d > hit_tolerance && (dist < 0 || d < dist))
        dist = d;

    if (dist_plane > 0)
    {
        // no translucence
        medium_attenuate = -1;
    }
    else
    {
        medium_attenuate = 0;
        if (hit_s0_0 && hit_s0_1)
        {
            medium_attenuate += distance(hit_s0_point0, hit_s0_point1);
        }

        if (hit_s1_0 && hit_s1_1)
        {
            medium_attenuate += distance(hit_s1_point0, hit_s1_point1);
        }

        if (hit_s2_0 && hit_s2_1)
        {
            medium_attenuate += distance(hit_s2_point0, hit_s2_point1);
        }
    }

    return dist;
}

// return 0 - hitted a normal object; 1 - hitted a pointlight; -1 - hitted nothing (to backgroud)
int hit_me(inout vec3 ray_start, inout vec3 ray_dir, inout float accumulate_distance, out vec3 color)
{
    color = vec3(0, 0, 0);
    vec3 hit_point_c0, hit_point_c1, hit_point_c2;
    float dist = hit_sphere(ray_start, ray_dir, sphere0_center, sphere0_radius, hit_point_c0);
    float dist_c1 = hit_sphere(ray_start, ray_dir, sphere1_center, sphere1_radius, hit_point_c1);
    float dist_c2 = hit_sphere(ray_start, ray_dir, sphere2_center, sphere2_radius, hit_point_c2);
    vec3 hit_point = hit_point_c0;
    vec3 sphere_center = sphere0_center;
    int hit_on = 0;

    if (dist_c1 > 0 && (dist_c1 < dist || dist < 0))
    {
        dist = dist_c1;
        hit_point = hit_point_c1;
        sphere_center = sphere1_center;
        hit_on = 1;
    }

    if (dist_c2 > 0 && (dist_c2 < dist || dist < 0))
    {
        dist = dist_c2;
        hit_point = hit_point_c2;
        sphere_center = sphere2_center;
        hit_on = 2;
    }

    vec3 hit_point_plane;
    float dist_plane = hit_plane(ray_start, ray_dir, plane_point, plane_normal, hit_point_plane);
    if (dist_plane > 0 && (dist_plane < dist || dist < 0))
    {
        dist = dist_plane;
        hit_point = hit_point_plane;
        hit_on = 3;
    }

    // check on the point light source
    if (enable_pointlight > 0.1)
    {
        vec3 hit_point0, hit_point1;
        bool hit_on_0, hit_on_1;
        cross_sphere(ray_start, ray_dir, pointlight_position, pointlight_hitradius,
            hit_point0, hit_point1, hit_on_0, hit_on_1);
        float dist_pl = distance(ray_start, (hit_point0 + hit_point1) / 2.0);
        if (hit_on_0 && hit_on_1 && (dist_pl < dist || dist < 0))
        {
            float dist_between = distance(hit_point0, hit_point1);
            accumulate_distance += dist_pl;
            vec3 pntlight = pointlight_ambient + pointlight_diffuse + pointlight_specular;
            color += pntlight * (sqr(dist_between) / sqr(pointlight_hitradius*2))
                * (attenuate_factor - min(attenuate_factor, accumulate_distance))/attenuate_factor;

            float d0 = distance(ray_start, hit_point0);
            float d1 = distance(ray_start, hit_point1);
            if (d0 > d1)
                ray_start = hit_point0;
            else
                ray_start = hit_point1;
            return 1;
        }
    }

    // decide the normal
    vec3 hit_normal = normalize(hit_point - sphere_center); // sphere
    if (hit_on == 3)
    {
        hit_normal = plane_normal;   // plane
    }

    accumulate_distance += dist;

    if (dist >= 0)
    {
        vec3 view_direction = -normalize(hit_point);
        vec3 infinited_dirlight = 100000000.f * dirlight_direction;

        // normalized direction vector against the the directional light
        vec3 light_direction = normalize(infinited_dirlight - hit_point);
        // distance from the nearest obstacle against the shadow
        float medium_attenuate;
        float shadowdist = shadow_distance(hit_point, light_direction, medium_attenuate);
        if (enable_directionallight > 0.1)
        {
            // computer the color contributed by the directional light
            vec3 reflect_direction = normalize(-reflect(light_direction, hit_normal));
            vec3 eye_normal = normalize(view_direction + light_direction);
            float diff = max(dot(hit_normal, light_direction), 0.0);
            float spec = pow(max(dot(view_direction, reflect_direction), 0.0), material_specularpower);
            vec3 ambient = dirlight_ambient * material_rgb;
            vec3 diffuse = dirlight_diffuse * diff * material_rgb;
            vec3 specular = dirlight_specular * spec * material_rgb;
            vec3 light_dir = ambient + diffuse + specular;
            vec3 tmp_color = light_dir * (attenuate_factor - min(attenuate_factor, accumulate_distance))/attenuate_factor;
            if (shadowdist > 0)
            {
                float medium_atte = attenuate_factor <= 0 ? 1 : (sqr(1-medium_attenuate / medium_attenuate_factor));
                tmp_color *= sqr(sqr(medium_atte));
                tmp_color *= min(1, shadowdist);
            }
            color += tmp_color;
        }

        // this is the correct direction vector against the point light
        light_direction = normalize(pointlight_position - hit_point);
        // distance from the nearest obstacle against the shadow
        shadowdist = shadow_distance(hit_point, light_direction, medium_attenuate);
        if (enable_pointlight > 0.1)
        {
            // computer the color illuminated direct from the point light
            vec3 reflect_direction = normalize(-reflect(light_direction, hit_normal));
            vec3 eye_normal = normalize(view_direction + light_direction);
            float diff = max(dot(hit_normal, light_direction), 0.0);
            float spec = pow(max(dot(view_direction, reflect_direction), 0.0), material_specularpower);
            float dist = length(pointlight_position - hit_point);
            float attenuation = 1.0f / (pointlight_attenuation[0] + pointlight_attenuation[1]*dist +
                pointlight_attenuation[2]*(dist * dist));
            vec3 ambient = attenuation * pointlight_ambient * material_rgb;
            vec3 diffuse = attenuation * pointlight_diffuse * material_rgb * max(0, dot(hit_normal, light_direction));
            vec3 specular = attenuation * pointlight_specular * material_rgb *
                pow(max(0, dot(reflect_direction, eye_normal)), material_specularpower);
            vec3 light_pnt = ambient + diffuse + specular;
            vec3 tmp_color = light_pnt * (attenuate_factor - min(attenuate_factor, accumulate_distance)) / attenuate_factor;
            if (shadowdist > 0)
            {
                float medium_atte = attenuate_factor <= 0 ? 1 : (sqr(1-medium_attenuate / medium_attenuate_factor));
                tmp_color *= sqr(sqr(medium_atte));
                tmp_color *= min(1, shadowdist);
            }
            color += tmp_color;
        }

        ray_start = hit_point;
        ray_dir = normalize(reflect(ray_dir, hit_normal));
    }
    else
    {
        return -1; // hit nothing
    }

    return 0;   // hit a normal object
}

void main()                                                               
{
    shadow_dist_factor = 2.0;
    medium_attenuate_factor = 2.0;
    vec3 ray_start = vec3(0, 0, 0);
    vec3 ray_dir = normalize(ray_direction);
    float reflect_strength = 1;
    vec3 final_color = vec3(0,0,0);

    for (int depth = 0; depth < 10; depth++)
    {
        float accumulate_distance = 0;
        vec3 color;
        int hitwhat = hit_me(ray_start, ray_dir, accumulate_distance, color);
        if (hitwhat == -1)
            break;

        if (hitwhat == 1 && depth == 0)
        {
            // the pointlight is inside the view field
            final_color += color;
        }else{
            final_color += color * reflect_strength;
            reflect_strength *= 0.97f;
        }
    }

    fragshader_color = vec4(final_color, 1);
    return;
}                                                                                              
