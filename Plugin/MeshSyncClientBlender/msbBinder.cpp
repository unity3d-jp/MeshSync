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
static PropertyRNA* UVLoopLayers_active;
static PropertyRNA* LoopColors_active;

Def(BMaterial);
Prop(BMaterial, use_nodes);
Prop(BMaterial, active_node_material);

Def(BCamera);
Prop(BCamera, angle_x);
Prop(BCamera, angle_y);

Def(BScene);
Prop(BScene, frame_start);
Prop(BScene, frame_end);
Prop(BScene, frame_current);
Func(BScene, frame_set);

Def(BData);

Def(BContext);
Prop(BContext, blend_data);
Prop(BContext, scene);

#undef Prop
#undef Func
#undef Def

PropertyRNA* BlendDataObjects_is_updated;

bool ready()
{
    return g_context != nullptr;
}

// context: bpi.context in python
void setup(py::object bpy_context)
{
    if (g_context)
        return;

    auto rna = (BPy_StructRNA*)bpy_context.ptr();
    if (strcmp(rna->ob_base.ob_type->tp_name, "Context") != 0) {
        return;
    }

    auto first_type = (StructRNA*)&rna->ptr.type->cont;
    while (first_type->cont.prev) {
        first_type = (StructRNA*)first_type->cont.prev;
    }
    rna_sdata(bpy_context, g_context);

    // resolve blender types and functions
#define match_type(N) strcmp(type->identifier, N) == 0
#define match_func(N) strcmp(func->identifier, N) == 0
#define match_prop(N) strcmp(prop->identifier, N) == 0
#define each_func for (auto *func : list_range((FunctionRNA*)type->functions.first))
#define each_prop for (auto *prop : list_range((PropertyRNA*)type->cont.properties.first))

    for (auto *type : list_range((StructRNA*)first_type)) {
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
        else if (match_type("UVLoopLayers")) {
            each_prop{
                if (match_prop("active")) UVLoopLayers_active = prop;
            }
        }
        else if (match_type("LoopColors")) {
            each_prop{
                if (match_prop("active")) LoopColors_active = prop;
            }
        }
        else if (match_type("Camera")) {
            BCamera::s_type = type;
            each_prop{
                if (match_prop("angle_x")) BCamera_angle_x = prop;
                if (match_prop("angle_y")) BCamera_angle_y = prop;
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
            each_prop{
                if (match_prop("frame_start")) BScene_frame_start = prop;
                if (match_prop("frame_end")) BScene_frame_end = prop;
                if (match_prop("frame_current")) BScene_frame_current = prop;
            }
            each_func{
                if (match_func("frame_set")) BScene_frame_set = func;
            }
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

template<class R>
struct ret_holder
{
    using ret_t = R & ;
    R r;
    R& get() { return r; }
};
template<>
struct ret_holder<void>
{
    using ret_t = void;
    void get() {}
};

template<typename R>
struct param_holder0
{
    ret_holder<R> ret;
    typename ret_holder<R>::ret_t get() { return ret.get(); }
};
template<typename R, typename A1>
struct param_holder1
{
    A1 a1;
    ret_holder<R> ret;
    typename ret_holder<R>::ret_t get() { return ret.get(); }
};
template<typename R, typename A1, typename A2>
struct param_holder2
{
    A1 a1;
    A2 a2;
    ret_holder<R> ret;
    typename ret_holder<R>::ret_t get() { return ret.get(); }
};


template<typename T, typename R>
R call(T *self, FunctionRNA *f)
{
    PointerRNA ptr;
    ptr.data = self;

    param_holder0<R> params;
    ParameterList param_list;
    param_list.data = &params;

    f->call(g_context, nullptr, &ptr, &param_list);
    return params.get();
}
template<typename T, typename R, typename A1>
R call(T *self, FunctionRNA *f, const A1& a1)
{
    PointerRNA ptr;
    ptr.data = self;

    param_holder1<R, A1> params = { a1 };
    ParameterList param_list;
    param_list.data = &params;

    f->call(g_context, nullptr, &ptr, &param_list);
    return params.get();
}
template<typename T, typename R, typename A1, typename A2>
R call(T *self, FunctionRNA *f, const A1& a1, const A2& a2)
{
    PointerRNA ptr;
    ptr.data = self;

    param_holder2<R, A1, A2> params = { a1, a2 };
    ParameterList param_list;
    param_list.data = &params;

    f->call(g_context, nullptr, &ptr, &param_list);
    return params.get();
}

template<typename Self>
static inline bool get_bool(Self *self, PropBooleanGetFunc f)
{
    PointerRNA ptr;
    ptr.id.data = ptr.data = self;

    return f(&ptr) != 0;
}
template<typename Self>
static inline int get_int(Self *self, PropIntGetFunc f)
{
    PointerRNA ptr;
    ptr.id.data = ptr.data = self;

    return f(&ptr);
}
template<typename Self>
static inline float get_float(Self *self, PropFloatGetFunc f)
{
    PointerRNA ptr;
    ptr.id.data = ptr.data = self;

    return f(&ptr);
}
template<typename Self>
static inline void get_float_array(Self *self, float *dst, PropFloatArrayGetFunc f)
{
    PointerRNA ptr;
    ptr.id.data = ptr.data = self;

    f(&ptr, dst);
}
template<typename Self>
static inline void* get_pointer(Self *self, PropPointerGetFunc f)
{
    PointerRNA ptr;
    ptr.id.data = ptr.data = self;

    PointerRNA ret = f(&ptr);
    return ret.data;
}



const char *BID::name() const { return m_ptr->name + 2; }
bool BID::is_updated() const
{
    return get_bool(m_ptr, ((BoolPropertyRNA*)BID_is_updated)->get);
}
bool BID::is_updated_data() const
{
    return get_bool(m_ptr, ((BoolPropertyRNA*)BID_is_updated_data)->get);
}


const char *BObject::name() const { return ((BID)*this).name(); }
void* BObject::data() { return m_ptr->data; }

float4x4 BObject::matrix_local() const
{
    float4x4 ret;
    get_float_array(m_ptr, (float*)&ret, ((FloatPropertyRNA*)BObject_matrix_local)->getarray);
    return ret;
}

bool BObject::is_visible(Scene * scene) const
{
    return call<Object, int, Scene*>(m_ptr, BObject_is_visible, { scene }) != 0;
}

blist_range<ModifierData> BObject::modifiers()
{
    return list_range((ModifierData*)m_ptr->modifiers.first);
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
barray_range<MLoopUV> BMesh::uv()
{
    auto layer_data = (CustomDataLayer*)get_pointer(m_ptr, ((PointerPropertyRNA*)UVLoopLayers_active)->get);
    if (layer_data && layer_data->data)
        return { (MLoopUV*)layer_data->data, (size_t)m_ptr->totloop };
    else
        return { nullptr, (size_t)0 };
}
barray_range<MLoopCol> BMesh::colors()
{
    auto layer_data = (CustomDataLayer*)get_pointer(m_ptr, ((PointerPropertyRNA*)LoopColors_active)->get);
    if (layer_data && layer_data->data)
        return { (MLoopCol*)layer_data->data, (size_t)m_ptr->totloop };
    else
        return { nullptr, (size_t)0 };
}

void BMesh::calc_normals_split()
{
    call<Mesh, void>(m_ptr, BMesh_calc_normals_split);
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
    return get_bool(m_ptr, ((BoolPropertyRNA*)BMaterial_use_nodes)->get);
}
Material * BMaterial::active_node_material() const
{
    return (Material*)get_pointer(m_ptr, ((PointerPropertyRNA*)BMaterial_active_node_material)->get);
}

float BCamera::fov_vertical() const
{
    return get_float(m_ptr, ((FloatPropertyRNA*)BCamera_angle_y)->get);
}

float blender::BCamera::fov_horizontal() const
{
    return get_float(m_ptr, ((FloatPropertyRNA*)BCamera_angle_x)->get);
}

blist_range<Base> BScene::objects()
{
    return list_range((Base*)m_ptr->base.first);
}

int BScene::fps()
{
    return m_ptr->r.frs_sec;
}

int BScene::frame_start()
{
    return get_int(m_ptr, ((IntPropertyRNA*)BScene_frame_start)->get);
}

int BScene::frame_end()
{
    return get_int(m_ptr, ((IntPropertyRNA*)BScene_frame_end)->get);
}

int BScene::frame_current()
{
    return get_int(m_ptr, ((IntPropertyRNA*)BScene_frame_current)->get);
}

void BScene::frame_set(int f, float subf)
{
    call<Scene, void, int, float>(m_ptr, BScene_frame_set, f, subf);
}


BContext BContext::get()
{
    return BContext(g_context);
}
Main* BContext::data()
{
    return (Main*)get_pointer(m_ptr, ((PointerPropertyRNA*)BContext_blend_data)->get);
}
Scene* BContext::scene()
{
    return (Scene*)get_pointer(m_ptr, ((PointerPropertyRNA*)BContext_scene)->get);
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
    return get_bool(m_ptr, ((BoolPropertyRNA*)BlendDataObjects_is_updated)->get);
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
