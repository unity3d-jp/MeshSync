#include "pch.h"
#include "MeshSyncClientBlender.h"
#include "msbUtils.h"
#include "msbBinder.h"

namespace blender
{
static bContext *g_context;
static Main *g_data;


#define Def(T) StructRNA* T::s_type
#define Func(T, F) FunctionRNA* T##_##F
#define Prop(T, F) PropertyRNA* T##_##F

Def(BObject);

Def(BMesh);
Func(BMesh, calc_normals_split);

Def(BFCurve);
Func(BFCurve, evaluate);

Def(BMaterial);

Def(BScene);

Def(BContext);
Prop(BContext, scene);

Def(BData);

#undef Prop
#undef Func
#undef Def

// context: bpi.context in python
void setup()
{
    if (g_context)
        return;

    py::object local = py::dict();
    py::eval<py::eval_mode::eval_statements>(
"import _bpy as bpi;"
"context = bpi.context;"
"data = bpi.data;"
, py::object(), local);

    py::object bpy_context = local["context"];
    py::object bpy_data = local["data"];

    auto rna = (BPy_StructRNA*)bpy_context.ptr();
    auto first_type = &rna->ptr.type->cont;
    while (first_type->prev) first_type = (ContainerRNA*)first_type->prev;
    rna_sdata(bpy_context, g_context);
    rna_sdata(bpy_data, g_data);


    // resolve blender types and functions
#define match_type(N) strcmp(type->identifier, N) == 0
#define match_func(N) strcmp(func->identifier, N) == 0
#define match_prop(N) strcmp(prop->identifier, N) == 0
#define each_func for (auto *func : range((FunctionRNA*)type->functions.first))
#define each_iprop for (auto *prop : range((PropertyRNA*)type->iteratorproperty))
#define each_nprop for (auto *prop : range((PropertyRNA*)type->iteratorproperty))

    for (auto *type : range((StructRNA*)&first_type)) {
        if (match_type("Object")) {
            BObject::s_type = type;
            each_func{

            }
        }
        else if (match_type("Mesh")) {
            BMesh::s_type = type;
            each_func {
                if (match_func("calc_normals_split")) BMesh_calc_normals_split = func;
            }
        }
        else if (match_type("FCurve")) {
            BFCurve::s_type = type;
            each_func{
                if (match_func("evaluate")) BFCurve_evaluate = func;
            }
        }
        else if (match_type("Material")) {
            BMaterial::s_type = type;
            each_func{
            }
        }
        else if (match_type("Scene")) {
            BScene::s_type = type;
        }
        else if (match_type("Context")) {
            BData::s_type = type;
            each_nprop{
                if (match_prop("scene")) BContext_scene = prop;
            }
        }
        else if (match_type("BlendData")) {
            BData::s_type = type;
            each_func{
            }
        }
    }
#undef each_iprop
#undef each_nprop
#undef each_func
#undef match_prop
#undef match_func
#undef match_type

    // test
    //auto scene = BContext::get().scene();
}

template<typename A,  typename R>
struct param_holder
{
    A args;
    R ret;
    param_holder(const A& a) : args(a) {}
    R& get() { return ret; }
};

template<typename A>
struct param_holder<A, void>
{
    A args;
    param_holder(const A& a) : args(a) {}
    void get() {}
};

template<typename R>
struct param_holder<std::tuple<>, R>
{
    R ret;
    param_holder(const std::tuple<>&) {}
    R& get() { return ret; }
};

template<>
struct param_holder<std::tuple<>, void>
{
    param_holder(const std::tuple<>&) {}
    void get() {}
};


template<typename T, typename R, typename... A>
R call(T *self, FunctionRNA *f, const std::tuple<A...>& args)
{
    PointerRNA ptr;
    ptr.data = self;

    param_holder<std::tuple<A...>, R> params{args};

    f->call(g_context, nullptr, &ptr, (ParameterList*)&params);
    return params.get();
}


template<typename T, typename R>
R getter(T *self, PropPointerGetFunc f)
{
    PointerRNA ptr;
    ptr.data = self;

    PointerRNA ret = f(&ptr);
    return (R)ret.data;
}

const char *BObject::name() const
{
    return m_ptr->id.name + 2;
}
void* BObject::data()
{
    return m_ptr->data;
}

brange<ModifierData> BObject::modifiers()
{
    return range((ModifierData*)m_ptr->modifiers.first);
}
brange<FCurve> BObject::fcurves()
{
    if (m_ptr->action) {
        return range((FCurve*)m_ptr->action->curves.first);
    }
    return range((FCurve*)nullptr);
}
brange<bDeformGroup> BObject::vertex_groups()
{
    return range((bDeformGroup*)m_ptr->defbase.first);
}


brange<Key> BMesh::keys()
{
    return range(m_ptr->key);
}

void BMesh::calc_normals_split()
{
    call<Mesh, void>(m_ptr, BMesh_calc_normals_split, {});
}


const char *BFCurve::path() const
{
    return m_ptr->rna_path;
}
int BFCurve::array_index() const
{
    return m_ptr->array_index;
}

float BFCurve::evaluate(float time)
{
    return call<FCurve, float, float>(m_ptr, BFCurve_evaluate, {time});
}


const char *BMaterial::name() const
{
    return m_ptr->id.name + 2;
}
const float3& BMaterial::color() const
{
    return (float3&)m_ptr->r;
}

brange<Object> BScene::objects()
{
    return range((Object*)m_ptr->base.first);
}


BContext BContext::get()
{
    return BContext(g_context);
}
BScene BContext::scene()
{
    return getter<bContext, Scene*>(m_ptr, ((PointerPropertyRNA*)BContext_scene)->get);
}

BData BData::get()
{
    return BData(g_data);
}
brange<Object> BData::objects()
{
    return range((Object*)g_data->object.first);
}
brange<Material> BData::materials()
{
    return range((Material*)g_data->mat.first);
}

const void* CustomData_get(const CustomData& data, int type)
{
    int layer_index = data.typemap[type];
    if (layer_index == -1)
        return nullptr;
    layer_index = layer_index + data.layers[layer_index].active;
    return data.layers[layer_index].data;
}

int CustomData_number_of_layers(const CustomData& data, int type)
{
    int i, number = 0;
    for (i = 0; i < data.totlayer; i++)
        if (data.layers[i].type == type)
            number++;
    return number;
}

} // namespace blender
