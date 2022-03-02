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
		//class Record {
		//public:
		//	virtual void serialize(ostream& os) {}
		//	virtual void deserialize(istream& is) {}
		//};

		//template <typename T>
		//class RecordImpl : public Record {
		//public:
		//	T value;
		//	T min;
		//	T max;

		//	void serialize(ostream& os) override;
		//	void deserialize(istream& is) override;

		//	RecordImpl() { }

		//	RecordImpl(T val, T minV, T maxV)
		//	{
		//		value = val;
		//		min = minV;
		//		max = maxV;
		//	}
		//};

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

		Type type;
		float min, max;

		SharedVector<char> data;

		//RecordImpl<int> recordInt;
		//RecordImpl<float> recordFloat;
		//RecordImpl<float3> recordVector;

		void serialize(ostream& os);
		void deserialize(istream& is);

		msDefinePool(PropertyInfo);

		void clear();
		
		static shared_ptr<PropertyInfo> create(std::istream& is);
		uint64_t hash();

		Identifier getIdentifier();

		template<class T> 
		void set(const T& v, const float& min, const float& max);

		template<class T> T& get() const;
		//void set(RecordImpl<int> record);
		//void set(RecordImpl<float> record);
		//void set(RecordImpl<float3> record);

		void copy(void* dst) const;
	};

	msSerializable(PropertyInfo)
}

