#include "load_m.h"
#include "../commoninclude/stringfuncs.h"

namespace glcommon
{

float mesh_bound_x_max, mesh_bound_x_min, mesh_bound_y_max,
    mesh_bound_y_min, mesh_bound_z_max, mesh_bound_z_min;

void update_mash_bound(const vec3f &pos)
{
    if (pos.x > mesh_bound_x_max)
        mesh_bound_x_max = pos.x;
    else if (pos.x < mesh_bound_x_min)
        mesh_bound_x_min = pos.x;

    if (pos.y > mesh_bound_y_max)
        mesh_bound_y_max = pos.y;
    else if (pos.y < mesh_bound_y_min)
        mesh_bound_y_min = pos.y;

    if (pos.z > mesh_bound_z_max)
        mesh_bound_z_max = pos.z;
    else if (pos.z < mesh_bound_z_min)
        mesh_bound_z_min = pos.z;
}

bool load_m(const string mfile, vector<pos_normal_uv> &vertices,
    vector<unsigned int> &indices, mesh_bound& bound)
{
    map<long, long> vid_2_index;

    cout << "load this one: " << mfile << endl;
    cout << "i am doing its vertices.";
    char buff[1000];
    ::memset(buff, 0, sizeof(buff));
    string line;
    int line_progress = 0;
    int slow_progress = 100;
    ifstream stream(mfile.c_str());
    // deal with Vertics
    bool first = true;
    while (stream.getline(buff, 1000))
    {
        if (buff[0] != 'V') {
            if (buff[0] == 'F')
                break;  // vertex part end, next part is for index
            else
                continue;
        }

        if (line_progress++ % slow_progress == 0)
        {
            cout << ".";
            fflush(stdout);
            if (slow_progress < 20734)
                slow_progress = (int)slow_progress*1.01;
        }

        pos_normal_uv vertex;
        line = buff;
        int offset = FindFirstChar(line.c_str(), ' ');
        // 找到vertex id的起点，后面跟整个rest
        string therest = trim(line.substr(offset+1, line.size()-(offset+1)));
        // 找到vertex id的终点
        offset = FindFirstChar(therest.c_str(), ' ');
        // 得到vertex id
        int vertex_id = StrToInt(therest.substr(0, offset));
        // vertex id => vertex index
        vid_2_index[vertex_id] = vertices.size();

        // 去掉vertex id，得到坐标串以及normal
        therest = trim(therest.substr(offset+1, line.size()-(offset+1)));
        // 去掉后面可能的the rest, 得到坐标串
        offset = FindFirstChar(therest.c_str(), '{');
        if (offset >= 0)
        {
            therest = trim(therest.substr(0, offset));
        }
        // x
        offset = FindFirstChar(therest.c_str(), ' ');
        vertex.pos.x = StrToFloat(therest.substr(0, offset));
        // y
        therest = trim(therest.substr(offset, therest.size()-offset));
        offset = FindFirstChar(therest.c_str(), ' ');
        vertex.pos.y = StrToFloat(therest.substr(0, offset));
        // z
        offset = FindLastChar(therest.c_str(), ' ');
        vertex.pos.z = StrToFloat(therest.substr(offset, therest.size()-offset));

        // retrieve the normal & uv
        offset = FindFirstChar(line.c_str(), '{');
        if (offset >= 0)
        {
            // all stuff inside the {}
            string inside = trim(line.substr(offset+1, line.size()-(offset+1)));
            inside = trim(inside.substr(0, inside.size() - 1));

            // decide which part is what
            string part_normal, part_uv, part_neighbor;
            vector<string> parts = split(inside, ")");
            vector<string>::const_iterator it_part = parts.begin();
            while (it_part != parts.end())
            {
                string part = trim(*it_part);
                if (part.size() > 4 && part[0] == 'u' && part[1] == 'v')
                    part_uv = part + ')';
                else if (part.size() > 4 && part[0] == 'n' && part[1] == 'o')
                    part_normal = part + ')';
                else if (part.size() > 4 && part[0] == 'n' && part[1] == 'e')
                    part_neighbor = part + ')';
                it_part++;
            }

            if (part_normal.size() > 0)
            {
                offset = FindFirstChar(part_normal.c_str(), '(');
                part_normal = trim(part_normal.substr(offset + 1, part_normal.size() - (offset + 1)));
                // x
                offset = FindFirstChar(part_normal.c_str(), ' ');
                vertex.normal.x = StrToFloat(part_normal.substr(0, offset));
                // y
                part_normal = trim(part_normal.substr(offset, part_normal.size() - offset));
                offset = FindFirstChar(part_normal.c_str(), ' ');
                vertex.normal.y = StrToFloat(part_normal.substr(0, offset));
                // z
                part_normal = trim(part_normal.substr(offset, part_normal.size() - offset));
                offset = FindFirstChar(part_normal.c_str(), ')');
                vertex.normal.z = StrToFloat(part_normal.substr(0, offset));
            }

            if (part_uv.size() > 0)
            {
                offset = FindFirstChar(part_uv.c_str(), '(');
                part_uv = trim(part_uv.substr(offset + 1, part_uv.size() - (offset + 1)));
                // u
                offset = FindFirstChar(part_uv.c_str(), ' ');
                vertex.u = StrToFloat(part_uv.substr(0, offset));
                // v
                part_uv = trim(part_uv.substr(offset, part_uv.size() - offset));
                offset = FindFirstChar(part_uv.c_str(), ')');
                vertex.v = StrToFloat(part_uv.substr(0, offset));
            }
        }
        else
        {
            // make things funny
            vertex.normal = vertex.pos;
        }

        vertices.push_back(vertex);
        if (first)
        {
            mesh_bound_x_min = mesh_bound_x_max = vertex.pos.x;
            mesh_bound_y_min = mesh_bound_y_max = vertex.pos.y;
            mesh_bound_z_min = mesh_bound_z_max = vertex.pos.z;
            first = false;
        }
        else
        {
            update_mash_bound(vertex.pos);
        }
    }

    if (buff[0] != 'F')
    {
        cout << "oh boy... no index!" << endl;
        stream.close();
        return false;
    }

    // deal with Faces
    cout << "\nall vertices loaded. now dealing with faces." << endl;
    do
    {
        if (buff[0] != 'F')
        {
            cout << "\nthere are sth between V&F";
            continue;
        }

        if (line_progress++ % slow_progress == 0)
        {
            cout << ".";
            fflush(stdout);
        }

        line = buff;
        int offset = FindFirstChar(line.c_str(), ' ');
        // 找到face index的起点，后面跟整个rest
        string therest = trim(line.substr(offset+1, line.size()-(offset+1)));
        // 找到face index的终点
        offset = FindFirstChar(therest.c_str(), ' ');
        // 去掉face index，得到vertex id 串
        therest = trim(therest.substr(offset+1, line.size()-(offset+1)));

        // a triangle is a triple of vertices

        // get 3 vertex ids
        offset = FindFirstChar(therest.c_str(), ' ');
        int vertex_id0 = StrToFloat(therest.substr(0, offset));
        therest = trim(therest.substr(offset+1, line.size()-(offset+1)));
        offset = FindFirstChar(therest.c_str(), ' ');
        int vertex_id1 = StrToFloat(therest.substr(0, offset));
        therest = trim(therest.substr(offset+1, line.size()-(offset+1)));
        offset = FindFirstChar(therest.c_str(), '{');
        int vertex_id2 = StrToFloat(therest.substr(0, offset));

        // look up the vid_2_index
        map<long, long>::const_iterator it_vertex0 = vid_2_index.find(vertex_id0);
        map<long, long>::const_iterator it_vertex1 = vid_2_index.find(vertex_id1);
        map<long, long>::const_iterator it_vertex2 = vid_2_index.find(vertex_id2);
        if (it_vertex0 != vid_2_index.end()
                && it_vertex1 != vid_2_index.end()
                && it_vertex2 != vid_2_index.end())
        {
            indices.push_back(it_vertex0->second);
            indices.push_back(it_vertex1->second);
            indices.push_back(it_vertex2->second);
        }
        else
        {
            cout << "shit happened... vertex no find" << endl;
        }
    } while (stream.getline(buff, 1000));

    stream.close();
    cout << "\ndone." << endl;
    cout << "vertex#: " << vertices.size() << endl;
    cout << "triangle#: " << indices.size()/3 << endl;
    fflush(stdout);

    bound.mesh_bound_x_max = mesh_bound_x_max; bound.mesh_bound_x_min = mesh_bound_x_min;
    bound.mesh_bound_y_max = mesh_bound_y_max; bound.mesh_bound_y_min = mesh_bound_y_min;
    bound.mesh_bound_z_max = mesh_bound_z_max; bound.mesh_bound_z_min = mesh_bound_z_min;
    return true;
}

#ifdef __GNUC__
bool is_model_existed(string model_dir, string model_filename)
{
    DIR *d = opendir(model_dir.c_str());
    if (d)
    {
        struct dirent *dir;
        while ((dir = readdir(d)) != NULL)
        {
            string filename = dir->d_name;
            if (filename.compare(model_filename) == 0)
            {
                closedir(d);
                return true;
            }
        }
        closedir(d);
    }
    return false;
}

string get_first_model(string model_dir, string model_ext)
{
    DIR *d = opendir(model_dir.c_str());
    if (d)
    {
        struct dirent *dir;
        while ((dir = readdir(d)) != NULL)
        {
            string filename = dir->d_name;
            string ext = LowerString(GetExtNamePart(filename));
            if (ext.compare(model_ext) == 0)
            {
                closedir(d);
                return model_dir + "/" + filename;
            }
        }
        closedir(d);
    }
    return "";
}
#endif

}
