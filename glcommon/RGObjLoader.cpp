//
//  RGObjLoader.cpp
//  obj-loader
//
//  Created by sbx_fc on 14-10-20.
//  Copyright (c) 2014年 rungame. All rights reserved.
//

#include "RGObjLoader.h"
#include <iostream>

bool mtl_existed = false;

float obj_bound_x_max = 0;
float obj_bound_x_min = 0;
float obj_bound_y_max = 0;
float obj_bound_y_min = 0;
float obj_bound_z_max = 0;
float obj_bound_z_min = 0;

#ifndef max
template <typename T>
inline T max(const T& a, const T& b)
{
    return a < b ? b : a;
}
template short max(const short &a, const short &b);
template int max(const int &a, const int &b);
template float max(const float &a, const float &b);
template double max(const double &a, const double &b);
#endif

void init_mash_bound()
{
    obj_bound_x_max = obj_bound_y_max = obj_bound_z_max = 0;
    obj_bound_x_min = obj_bound_y_min = obj_bound_z_min = 0;
}

void update_mash_bound(const float *vertex)
{
    if (vertex[0] > obj_bound_x_max)
        obj_bound_x_max = vertex[0];
    else if (vertex[0] < obj_bound_x_min)
        obj_bound_x_min = vertex[0];

    if (vertex[1] > obj_bound_y_max)
        obj_bound_y_max = vertex[1];
    else if (vertex[1] < obj_bound_y_min)
        obj_bound_y_min = vertex[1];

    if (vertex[2] > obj_bound_z_max)
        obj_bound_z_max = vertex[2];
    else if (vertex[2] < obj_bound_z_min)
        obj_bound_z_min = vertex[2];
}

struct vertex_index
{
    int v_idx, vt_idx, vn_idx;
    vertex_index() {}
    vertex_index(int idx) : v_idx(idx), vt_idx(idx), vn_idx(idx) {}
    vertex_index(int vidx, int vtidx, int vnidx) : v_idx(vidx), vt_idx(vtidx), vn_idx(vnidx) {}
};

static inline bool operator<(const vertex_index& a, const vertex_index& b)
{
    if (a.v_idx != b.v_idx)
        return (a.v_idx < b.v_idx);

    if (a.vn_idx != b.vn_idx)
        return (a.vn_idx < b.vn_idx);

    if (a.vt_idx != b.vt_idx)
        return (a.vt_idx < b.vt_idx);
    
    return false;
}

static inline bool isNewLine(const char c)
{
    return (c == '\r') || (c == '\n') || (c == '\0');
}

//是否为空格
static inline bool isSpace(const char c)
{
    return (c == ' ') || (c == '\t');
}

static inline std::string parseString(const char*& token)
{
    std::string s;
    int b = strspn(token, " \t");
    int e = strcspn(token, " \t\r");
    s = std::string(&token[b], &token[e]);
    
    token += (e - b);
    return s;
}

static inline int parseInt(const char*& token)
{
    token += strspn(token, " \t");
    int i = atoi(token);
    token += strcspn(token, " \t\r");
    return i;
}

//解析为float类型的值
static inline float parseFloat(const char*& token)
{
    token += strspn(token, " \t");
    float f = (float)atof(token);
    token += strcspn(token, " \t\r");
    return f;
}

static inline void parseFloat2(float& x, float& y,const char*& token)
{
    x = parseFloat(token);
    y = parseFloat(token);
}

static inline void parseFloat3(float& x, float& y, float& z,const char*& token)
{
    x = parseFloat(token);
    y = parseFloat(token);
    z = parseFloat(token);
}

// Make index zero-base, and also support relative index.
static inline int fixIndex(int idx, int n)
{
    int i;
    
    if (idx > 0)
    {
        i = idx - 1;
    }
    else if (idx == 0)
    {
        i = 0;
    }
    else
    { // negative value = relative
        i = n + idx;
    }

    return i;
}

//解析面信息 i, i/j/k, i//k, i/j
static vertex_index parseTriple(const char* &token,int vsize,int vnsize,int vtsize)
{
    vertex_index vi(-1);
    
    vi.v_idx = fixIndex(atoi(token), vsize);
    token += strcspn(token, "/ \t\r");
    if (token[0] != '/')
    {
        return vi;
    }

    token++;
    
    // i//k
    if (token[0] == '/')
    {
        token++;
        vi.vn_idx = fixIndex(atoi(token), vnsize);
        token += strcspn(token, "/ \t\r");
        return vi;
    }
    
    // i/j/k or i/j
    vi.vt_idx = fixIndex(atoi(token), vtsize);
    token += strcspn(token, "/ \t\r");
    if (token[0] != '/')
    {
        return vi;
    }
    
    // i/j/k
    token++;  // skip '/'
    vi.vn_idx = fixIndex(atoi(token), vnsize);
    token += strcspn(token, "/ \t\r");
    return vi;
}

