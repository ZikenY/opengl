//
//  RGObjLoader.h
//  obj-loader
//
//  Created by sbx_fc on 14-10-20.
//  Copyright (c) 2014骞?rungame. All rights reserved.
//

#ifndef __OBJ_LOADER__RGOBJLOADER__
#define __OBJ_LOADER__RGOBJLOADER__

#pragma warning(disable: 4786)
#include <cstdlib>
#include <cstring>
#include <cassert>

#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <sstream>

extern bool mtl_existed;

struct obj_bound
{
    float obj_bound_x_max;
    float obj_bound_x_min;
    float obj_bound_y_max;
    float obj_bound_y_min;
    float obj_bound_z_max;
    float obj_bound_z_min;
};

// 定义每个group的格式:
typedef struct
{
    std::vector<unsigned int>   indices;        //这个组所有顶点信息的索引
    std::vector<float>          positions;      //顶点坐标
    std::vector<float>          normals;        //顶点法线
    std::vector<float>          texcoords;      //顶点的UV坐标
    std::vector<int>            material_ids;   //材质ID的索引对应的材质数组materials,它包含了所有顶点的材质信息，每个顶点通过ID去对应相应的材质信息
} mesh_r;

typedef struct
{
    std::string     name;
    mesh_r          mesh;
} shape_r;

typedef struct
{
    std::string name;

    float ambient[4];       // 环境反射 Ka
    float diffuse[4];       // 漫反射 Kd
    float specular[4];      // 镜面反射 Ks
    float transmittance[4]; // 透明度（滤光透射率） Tf
    float emission[4];
    float shininess;        // 镜面指数
    float ior;
    float dissolve;
    int illum;

    std::string ambient_texname;
    std::string diffuse_texname;
    std::string specular_texname;
    std::string normal_texname;
    std::map<std::string, std::string> unknown_parameter;
} material_r;


class MaterialReader
{
public:
    MaterialReader(){}
    virtual ~MaterialReader(){}
    virtual std::string operator() (const std::string& matId,
        std::vector<material_r>& materials,
        std::map<std::string, int>& matMap
        ) = 0;
};


class MaterialFileReader: public MaterialReader
{
public:
    MaterialFileReader(const std::string& mtl_basepath): m_mtlBasePath(mtl_basepath) {}
    virtual ~MaterialFileReader() {}
    virtual std::string operator()(
        const std::string& matId,
        std::vector<material_r>& materials,
        std::map<std::string,
        int>& matMap
        );

private:
    std::string m_mtlBasePath;
};

std::string LoadObj(
    std::vector<shape_r>& shapes,
    std::vector<material_r>& materials,
    obj_bound &bound,
    const char* filename
    );

std::string LoadObj(
    std::vector<shape_r>& shapes,
    std::vector<material_r>& materials,
    obj_bound &bound,
    std::istream& inStream,
    MaterialReader& readMatFn
    );

#endif // defined(__OBJ_LOADER__RGOBJLOADER__)
