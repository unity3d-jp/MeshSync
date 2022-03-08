#pragma once
#include "MeshUtils/muMath.h"
#include "MeshSync/msFoundation.h"
#include "MeshSync/SceneGraph/msIdentifier.h"

#include "MeshSync/SceneGraph/msVariant.h"


namespace ms {
	using namespace std;
	using namespace mu;

	class PropertyInfo
	{
		uint64_t m_hash = 0;

	public:
		enum Type {
			Int,
			Float,
			Vector
		};

		/// <summary>
		/// Path on the hierachy tree to the object it refers to
		/// </summary>
		string path;

		string name;

		string modifierName;
		string propertyName;
		
		Type type;
		float min, max;

		SharedVector<char> data;

		void serialize(ostream& os) const;
		void deserialize(istream& is);

		msDefinePool(PropertyInfo);

		void clear();
		
		static shared_ptr<PropertyInfo> create(std::istream& is);
		uint64_t hash();

		Identifier getIdentifier();

		template<class T> 
		void set(const T& v, const float& min, const float& max);

		template<class T> T& get() const;

		void copy(void* dst) const;
	};

	msSerializable(PropertyInfo)
}

