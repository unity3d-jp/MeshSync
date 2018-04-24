#pragma once

namespace blender
{
    void setup();
    const void* CustomData_get(const CustomData& data, int type);
    int CustomData_number_of_layers(const CustomData& data, int type);

    struct ListHeader { ListHeader *next, *prev; };
    template<typename T> inline T rna_data(py::object p) { return (T)((BPy_StructRNA*)p.ptr())->ptr.id.data; }
    template<typename T> inline void rna_data(py::object p, T& v) { v = (T)((BPy_StructRNA*)p.ptr())->ptr.id.data; }
    template<typename T> inline T rna_sdata(py::object p) { return (T)((BPy_StructRNA*)p.ptr())->ptr.data; }
    template<typename T> inline void rna_sdata(py::object p, T& v) { v = (T)((BPy_StructRNA*)p.ptr())->ptr.data; }

    template<typename T>
    struct biterator
    {
        T *ptr;

        biterator(T *p) : ptr(p) {}
        T* operator*() const { return ptr; }
        T* operator++() { ptr = (T*)(((ListHeader*)ptr)->next); return ptr; }
        bool operator==(const biterator& v) const { return ptr == v.ptr; }
        bool operator!=(const biterator& v) const { return ptr != v.ptr; }
    };

    template<typename T>
    struct brange
    {
        using iterator = biterator<T>;
        T *ptr;

        brange(T *p) : ptr(p) {}
        iterator begin() { return iterator(ptr); }
        iterator end() { return iterator(nullptr); }
    };
    template<typename T> brange<T> range(T *t) { return brange<T>(t); }


#define Boilerplate2(Type, BType)\
    using btype = BType;\
    static StructRNA *s_type;\
    ::BType *m_ptr;\
    static StructRNA* type() { return s_type; }\
    Type(::BType *p) : m_ptr(p) {}\
    Type(py::object p) : m_ptr(rna_data<::BType*>(p)) {}

#define Boilerplate(Type) Boilerplate2(B##Type, Type)



    class BObject
    {
    public:
        Boilerplate(Object)

        const char *name() const;
        void* data();
        brange<ModifierData> modifiers();
        brange<FCurve> fcurves();
        brange<bDeformGroup> vertex_groups();
    };

    class BMesh
    {
    public:
        Boilerplate(Mesh)

        brange<Key> keys();
        void calc_normals_split();
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

        const char *name() const;
        const float3& color() const;
    };

    class BScene
    {
    public:
        Boilerplate(Scene)

        brange<Object> objects();
    };

    class BContext
    {
    public:
        Boilerplate2(BContext, bContext)

        static BContext get();
        BScene scene();
    };

    class BData
    {
    public:
        Boilerplate2(BData, Main)

        static BData get();
        brange<Object> objects();
        brange<Material> materials();
    };

} // namespace blender
