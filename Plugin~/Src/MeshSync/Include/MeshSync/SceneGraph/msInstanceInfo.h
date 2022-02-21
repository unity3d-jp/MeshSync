#pragma once
#include "MeshUtils/muMath.h"
#include "MeshSync/msFoundation.h"
#include "MeshSync/SceneGraph/msIdentifier.h"

namespace ms {
	using namespace std;
	using namespace mu;
	class InstanceInfo
	{
		uint64_t m_hash = 0;

	public:

		/// <summary>
		/// /// Path on the hierachy tree to the mesh that it referes to
		/// /// </summary>
		string path;

		/// <summary>
		/// World transforms of the instances
		/// </summary>
		vector<float4x4> transforms;

		void serialize(ostream& os);
		void deserialize(istream& is);

		msDefinePool(InstanceInfo);

		void clear();

		static shared_ptr<InstanceInfo> create(std::istream& is);
		uint64_t hash();

		Identifier getIdentifier();
	};
	msSerializable(InstanceInfo)
}

