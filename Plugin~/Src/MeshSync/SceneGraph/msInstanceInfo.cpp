#include "MeshSync/SceneGraph/msInstanceInfo.h"
#include <MeshSync/msFoundation.h>

namespace ms {
	void InstanceInfo::serialize(ostream& os)
	{
		write(os, path);
		write(os, transforms);
	}

	void InstanceInfo::deserialize(istream& is)
	{
		read(is, path);
		read(is, transforms);
	}
	void InstanceInfo::clear()
	{
		this->path = "";
		this->transforms.clear();
	}

	shared_ptr<InstanceInfo> InstanceInfo::create(istream& is)
	{
		auto ret = InstanceInfo::create();
		ret->deserialize(is);
		return ret;
	}

	uint64_t InstanceInfo::hash()
	{
		return m_hash;
	}
}
