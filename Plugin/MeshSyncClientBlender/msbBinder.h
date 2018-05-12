#pragma once

namespace blender
{
    void setup();
    const void* CustomData_get(const CustomData& data, int type);
    int CustomData_number_of_layers(const CustomData& data, int type);
    int CustomData_get_offset(const CustomData& data, int type);
    float3 BM_loop_calc_face_normal(const BMLoop& l);


    struct ListHeader { ListHeader *next, *prev; };
    template<typename T> inline T rna_data(py::object p) { return (T)((BPy_StructRNA*)p.ptr())->ptr.id.data; }
    template<typename T> inline void rna_data(py::object p, T& v) { v = (T)((BPy_StructRNA*)p.ptr())->ptr.id.data; }
    template<typename T> inline T rna_sdata(py::object p) { return (T)((BPy_StructRNA*)p.ptr())->ptr.data; }
    template<typename T> inline void rna_sdata(py::object p, T& v) { v = (T)((BPy_StructRNA*)p.ptr())->ptr.data; }

    template<typename T>
    struct blist_iterator
    {
        T *m_ptr;

        blist_iterator(T *p) : m_ptr(p) {}
        T* operator*() const { return m_ptr; }
        T* operator++() { m_ptr = (T*)(((ListHeader*)m_ptr)->next); return m_ptr; }
        bool operator==(const blist_iterator& v) const { return m_ptr == v.m_ptr; }
        bool operator!=(const blist_iterator& v) const { return m_ptr != v.m_ptr; }
    };

    template<typename T>
    struct blist_range
    {
        using iterator = blist_iterator<T>;
        using const_iterator = blist_iterator<const T>;
        T *ptr;

        blist_range(T *p) : ptr(p) {}
        iterator begin() { return iterator(ptr); }
        iterator end() { return iterator(nullptr); }
        const_iterator begin() const { return const_iterator(ptr); }
        const_iterator end() const { return const_iterator(nullptr); }
    };
    template<typename T> blist_range<T> list_range(T *t) { return blist_range<T>(t); }

    template<typename T>
    struct barray_range
    {
        using iterator = T * ;
        using const_iterator = const T * ;
        using reference = T & ;
        using const_reference = const T & ;

        T *m_ptr;
        size_t m_size;

        barray_range(T *p, size_t s) : m_ptr(p), m_size(s) {}
        iterator begin() { return m_ptr; }
        iterator end() { return m_ptr + m_size; }
        const_iterator begin() const { return m_ptr; }
        const_iterator end() const { return m_ptr + m_size; }
        reference operator[](size_t i) { return m_ptr[i]; }
        const_reference operator[](size_t i) const { return m_ptr[i]; }
        size_t size() const { return m_size; }
        bool empty() const { return !m_ptr || m_size == 0; }
    };
    template<typename T> barray_range<T> array_range(T *t, size_t s) { return barray_range<T>(t, s); }



#define Boilerplate2(Type, BType)\
    using btype = ::BType;\
    static StructRNA *s_type;\
    ::BType *m_ptr;\
    static StructRNA* type() { return s_type; }\
    Type(const void *p) : m_ptr((::BType*)p) {}\
    Type(py::object p) : m_ptr(rna_data<::BType*>(p)) {}\
    ::BType* ptr() {return m_ptr; }

#define Boilerplate(Type) Boilerplate2(B##Type, Type)

#define Compatible(Type)\
    operator Type() { return *(Type*)this; }\
    operator Type() const { return *(const Type*)this; }

    struct TypeCode
    {
        uint16_t code;

        TypeCode() : code(0) {}
        TypeCode(char *s) : code(*(uint16_t*)s) {}
        TypeCode(char a, char b) { auto *p = (char*)&code; p[0] = a; p[1] = b; }
        TypeCode(int v) : code((uint16_t)v) {}
        bool operator==(const TypeCode& v) const { return code == v.code; }
        bool operator!=(const TypeCode& v) const { return code != v.code; }
    };

    class BID
    {
    public:
        Boilerplate(ID)

        const char *name() const;
        bool is_updated() const;
        bool is_updated_data() const;
    };

    class BObject
    {
    public:
        Boilerplate(Object)
        Compatible(BID)

        blist_range<ModifierData> modifiers();
        blist_range<FCurve> fcurves();
        blist_range<bDeformGroup> deform_groups();

        const char *name() const;
        void* data();
        float4x4 matrix_local() const;
        bool is_visible(Scene *scene) const;
    };

    class BMesh
    {
    public:
        Boilerplate(Mesh)
        Compatible(BID)

        barray_range<MLoop> indices();
        barray_range<MEdge> edges();
        barray_range<MPoly> polygons();
        barray_range<MVert> vertices();
        barray_range<float3> normals();
        barray_range<float2> uv();
        barray_range<MLoopCol> colors();

        void calc_normals_split();
    };

    using BMTriangle = BMLoop*[3];
    class BEditMesh
    {
    public:
        Boilerplate2(BEditMesh, BMEditMesh)

        barray_range<BMFace*> polygons();
        barray_range<BMVert*> vertices();
        barray_range<BMTriangle> triangles();
        int uv_data_offset() const;
    };


    class BFCurve
    {
    public:
        Boilerplate(FCurve)

        const char *path() const;
        int array_index() const;
        float evaluate(float time);
    };

    class BMaterial
    {
    public:
        Boilerplate(Material)
        Compatible(BID)

        const char *name() const;
        const float3& color() const;
        bool use_nodes() const;
        Material* active_node_material() const;
    };

    class BCamera
    {
    public:
        Boilerplate(Camera)
        Compatible(BID)

        float fov() const;
    };

    class BScene
    {
    public:
        Boilerplate(Scene)
        Compatible(BID)

        blist_range<Base> objects();

        int fps();
        int frame_start();
        int frame_end();
        int frame_current();
        void frame_set(int f, float subf = 0.0f);
    };

    class BData
    {
    public:
        Boilerplate2(BData, Main)

        blist_range<Object> objects();
        blist_range<Material> materials();

        bool objects_is_updated();
    };

    class BContext
    {
    public:
        Boilerplate2(BContext, bContext)

        static BContext get();
        Main* data();
        Scene* scene();
    };

#undef Compatible
#undef Boilerplate
#undef Boilerplate2
} // namespace blender
