#include "pch.h"
#include "MeshSyncClientBlender.h"
#include "msbUtils.h"
#include "msbBinder.h"

namespace blender
{

static bContext *g_context;


#define Def(T) StructRNA* T::s_type
#define Func(T, F) static FunctionRNA* T##_##F
#define Prop(T, F) static PropertyRNA* T##_##F

Def(BID);
Prop(BID, is_updated);
Prop(BID, is_updated_data);

Def(BObject);
Prop(BObject, matrix_local);
Func(BObject, is_visible);

Def(BMesh);
Func(BMesh, calc_normals_split);

Def(BFCurve);
Func(BFCurve, evaluate);

Def(BMaterial);
Prop(BMaterial, use_nodes);
Prop(BMaterial, active_node_material);

Def(BScene);

Def(BData);

Def(BContext);
Prop(BContext, blend_data);
Prop(BContext, scene);

#undef Prop
#undef Func
#undef Def

PropertyRNA* BlendDataObjects_is_updated;


// context: bpi.context in python
void setup()
{
    if (g_context)
        return;

    py::object local = py::dict();
    py::eval<py::eval_mode::eval_statements>(
"import _bpy as bpi;"
"context = bpi.context;"
, py::object(), local);

    py::object bpy_context = local["context"];

    auto rna = (BPy_StructRNA*)bpy_context.ptr();
    auto first_type = &rna->ptr.type->cont;
    while (first_type->prev) first_type = (ContainerRNA*)first_type->prev;
    rna_sdata(bpy_context, g_context);


    // resolve blender types and functions
#define match_type(N) strcmp(type->identifier, N) == 0
#define match_func(N) strcmp(func->identifier, N) == 0
#define match_prop(N) strcmp(prop->identifier, N) == 0
#define each_func for (auto *func : list_range((FunctionRNA*)type->functions.first))
#define each_prop for (auto *prop : list_range((PropertyRNA*)type->cont.properties.first))

    for (auto *type : list_range((StructRNA*)&first_type)) {
        if (match_type("ID")) {
            BID::s_type = type;
            each_prop{
                if (match_prop("is_updated")) BID_is_updated = prop;
                if (match_prop("is_updated_data")) BID_is_updated_data = prop;
            }
        }
        else if (match_type("Object")) {
            BObject::s_type = type;
            each_prop{
                if (match_prop("matrix_local")) BObject_matrix_local = prop;
            }
            each_func {
                if (match_func("is_visible")) BObject_is_visible = func;
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
            each_prop{
                if (match_prop("use_nodes")) BMaterial_use_nodes = prop;
                if (match_prop("active_node_material")) BMaterial_active_node_material = prop;
            }
        }
        else if (match_type("Scene")) {
            BScene::s_type = type;
        }
        else if (match_type("BlendData")) {
            BData::s_type = type;
        }
        else if (match_type("BlendDataObjects")) {
            each_prop{
                if (match_prop("is_updated")) BlendDataObjects_is_updated = prop;
            }
        }
        else if (match_type("Context")) {
            BData::s_type = type;
            each_prop{
                if (match_prop("blend_data")) BContext_blend_data = prop;
                if (match_prop("scene")) BContext_scene = prop;
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
    ParameterList param_list;
    param_list.data = &params;

    f->call(g_context, nullptr, &ptr, &param_list);
    return params.get();
}

template<typename T, typename U, typename R>
R getter(T *idd, U *d, PropFloatArrayGetFunc f)
{
    PointerRNA ptr;
    ptr.id.data = idd;
    ptr.data = d;

    R r;
    f(&ptr, (float*)&r);
    return r;
}

template<typename T, typename U, typename R>
R getter(T *idd, U *d, PropPointerGetFunc f)
{
    PointerRNA ptr;
    ptr.id.data = idd;
    ptr.data = d;

    PointerRNA ret = f(&ptr);
    return (R)ret.data;
}

template<typename T, typename U>
bool getter(T *idd, U *d, PropBooleanGetFunc f)
{
    PointerRNA ptr;
    ptr.id.data = idd;
    ptr.data = d;

    return f(&ptr) != 0;
}



const char *BID::name() const { return m_ptr->name + 2; }
bool BID::is_updated() const
{
    return getter<nullptr_t, ID>(nullptr, m_ptr, ((BoolPropertyRNA*)BID_is_updated)->get);
}
bool BID::is_updated_data() const
{
    return getter<nullptr_t, ID>(nullptr, m_ptr, ((BoolPropertyRNA*)BID_is_updated_data)->get);
}


const char *BObject::name() const { return ((BID)*this).name(); }
void* BObject::data() { return m_ptr->data; }

float4x4 BObject::matrix_local() const
{
    return getter<Object, nullptr_t, float4x4>(m_ptr, nullptr, ((FloatPropertyRNA*)BObject_matrix_local)->getarray);
}

bool blender::BObject::is_visible(Scene * scene) const
{
    return call<Object, int, Scene*>(m_ptr, BObject_is_visible, { scene }) != 0;
}

blist_range<ModifierData> BObject::modifiers()
{
    return list_range((ModifierData*)m_ptr->modifiers.first);
}
blist_range<FCurve> BObject::fcurves()
{
    if (m_ptr->action) {
        return list_range((FCurve*)m_ptr->action->curves.first);
    }
    return list_range((FCurve*)nullptr);
}
blist_range<bDeformGroup> BObject::deform_groups()
{
    return list_range((bDeformGroup*)m_ptr->defbase.first);
}


barray_range<MLoop> BMesh::indices()
{
    return { m_ptr->mloop, (size_t)m_ptr->totloop };
}
barray_range<MEdge> BMesh::edges()
{
    return { m_ptr->medge, (size_t)m_ptr->totedge };
}
barray_range<MPoly> BMesh::polygons()
{
    return { m_ptr->mpoly, (size_t)m_ptr->totpoly };
}

barray_range<MVert> BMesh::vertices()
{
    return { m_ptr->mvert, (size_t)m_ptr->totvert };
}
barray_range<float3> BMesh::normals()
{
    if (CustomData_number_of_layers(m_ptr->ldata, CD_NORMAL) > 0) {
        auto data = (float3*)CustomData_get(m_ptr->ldata, CD_NORMAL);
        if (data != nullptr)
            return { data, (size_t)m_ptr->totloop };
    }
    return { nullptr, (size_t)0 };
}
barray_range<float2> BMesh::uv()
{
    if (CustomData_number_of_layers(m_ptr->ldata, CD_MLOOPUV) > 0) {
        auto data = (float2*)CustomData_get(m_ptr->ldata, CD_MLOOPUV);
        if (data != nullptr) {
            return { data, (size_t)m_ptr->totloop };
        }
    }
    return { nullptr, (size_t)0 };
}
barray_range<MLoopCol> BMesh::colors()
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



barray_range<BMFace*> BEditMesh::polygons()
{
    return { m_ptr->bm->ftable, (size_t)m_ptr->bm->ftable_tot };
}

barray_range<BMVert*> BEditMesh::vertices()
{
    return { m_ptr->bm->vtable, (size_t)m_ptr->bm->vtable_tot };
}

barray_range<BMTriangle> BEditMesh::triangles()
{
    return barray_range<BMTriangle> { m_ptr->looptris, (size_t)m_ptr->tottri };
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
bool BMaterial::use_nodes() const
{
    return getter<nullptr_t, Material>(nullptr, m_ptr, ((BoolPropertyRNA*)BMaterial_use_nodes)->get);
}
Material * BMaterial::active_node_material() const
{
    return getter<nullptr_t, Material, Material*>(nullptr, m_ptr, ((PointerPropertyRNA*)BMaterial_active_node_material)->get);
}

blist_range<Object> BScene::objects()
{
    return list_range((Object*)m_ptr->base.first);
}


BContext BContext::get()
{
    return BContext(g_context);
}
Main* BContext::data()
{
    return getter<nullptr_t, bContext, Main*>(nullptr, m_ptr, ((PointerPropertyRNA*)BContext_blend_data)->get);
}
Scene* BContext::scene()
{
    return getter<nullptr_t, bContext, Scene*>(nullptr, m_ptr, ((PointerPropertyRNA*)BContext_scene)->get);
}

blist_range<Object> BData::objects()
{
    return list_range((Object*)m_ptr->object.first);
}
blist_range<Material> BData::materials()
{
    return list_range((Material*)m_ptr->mat.first);
}
bool BData::objects_is_updated()
{
    return getter<nullptr_t, Main>(nullptr, m_ptr, ((BoolPropertyRNA*)BlendDataObjects_is_updated)->get);
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