static unsigned int updateVertex(
    std::map<vertex_index, unsigned int>& vertexCache,
    std::vector<float>& positions,
    std::vector<float>& normals,
    std::vector<float>& texcoords,
    const std::vector<float>& in_positions,
    const std::vector<float>& in_normals,
    const std::vector<float>& in_texcoords,
    const vertex_index& vertex)
{
    const std::map<vertex_index,unsigned int>::iterator it = vertexCache.find(vertex);

    if (it != vertexCache.end())
    {
        // found cache
        return it->second;
    }

    assert(in_positions.size() > (unsigned int)(3*vertex.v_idx+2));

    positions.push_back(in_positions[3*vertex.v_idx+0]);
    positions.push_back(in_positions[3*vertex.v_idx+1]);
    positions.push_back(in_positions[3*vertex.v_idx+2]);

    // update MBR
    update_mash_bound(&in_positions[3*vertex.v_idx+0]);
    update_mash_bound(&in_positions[3*vertex.v_idx+1]);
    update_mash_bound(&in_positions[3*vertex.v_idx+2]);

    if (vertex.vn_idx >= 0)
    {
        normals.push_back(in_normals[3*vertex.vn_idx+0]);
        normals.push_back(in_normals[3*vertex.vn_idx+1]);
        normals.push_back(in_normals[3*vertex.vn_idx+2]);
    }

    if (vertex.vt_idx >= 0)
    {
        texcoords.push_back(in_texcoords[2*vertex.vt_idx+0]);
        texcoords.push_back(in_texcoords[2*vertex.vt_idx+1]);
    }

    unsigned int idx = positions.size() / 3 - 1;
    vertexCache[vertex] = idx;

    return idx;
}



// 导出多边形
static bool exportFaceGroupToShape(
    shape_r& shape,
    std::map<vertex_index, unsigned int> vertexCache,
    const std::vector<float> &in_positions,
    const std::vector<float> &in_normals,
    const std::vector<float> &in_texcoords,
    const std::vector<std::vector<vertex_index> >& faceGroup,
    const int material_id,
    const std::string &name,
    bool clearCache)
{
    if (faceGroup.empty())
    {
        return false;
    }

    size_t offset;

    //每个面的顶点数
    offset = shape.mesh.indices.size();

    for (size_t i = 0; i < faceGroup.size(); i++)
    {
        //每个面可能含有若干个顶点,将它们分拆成三角形
        const std::vector<vertex_index>& face = faceGroup[i];

        vertex_index i0 = face[0];
        vertex_index i1(-1);
        vertex_index i2 = face[1];

        size_t npolys = face.size();
        //将多边形转化为以i0为顶点的扇形
        for (size_t k = 2; k < npolys; k++)
        {
            i1 = i2;
            i2 = face[k];
            
            unsigned int v0 = updateVertex(vertexCache,shape.mesh.positions, shape.mesh.normals, shape.mesh.texcoords, in_positions, in_normals, in_texcoords, i0);
            unsigned int v1 = updateVertex(vertexCache,shape.mesh.positions, shape.mesh.normals, shape.mesh.texcoords, in_positions, in_normals, in_texcoords, i1);
            unsigned int v2 = updateVertex(vertexCache,shape.mesh.positions, shape.mesh.normals, shape.mesh.texcoords, in_positions, in_normals, in_texcoords, i2);

            shape.mesh.indices.push_back(v0);
            shape.mesh.indices.push_back(v1);
            shape.mesh.indices.push_back(v2);

            shape.mesh.material_ids.push_back(material_id);
        }
    }

    if (clearCache)
    {
        vertexCache.clear();
    }

    return true;
}

void InitMaterial(material_r& material)
{
    material.name = "";
    material.ambient_texname = "";
    material.diffuse_texname = "";
    material.specular_texname = "";
    material.normal_texname = "";
    for (int i = 0; i < 3; i ++)
    {
        material.ambient[i] = 0.f;
        material.diffuse[i] = 0.f;
        material.specular[i] = 0.f;
        material.transmittance[i] = 0.f;
        material.emission[i] = 0.f;
    }

    material.ambient[3] = 1.0f;
    material.diffuse[3] = 1.0f;
    material.specular[3] = 1.0f;
    material.transmittance[3] = 1.0f;
    material.emission[3] = 1.0f;

    material.illum = 0;
    material.dissolve = 1.f;
    material.shininess = 1.f;
    material.ior = 1.f;
    material.unknown_parameter.clear();
}

