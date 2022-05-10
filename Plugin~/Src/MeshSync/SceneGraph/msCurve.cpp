#include "pch.h"
#include "MeshSync/SceneGraph/msScene.h"
#include "MeshSync/SceneGraph/msCurve.h"

namespace ms {

	Curve::Curve() { clear(); }
	Curve::~Curve() {}

	EntityType Curve::getType() const { return Type::Curve; }

	shared_ptr<Curve> Curve::create(istream& is)
	{
		auto ret = Curve::create();

		// When not creating via Entity::Create, ensure the entity type is read:
		EntityType type;
		read(is, type);

		ret->deserialize(is);
		return ret;
	}

	void Curve::clear()
	{
		super::clear();

		splines.clear();
	}

	bool Curve::isGeometry() const { return true; }

	uint64_t Curve::hash() const
	{
		auto ret = super::hash();

		for (auto spline : splines) {
			ret += csum(spline->closed);
			ret += vhash(spline->cos);
			ret += vhash(spline->handles_left);
			ret += vhash(spline->handles_right);
		}

		return ret;
	}

	uint64_t Curve::checksumGeom() const
	{
		auto ret = super::checksumGeom();
		
		for (auto spline : splines) {
			ret += csum(spline->closed);
			ret += csum(spline->cos);
			ret += csum(spline->handles_left);
			ret += csum(spline->handles_right);
		}

		return ret;
	}

	void Curve::serialize(std::ostream& os) const
	{
		super::serialize(os);

		write(os, splines);
	}

	void Curve::deserialize(std::istream& is)
	{
		super::deserialize(is);

		read(is, splines);
	}

	bool Curve::isUnchanged() const
	{
		return false;
	}

	void Curve::detach()
	{
		vdetach(splines);
	}

#define EachMember(F) F(cos) F(handles_left) F(handles_right) F(closed)

	void CurveSpline::serialize(std::ostream& os) const
	{
		EachMember(msWrite);
	}

	void CurveSpline::deserialize(std::istream& is)
	{
		EachMember(msRead);
	}

#undef EachMember

	shared_ptr<CurveSpline> CurveSpline::create(istream& is)
	{
		auto ret = CurveSpline::create();

		ret->deserialize(is);
		return ret;
	}

	void CurveSpline::clear()
	{
		cos.clear();
		handles_left.clear();
		handles_right.clear();
	}
}
