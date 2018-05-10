#if !defined(GLAUX_STORE_INCLUDED_)
#define GLAUX_STORE_INCLUDED_

#include "../commoninclude/commoninclude.h"
#include "../commoninclude/stringfuncs.h"
#include "glaux.h"

using namespace ziken;

namespace glcommon
{
    static void store_dirlight(const DirLight &dirlight, Strings &strings)
    {
        strings.AppendLine(FloatToStr(dirlight.ambient.r));
        strings.AppendLine(FloatToStr(dirlight.ambient.g));
        strings.AppendLine(FloatToStr(dirlight.ambient.b));
        strings.AppendLine(FloatToStr(dirlight.diffuse.r));
        strings.AppendLine(FloatToStr(dirlight.diffuse.g));
        strings.AppendLine(FloatToStr(dirlight.diffuse.b));
        strings.AppendLine(FloatToStr(dirlight.specular.r));
        strings.AppendLine(FloatToStr(dirlight.specular.g));
        strings.AppendLine(FloatToStr(dirlight.specular.b));
        strings.AppendLine(FloatToStr(dirlight.direction.x));
        strings.AppendLine(FloatToStr(dirlight.direction.y));
        strings.AppendLine(FloatToStr(dirlight.direction.z));
    }

    static void store_pointlight(const PointLight &pointlight, Strings &strings)
    {
        strings.AppendLine(FloatToStr(pointlight.ambient.r));
        strings.AppendLine(FloatToStr(pointlight.ambient.g));
        strings.AppendLine(FloatToStr(pointlight.ambient.b));
        strings.AppendLine(FloatToStr(pointlight.diffuse.r));
        strings.AppendLine(FloatToStr(pointlight.diffuse.g));
        strings.AppendLine(FloatToStr(pointlight.diffuse.b));
        strings.AppendLine(FloatToStr(pointlight.specular.r));
        strings.AppendLine(FloatToStr(pointlight.specular.g));
        strings.AppendLine(FloatToStr(pointlight.specular.b));
        strings.AppendLine(FloatToStr(pointlight.position.x));
        strings.AppendLine(FloatToStr(pointlight.position.y));
        strings.AppendLine(FloatToStr(pointlight.position.z));
        strings.AppendLine(FloatToStr(pointlight.attenuation._data[0]));
        strings.AppendLine(FloatToStr(pointlight.attenuation._data[1]));
        strings.AppendLine(FloatToStr(pointlight.attenuation._data[2]));
    }

    static void load_dirlight(const Strings &strings, int &delta, DirLight &dirlight)
    {
        dirlight.ambient.r = strings_getf(strings, delta++);
        dirlight.ambient.g = strings_getf(strings, delta++);
        dirlight.ambient.b = strings_getf(strings, delta++);
        dirlight.diffuse.r = strings_getf(strings, delta++);
        dirlight.diffuse.g = strings_getf(strings, delta++);
        dirlight.diffuse.b = strings_getf(strings, delta++);
        dirlight.specular.r = strings_getf(strings, delta++);
        dirlight.specular.g = strings_getf(strings, delta++);
        dirlight.specular.b = strings_getf(strings, delta++);
        dirlight.direction.x = strings_getf(strings, delta++);
        dirlight.direction.y = strings_getf(strings, delta++);
        dirlight.direction.z = strings_getf(strings, delta++);
    }

    static void load_pointlight(const Strings &strings, int &delta, PointLight &pointlight)
    {
        pointlight.ambient.r = strings_getf(strings, delta++);
        pointlight.ambient.g = strings_getf(strings, delta++);
        pointlight.ambient.b = strings_getf(strings, delta++);
        pointlight.diffuse.r = strings_getf(strings, delta++);
        pointlight.diffuse.g = strings_getf(strings, delta++);
        pointlight.diffuse.b = strings_getf(strings, delta++);
        pointlight.specular.r = strings_getf(strings, delta++);
        pointlight.specular.g = strings_getf(strings, delta++);
        pointlight.specular.b = strings_getf(strings, delta++);
        pointlight.position.x = strings_getf(strings, delta++);
        pointlight.position.y = strings_getf(strings, delta++);
        pointlight.position.z = strings_getf(strings, delta++);
        pointlight.attenuation._data[0] = strings_getf(strings, delta++);
        pointlight.attenuation._data[1] = strings_getf(strings, delta++);
        pointlight.attenuation._data[2] = strings_getf(strings, delta++);
    }

    static void store_matrix4f(const matrix4f &m, Strings &strings)
    {
        for (int i = 0; i < 4; i++)
        {
            for (int j = 0; j < 4; j++)
            {
                strings.AppendLine(FloatToStr(m._data[i][j]));
            }
        }
    }

    static void load_matrix4f(const Strings &strings, int &delta, matrix4f &m)
    {
        for (int i = 0; i < 4; i++)
        {
            for (int j = 0; j < 4; j++)
            {
                m._data[i][j] = strings_getf(strings, delta++);
            }
        }
    }

    static void store_transform_params(const transform_params &params, Strings &strings)
    {
        strings.AppendLine(params.model_name);
        strings.AppendLine(params.texture_name);
        strings.AppendLine(FloatToStr(params.rotate_x));
        strings.AppendLine(FloatToStr(params.rotate_y));
        strings.AppendLine(FloatToStr(params.rotate_z));
        strings.AppendLine(FloatToStr(params.scale_x));
        strings.AppendLine(FloatToStr(params.scale_y));
        strings.AppendLine(FloatToStr(params.scale_z));
        strings.AppendLine(FloatToStr(params.translate_x));
        strings.AppendLine(FloatToStr(params.translate_y));
        strings.AppendLine(FloatToStr(params.translate_z));
        store_matrix4f(params.trackball, strings);
    }