std::string LoadMtl(
    std::map<std::string, int>& material_map,
    std::vector<material_r>& materials,
    std::istream& inStream)
{
    material_map.clear();
    std::stringstream err;

    material_r material;

    int maxchars = 8192;
    std::vector<char> buf(maxchars);  //数组初始化加载足够长度,避免反复每次增加长度时的内存赋值
    while (inStream.peek() != -1)
    {
        inStream.getline(&buf[0], maxchars);

        std::string linebuf(&buf[0]);

        // Trim newline '\r\n' or '\n'
        if (linebuf.size() > 0)
        {
            if (linebuf[linebuf.size()-1] == '\n') linebuf.erase(linebuf.size()-1);
        }

        if (linebuf.size() > 0)
        {
            if (linebuf[linebuf.size()-1] == '\r') linebuf.erase(linebuf.size()-1);
        }

        // Skip if empty line.
        if (linebuf.empty())
        {
            continue;
        }

        // Skip leading space.
        const char* token = linebuf.c_str();
        token += strspn(token, " \t");

        assert(token);
        if (token[0] == '\0')
            continue; // empty line
        
        if (token[0] == '#')
            continue;  // comment line

        // new mtl
        if ((0 == strncmp(token, "newmtl", 6)) && isSpace((token[6])))
        {
            // flush previous material.
            if (!material.name.empty())
            {
                material_map.insert(std::pair<std::string, int>(material.name, materials.size()));
                materials.push_back(material);
            }

            //初始化一个临时材质
            InitMaterial(material);

            // set new mtl name
            char namebuf[4096];
            token += 7;
            sscanf(token, "%s", namebuf);
            material.name = namebuf;
            continue;
        }

        //环境光
        if (token[0] == 'K' && token[1] == 'a' && isSpace((token[2])))
        {
            token += 2;
            float r, g, b;
            parseFloat3(r, g, b, token);
            material.ambient[0] = r;
            material.ambient[1] = g;
            material.ambient[2] = b;
            continue;
        }

        //漫反射光
        if (token[0] == 'K' && token[1] == 'd' && isSpace((token[2])))
        {
            token += 2;
            float r, g, b;
            parseFloat3(r, g, b, token);
            material.diffuse[0] = r;
            material.diffuse[1] = g;
            material.diffuse[2] = b;
            continue;
        }

        //光泽
        if (token[0] == 'K' && token[1] == 's' && isSpace((token[2])))
        {
            token += 2;
            float r, g, b;
            parseFloat3(r, g, b, token);
            material.specular[0] = r;
            material.specular[1] = g;
            material.specular[2] = b;
            continue;
        }

        //透光度
        if (token[0] == 'K' && token[1] == 't' && isSpace((token[2])))
        {
            token += 2;
            float r, g, b;
            parseFloat3(r, g, b, token);
            material.transmittance[0] = r;
            material.transmittance[1] = g;
            material.transmittance[2] = b;
            continue;
        }

        // ior(index of refraction)
        if (token[0] == 'N' && token[1] == 'i' && isSpace((token[2])))
        {
            token += 2;
            material.ior = parseFloat(token);
            continue;
        }

        // emission
        if(token[0] == 'K' && token[1] == 'e' && isSpace(token[2]))
        {
            token += 2;
            float r, g, b;
            parseFloat3(r, g, b, token);
            material.emission[0] = r;
            material.emission[1] = g;
            material.emission[2] = b;
            continue;
        }

        // shininess
        if(token[0] == 'N' && token[1] == 's' && isSpace(token[2]))
        {
            token += 2;
            material.shininess = parseFloat(token);
            continue;
        }

        // illum model
        if (0 == strncmp(token, "illum", 5) && isSpace(token[5]))
        {
            token += 6;
            material.illum = parseInt(token);
            continue;
        }

        // dissolve
        if ((token[0] == 'd' && isSpace(token[1])))
        {
            token += 1;
            material.dissolve = parseFloat(token);
            continue;
        }

        if (token[0] == 'T' && token[1] == 'r' && isSpace(token[2]))
        {
            token += 2;
            material.dissolve = parseFloat(token);
            continue;
        }

        // ambient texture
        if ((0 == strncmp(token, "map_Ka", 6)) && isSpace(token[6]))
        {
            token += 7;
            material.ambient_texname = token;
            continue;
        }

        // diffuse texture
        if ((0 == strncmp(token, "map_Kd", 6)) && isSpace(token[6]))
        {
            token += 7;
            material.diffuse_texname = token;
            continue;
        }

        // specular texture
        if ((0 == strncmp(token, "map_Ks", 6)) && isSpace(token[6]))
        {
            token += 7;
            material.specular_texname = token;
            continue;
        }

        // normal texture
        if ((0 == strncmp(token, "map_Ns", 6)) && isSpace(token[6]))
        {
            token += 7;
            material.normal_texname = token;
            continue;
        }
        
        // unknown parameter
        const char* _space = strchr(token, ' ');
        if(!_space)
        {
            _space = strchr(token, '\t');
        }
        if(_space)
        {
            int len = _space - token;
            std::string key(token, len);
            std::string value = _space + 1;
            material.unknown_parameter.insert(std::pair<std::string, std::string>(key, value));
        }
    }

    // flush last material.
    material_map.insert(std::pair<std::string, int>(material.name, materials.size()));
    materials.push_back(material);

    return err.str();
}

