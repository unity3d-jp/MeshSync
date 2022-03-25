#pragma once
#include "MeshUtils/muMath.h"
#include "MeshSync/msFoundation.h"
#include "MeshSync/SceneGraph/msIdentifier.h"
#include "MeshSync/SceneGraph/msMesh.h"

namespace ms {
	using namespace std;
	using namespace mu;
	class InstanceInfo
	{
		uint64_t m_hash = 0;

	public:

		/// <summary>
		/// /// Path on the hierachy tree to the mesh that the instances refer to.
		/// /// </summary>
		string path;

		string parent_path;

		/// <summary>
		/// World transforms of the instances
		/// </summary>
		SharedVector<float4x4> transforms;

		void serialize(ostream& os);
		void deserialize(istream& is);

		InstanceInfo();
		InstanceInfo(const InstanceInfo& v);
		InstanceInfo& operator=(const InstanceInfo& v);
		InstanceInfo(InstanceInfo&& v) noexcept; // noexcept to enforce std::vector to use move constructor
		InstanceInfo& operator=(InstanceInfo&& v);

		msDefinePool(InstanceInfo);

		void clear();

		static shared_ptr<InstanceInfo> create(std::istream& is);
		uint64_t hash();

		Identifier getIdentifier();

	};
	//msSerializable(InstanceInfo)
}

