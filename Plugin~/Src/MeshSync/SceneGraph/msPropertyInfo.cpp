#include "MeshSync/SceneGraph/msPropertyInfo.h"
#include <MeshSync/msFoundation.h>
#include <MeshSync/SceneGraph/msIdentifier.h>

namespace ms {
#define EachMember(F) F(type) F(name) F(path) F(min) F(max) F(data)
	void PropertyInfo::serialize(ostream& os)
	{
		EachMember(msWrite);
		/*write(os, type);
		write(os, path);

		write(os, data);*/

		//switch (type)
		//{
		//case ms::PropertyInfo::Int: recordInt.serialize(os); break;
		//case ms::PropertyInfo::Float: recordFloat.serialize(os); break;
		//case ms::PropertyInfo::Vector: recordVector.serialize(os); break;
		//default:
		//	break;
		//}
	}

	void PropertyInfo::deserialize(istream& is)
	{
		EachMember(msRead);

		//switch (type)
		//{
		//case ms::PropertyInfo::Int: recordInt.deserialize(is); break;
		//case ms::PropertyInfo::Float: recordFloat.deserialize(is); break;
		//case ms::PropertyInfo::Vector: recordVector.deserialize(is); break;
		//default:
		//	break;
		//}
	}

	//#define EachMember(F) F(value) F(min) F(max)
	//
	//	template <typename T>
	//	void PropertyInfo::RecordImpl<T>::serialize(ostream& os)
	//	{
	//		EachMember(msWrite);
	//	}
	//
	//	template <typename T>
	//	void PropertyInfo::RecordImpl<T>::deserialize(istream& is)
	//	{
	//		EachMember(msRead);
	//	}
	

	#undef EachMember(F)

	void PropertyInfo::clear()
	{
		this->path = "";
	}

	shared_ptr<PropertyInfo> PropertyInfo::create(istream& is)
	{
		auto ret = PropertyInfo::create();

		ret->deserialize(is);
		return ret;
	}

	uint64_t PropertyInfo::hash()
	{
		return m_hash;
	}

	Identifier PropertyInfo::getIdentifier()
	{
		return Identifier{ path, InvalidID };
	}

	void PropertyInfo::copy(void* dst) const
	{
		data.copy_to((char*)dst);
	}

	//void PropertyInfo::set(RecordImpl<int> record)
	//{
	//	type = RecordType::Int;
	//	this->recordInt = record;
	//}

	//void PropertyInfo::set(RecordImpl<float> record)
	//{
	//	type = RecordType::Float;
	//	this->recordFloat = record;
	//}

	//void PropertyInfo::set(RecordImpl<float3> record)
	//{
	//	type = RecordType::Vector;
	//	this->recordVector = record;
	//}

	template<class T>
	static inline void set_impl(SharedVector<char>& dst, const T& v)
	{
		dst.resize_discard(sizeof(T));
		(T&)dst[0] = v;
	}

	template<class T>
	static inline void set_impl(SharedVector<char>& dst, const T* v, size_t n)
	{
		dst.resize_discard(sizeof(T) * n);
		memcpy(dst.data(), v, dst.size());
	}

#define EachType(Body)\
Body(int, Int)\
Body(float, Float)\
Body(mu::float3, Vector)


#define Body(A, B)\
    template<> void PropertyInfo::set(const A& v, const float& min, const float& max) { type = Type::B; this->min = min; this->max = max; set_impl(data, v); }\
    /*template<> void PropertyInfo::set(const A *v, size_t n)\
    {\
        type = Type::B;\
        set_impl(data, v, n);\
    }\*/\
    template<> A& PropertyInfo::get() const { return *(A*)data.cdata(); }\
    //template<> const A* PropertyInfo::getArray() const { return (A*)data.cdata(); }

	EachType(Body)
#undef Body

		//	template<> void PropertyInfo::set(const char* v, size_t n)
		//{
		//	type = Type::String;
		//	set_impl(data, v, n);
		//}
}
