#include "MeshSync/SceneGraph/msPropertyInfo.h"
#include <MeshSync/msFoundation.h>
#include <MeshSync/SceneGraph/msIdentifier.h>

namespace ms {

#define EachMember(F) F(type) F(sourceType) F(name) F(path) F(min) F(max) F(data) F(modifierName) F(propertyName) 

	void PropertyInfo::serialize(ostream& os) const
	{
		EachMember(msWrite);
	}

	void PropertyInfo::deserialize(istream& is)
	{
		EachMember(msRead);
	}

#undef EachMember(F)

	void PropertyInfo::clear()
	{
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

		if (type == Type::String) {
			((char*)dst)[getArrayLength()] = 0;
		}
	}

	bool PropertyInfo::matches(const PropertyInfoPtr other) const {
		return
			path == other->path &&
			type == other->type &&
			sourceType == other->sourceType &&
			name == other->name &&
			modifierName == other->modifierName &&
			propertyName == other->propertyName;
	}

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
Body(float, Float)


#define Body(A, B)\
    template<> void PropertyInfo::set(const A& v, const float& min, const float& max) { type = Type::B; this->min = min; this->max = max; set_impl(data, v); }\
	template<> void PropertyInfo::set(const A* v, const float& min, const float& max, size_t length)\
	{\
		type = Type::B##Array; this->min = min; this->max = max;\
		set_impl(data, v, length);\
	}\
	template<> A& PropertyInfo::get() const { return *(A*)data.cdata(); }\
	template<> A* PropertyInfo::getArray() const { return (A*)data.cdata(); }

	EachType(Body)
#undef Body

	void PropertyInfo::set(const char* v, size_t length)
	{
		type = Type::String;
		set_impl(data, v, length);
	}

	size_t PropertyInfo::getArrayLength() const
	{
		switch (type) {
		case Type::String: return data.size();
#define Body(A, B) case Type::B##Array: return data.size() / sizeof(A);
			EachType(Body)
#undef Body
		default: return 0;
		}
	}
}