std::string MaterialFileReader::operator() (
        const std::string& matId,
        std::vector<material_r>& materials,
        std::map<std::string, int>& matMap)
{
    std::string filepath;

    if (!m_mtlBasePath.empty())
    {
        filepath = std::string(m_mtlBasePath) + matId;
    }
    else
    {
        filepath = matId;
    }

    std::ifstream matIStream(filepath.c_str());

    mtl_existed = false;
    if (matIStream)
        mtl_existed = true;

    return LoadMtl(matMap, materials, matIStream);
}

static long FindLastChar(const char* pc, const char find)
{
    char* p = (char*)pc;
    long offset = -1;
    while (0 != *p)
    {
        if (find == *p) offset = p - pc;
        p++;
    }

    return offset;
}

static std::string GetDirectoryPart(const std::string& pathfilename)
{
    if (pathfilename.size() == 0)
        return "";

    long offset0 = FindLastChar(pathfilename.c_str(), '\\');
    long offset1 = FindLastChar(pathfilename.c_str(), '/');
    long offset = max(offset0, offset1);
    if (offset < 0)
    {
        return "";
    }
    else if (offset == 0)
    {
        return "/";
    }

    return std::string(pathfilename.substr(0, offset));
}


std::string LoadObj(
    std::vector<shape_r>& shapes,
    std::vector<material_r>& materials,   // [output]
    obj_bound &bound,
    const char* filename)
{
    shapes.clear();

    std::stringstream err;
    std::ifstream ifs(filename);
    if(!ifs)
    {
        err << "Cannot open file [" << filename << "]" << std::endl;
        return err.str();
    }

    std::string basePath = GetDirectoryPart(filename);
    if (basePath != "")
    {
        basePath = GetDirectoryPart(filename) + "/";
    }

    MaterialFileReader matFileReader(basePath);
    
    return LoadObj(shapes, materials, bound, ifs, matFileReader);
}