    static void load_transform_params(const Strings &strings, int &delta, transform_params &params)
    {
        strings.GetLine(delta++, params.model_name);
        strings.GetLine(delta++, params.texture_name);
        params.rotate_x = strings_getf(strings, delta++);
        params.rotate_y = strings_getf(strings, delta++);
        params.rotate_z = strings_getf(strings, delta++);
        params.scale_x = strings_getf(strings, delta++);
        params.scale_y = strings_getf(strings, delta++);
        params.scale_z = strings_getf(strings, delta++);
        params.translate_x = strings_getf(strings, delta++);
        params.translate_y = strings_getf(strings, delta++);
        params.translate_z = strings_getf(strings, delta++);
        load_matrix4f(strings, delta, params.trackball);
    }

    static string store_scene()
    {
        Strings strings;
        strings.AppendLine(FloatToStr(data_bound_x_max));
        strings.AppendLine(FloatToStr(data_bound_x_min));
        strings.AppendLine(FloatToStr(data_bound_y_max));
        strings.AppendLine(FloatToStr(data_bound_y_min));
        strings.AppendLine(FloatToStr(data_bound_z_max));
        strings.AppendLine(FloatToStr(data_bound_z_min));
        strings.AppendLine(FloatToStr(max_r));
        strings.AppendLine(FloatToStr(fovy));
        strings.AppendLine(FloatToStr(target_phi));
        strings.AppendLine(FloatToStr(target_sita));

        strings.AppendLine(FloatToStr(camera_pos_x));
        strings.AppendLine(FloatToStr(camera_pos_y));
        strings.AppendLine(FloatToStr(camera_pos_z));
        strings.AppendLine(FloatToStr(camera_up_x));
        strings.AppendLine(FloatToStr(camera_up_y));
        strings.AppendLine(FloatToStr(camera_up_z));
        strings.AppendLine(FloatToStr(rotate_x));
        strings.AppendLine(FloatToStr(rotate_y));
        strings.AppendLine(FloatToStr(rotate_z));
        strings.AppendLine(FloatToStr(translate_x));
        strings.AppendLine(FloatToStr(translate_y));
        strings.AppendLine(FloatToStr(translate_z));
        strings.AppendLine(FloatToStr(scale_x));
        strings.AppendLine(FloatToStr(scale_y));
        strings.AppendLine(FloatToStr(scale_z));
        strings.AppendLine(FloatToStr(material_shininess));
        strings.AppendLine(FloatToStr(material_alpha));
        strings.AppendLine(FloatToStr(pointlight1_radius));
        strings.AppendLine(FloatToStr(pointlight1_phi));
        strings.AppendLine(FloatToStr(pointlight1_sita));

        store_matrix4f(trackball, strings);
        store_dirlight(dirlight0, strings);
        store_pointlight(pointlight1, strings);

        return strings.GetText();
    }

    static void load_scene(const string s)
    {
        Strings strings;
        strings.SetText(s);
        int delta = 0;

        data_bound_x_max = strings_getf(strings, delta++);
        data_bound_x_min = strings_getf(strings, delta++);
        data_bound_y_max = strings_getf(strings, delta++);
        data_bound_y_min = strings_getf(strings, delta++);
        data_bound_z_max = strings_getf(strings, delta++);
        data_bound_z_min = strings_getf(strings, delta++);
        max_r = strings_getf(strings, delta++);

        InitialAux initaux;
        initaux.fovy = strings_getf(strings, delta++);
        initaux.target_phi = strings_getf(strings, delta++);
        initaux.target_sita = strings_getf(strings, delta++);
        initaux.camera_pos_x = strings_getf(strings, delta++);
        initaux.camera_pos_y = strings_getf(strings, delta++);
        initaux.camera_pos_z = strings_getf(strings, delta++);
        initaux.camera_up_x = strings_getf(strings, delta++);
        initaux.camera_up_y = strings_getf(strings, delta++);
        initaux.camera_up_z = strings_getf(strings, delta++);
        initaux.rotate_x = strings_getf(strings, delta++);
        initaux.rotate_y = strings_getf(strings, delta++);
        initaux.rotate_z = strings_getf(strings, delta++);
        initaux.translate_x = strings_getf(strings, delta++);
        initaux.translate_y = strings_getf(strings, delta++);
        initaux.translate_z = strings_getf(strings, delta++);
        initaux.scale_x = strings_getf(strings, delta++);
        initaux.scale_y = strings_getf(strings, delta++);
        initaux.scale_z = strings_getf(strings, delta++);
        initaux.material_shininess = strings_getf(strings, delta++);
        initaux.material_alpha = strings_getf(strings, delta++);
        initaux.pointlight1_radius = strings_getf(strings, delta++);
        initaux.pointlight1_phi = strings_getf(strings, delta++);
        initaux.pointlight1_sita = strings_getf(strings, delta++);

        load_matrix4f(strings, delta, initaux.trackball);
        load_dirlight(strings, delta, initaux.dirlight0);
        load_pointlight(strings, delta, initaux.pointlight1);

        reset_space(initaux, false);
    }
}
#endif
