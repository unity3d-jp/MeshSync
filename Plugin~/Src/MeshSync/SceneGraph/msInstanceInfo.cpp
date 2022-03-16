#include "MeshSync/SceneGraph/msInstanceInfo.h"
#include <MeshSync/msFoundation.h>
#include <MeshSync/SceneGraph/msIdentifier.h>

namespace ms {
	void InstanceInfo::serialize(ostream& os)
	{
		write(os, type);
		write(os, path);
		write(os, transforms);
		
	}

	void InstanceInfo::deserialize(istream& is)
	{
		read(is, type);
		read(is, path);
		read(is, transforms);
	}
	void InstanceInfo::clear()
	{
		this->path = "";
		this->transforms.clear();
		this->type = ReferenceType::NONE;
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
		type = v.type;
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
		type = std::move(v.type);
		path = std::move(v.path);
		transforms = std::move(v.transforms);
		return *this;
	}

}