std::string LoadObj(
    std::vector<shape_r>& shapes,
    std::vector<material_r>& materials,
    obj_bound &bound,
    std::istream& inStream,
    MaterialReader& readMatFn)
{
    init_mash_bound();

    std::stringstream err;

    std::vector<float> v;
    std::vector<float> vn;
    std::vector<float> vt;
    std::vector< std::vector<vertex_index> > faceGroup;
    std::string name;

    std::map<std::string, int> material_map;
    std::map<vertex_index, unsigned int> vertexCache;
    int  material = -1;

    shape_r shape;

    int maxchars = 8192;
    std::vector<char> buf(maxchars);

    //读取下一个字符不为空
    while(inStream.peek() != -1)
    {
        inStream.getline(&buf[0], maxchars);

        std::string linebuf(&buf[0]);

        //去除换行符标识 '\r\n' or '\n'
        if (linebuf.size() > 0)
        {
            if (linebuf[linebuf.size()-1] == '\n') linebuf.erase(linebuf.size()-1);
        }
        if (linebuf.size() > 0)
        {
            if (linebuf[linebuf.size()-1] == '\r') linebuf.erase(linebuf.size()-1);
        }

        //若本行为空跳出
        if (linebuf.empty())
        {
            continue;
        }

        //跳过开头的空白部分
        const char* token = linebuf.c_str();
        token += strspn(token, " \t");

        assert(token);
        if (token[0] == '\0')
            continue; //遇到行结束符跳出。
        
        if (token[0] == '#')
            continue;  //遇到注释符，说明这一行结束，跳出。

        //解析顶点
        //如果第一个点是'v',并且接下来一个字符是空格
        if (token[0] == 'v' && isSpace((token[1])))
        {
            //直接跳至第三个字符
            token += 2;
            float x, y, z;
            parseFloat3(x, y, z, token);
            v.push_back(x);
            v.push_back(y);
            v.push_back(z);
            continue;
        }

        //解析顶点法线 vn
        if (token[0] == 'v' && token[1] == 'n' && isSpace((token[2])))
        {
            token += 3;
            float x, y, z;
            parseFloat3(x, y, z, token);
            vn.push_back(x);
            vn.push_back(y);
            vn.push_back(z);
            continue;
        }

        //解析UV坐标 vt
        if (token[0] == 'v' && token[1] == 't' && isSpace((token[2])))
        {
            token += 3;
            float x, y;
            parseFloat2(x, y, token);
            vt.push_back(x);
            vt.push_back(y);
            continue;
        }

        //解析面信息
        if (token[0] == 'f' && isSpace((token[1])))
        {
            token += 2;
            token += strspn(token, " \t");
            std::vector<vertex_index> face;
            while (!isNewLine(token[0]))
            {
                vertex_index vi = parseTriple(token, v.size() / 3, vn.size() / 3, vt.size() / 2);
                face.push_back(vi);
                size_t n = strspn(token, " \t\r");
                token += n;
            }

            faceGroup.push_back(face);
            continue;
        }

        //使用材质名称
        if ((0 == strncmp(token, "usemtl", 6)) && isSpace((token[6])))
        {
            char namebuf[4096];
            token += 7;
            sscanf(token, "%s", namebuf);

            bool ret = exportFaceGroupToShape(shape, vertexCache, v, vn, vt, faceGroup, material, name, false);
            faceGroup.clear();

            if (material_map.find(namebuf) != material_map.end())
            {
                material = material_map[namebuf];
            } else
            {
                // { error!! material not found }
                material = -1;
            }

            continue;
        }

        //是否使用材质库
        if ((0 == strncmp(token, "mtllib", 6)) && isSpace((token[6])))
        {
            char namebuf[4096];
            token += 7;
            sscanf(token, "%s", namebuf);

            std::string err_mtl = readMatFn(namebuf, materials, material_map);
            if (!err_mtl.empty())
            {
                faceGroup.clear();  // for safety
                return err_mtl;
            }

            continue;
        }

        // group name
        if (token[0] == 'g' && isSpace((token[1])))
        {
            // flush previous face group.
            bool ret = exportFaceGroupToShape(shape, vertexCache, v, vn, vt, faceGroup, material, name, true);
            if (ret)
            {
                shapes.push_back(shape);
            }

            shape = shape_r();

            //material = -1;
            faceGroup.clear();

            std::vector<std::string> names;
            while (!isNewLine(token[0]))
            {
                std::string str = parseString(token);
                names.push_back(str);
                token += strspn(token, " \t\r"); // skip tag
            }

            assert(names.size() > 0);

            // names[0] must be 'g', so skipt 0th element.
            if (names.size() > 1)
            {
                name = names[1];
            }
            else
            {
                name = "";
            }

            continue;
        }

        // object name
        if (token[0] == 'o' && isSpace((token[1])))
        {
            // flush previous face group.
            bool ret = exportFaceGroupToShape(shape, vertexCache, v, vn, vt, faceGroup, material, name, true);
            if (ret)
            {
                shapes.push_back(shape);
            }

            //material = -1;
            faceGroup.clear();
            shape = shape_r();

            // @todo { multiple object name? }
            char namebuf[4096];
            token += 2;
            sscanf(token, "%s", namebuf);
            name = std::string(namebuf);

            continue;
        }
    }

    //导出多边形
    bool ret = exportFaceGroupToShape(shape,vertexCache,v, vn, vt, faceGroup,material, name, true);
    if (ret)
    {
        shapes.push_back(shape);
    }

    faceGroup.clear();
    bound.obj_bound_x_max = obj_bound_x_max;
    bound.obj_bound_x_min = obj_bound_x_min;
    bound.obj_bound_y_max = obj_bound_y_max;
    bound.obj_bound_y_min = obj_bound_y_min;
    bound.obj_bound_z_max = obj_bound_z_max;
    bound.obj_bound_z_min = obj_bound_z_min;
    return err.str();
}
