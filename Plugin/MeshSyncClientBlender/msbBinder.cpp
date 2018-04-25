#include "pch.h"
#include "MeshSyncClientBlender.h"
#include "msbUtils.h"
#include "msbBinder.h"

namespace blender
{

static Main *g_data;
static bContext *g_context;


#define Def(T) StructRNA* T::s_type
#define Func(T, F) static FunctionRNA* T##_##F
#define Prop(T, F) static PropertyRNA* T##_##F

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
brange<bDeformGroup> BObject::deform_groups()
{
    return range((bDeformGroup*)m_ptr->defbase.first);
}


barray<MLoop> BMesh::indices()
{
    return { m_ptr->mloop, (size_t)m_ptr->totloop };
}
barray<MPoly> BMesh::polygons()
{
    return { m_ptr->mpoly, (size_t)m_ptr->totpoly };
}

barray<MVert> BMesh::vertices()
{
    return { m_ptr->mvert, (size_t)m_ptr->totvert };
}
barray<float3> BMesh::normals()
{
    if (CustomData_number_of_layers(m_ptr->ldata, CD_NORMAL) > 0) {
        auto data = (float3*)CustomData_get(m_ptr->ldata, CD_NORMAL);
        if (data != nullptr)
            return { data, (size_t)m_ptr->totloop };
    }
    return { nullptr, (size_t)0 };
}
barray<float2> BMesh::uv()
{
    if (CustomData_number_of_layers(m_ptr->ldata, CD_MLOOPUV) > 0) {
        auto data = (float2*)CustomData_get(m_ptr->ldata, CD_MLOOPUV);
        if (data != nullptr) {
            return { data, (size_t)m_ptr->totloop };
        }
    }
    return { nullptr, (size_t)0 };
}
barray<MLoopCol> BMesh::colors()
{
    if (CustomData_number_of_layers(m_ptr->ldata, CD_MLOOPCOL) > 0) {
        auto data = (MLoopCol*)CustomData_get(m_ptr->ldata, CD_MLOOPCOL);
        if (data != nullptr) {
            return { data, (size_t)m_ptr->totloop };
        }
    }
    return { nullptr, (size_t)0 };
}

void BMesh::calc_normals_split()
{
    call<Mesh, void>(m_ptr, BMesh_calc_normals_split, {});
}



barray<BMFace*> BEditMesh::polygons()
{
    return { m_ptr->bm->ftable, (size_t)m_ptr->bm->ftable_tot };
}

barray<BMVert*> BEditMesh::vertices()
{
    return { m_ptr->bm->vtable, (size_t)m_ptr->bm->vtable_tot };
}

barray<BMTriangle> BEditMesh::triangles()
{
    return barray<BMTriangle> { m_ptr->looptris, (size_t)m_ptr->tottri };
}

int BEditMesh::uv_data_offset() const
{
    return CustomData_get_offset(m_ptr->bm->ldata, CD_MLOOPUV);
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

int CustomData_get_offset(const CustomData& data, int type)
{
    int layer_index = data.typemap[type];
    if (layer_index == -1)
        return -1;

    return data.layers[layer_index].offset;
}


float3 BM_loop_calc_face_normal(const BMLoop& l)
{
    float r_normal[3];
    float v1[3], v2[3];
    sub_v3_v3v3(v1, l.prev->v->co, l.v->co);
    sub_v3_v3v3(v2, l.next->v->co, l.v->co);

    cross_v3_v3v3(r_normal, v1, v2);
    const float len = normalize_v3(r_normal);
    if (UNLIKELY(len == 0.0f)) {
        copy_v3_v3(r_normal, l.f->no);
    }
    return (float3&)r_normal;
}

} // namespace blender
