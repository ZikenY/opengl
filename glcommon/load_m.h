#include "../commoninclude/commoninclude.h"
using namespace ziken;
using namespace std;

#include "matrix4f.h"
namespace glcommon
{

extern bool load_m(
        const string mfile,
        vector<pos_normal_uv> &vertices,   //position + normal + uv with 8 floats in total for each vertex
        vector<unsigned int> &indices,
        mesh_bound& bound
        );

extern bool is_model_existed(string model_dir, string model_filename);
extern string get_first_model(string model_dir, string model_ext);

}
