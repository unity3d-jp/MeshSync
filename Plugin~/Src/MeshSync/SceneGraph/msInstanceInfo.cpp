#include "MeshSync/SceneGraph/msInstanceInfo.h"
#include <MeshSync/msFoundation.h>
#include <MeshSync/SceneGraph/msIdentifier.h>

namespace ms {
	void InstanceInfo::serialize(ostream& os)
	{
		write(os, path);
		write(os, parent_path);
		write(os, transforms);
		
	}

	void InstanceInfo::deserialize(istream& is)
	{
		read(is, path);
		read(is, parent_path);
		read(is, transforms);
	}
	void InstanceInfo::clear()
	{
		this->path = "";
		this->parent_path = "";
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
	Identifier InstanceInfo::getIdentifier()
	{
		return Identifier{ path, InvalidID};
	}

	InstanceInfo::InstanceInfo() {}

	InstanceInfo::InstanceInfo(const InstanceInfo& v)
	{
		*this = v;
	}

	InstanceInfo& InstanceInfo::operator=(const InstanceInfo& v)
	{
		path = v.path;
		transforms = v.transforms;
		return *this;
	}

	InstanceInfo::InstanceInfo(InstanceInfo&& v) noexcept
	{
		*this = std::move(v);
	}

	InstanceInfo& InstanceInfo::operator=(InstanceInfo&& v)
	{
		path = std::move(v.path);
		transforms = std::move(v.transforms);
		return *this;
	}

}
