#pragma once

#include "MeshSync/MeshSyncConstants.h"

#include "MeshSync/SceneGraph/msIdentifier.h"
#include "MeshSync/SceneGraph/msTransform.h"

#include "MeshUtils/muVertex.h" //mu::Weights4

msDeclStructPtr(CurveSpline);

using namespace std;
using namespace mu;

namespace ms {
	struct CurveSpline {
		SharedVector<mu::float3> cos;
		SharedVector<mu::float3> handles_left;
		SharedVector<mu::float3> handles_right;
		bool closed;

		msDefinePool(CurveSpline);

		static shared_ptr<CurveSpline> create(std::istream& is);

		void serialize(std::ostream& os) const;
		void deserialize(std::istream& is);

		void clear();
	};
	msSerializable(CurveSpline);

	class Curve : public Transform
	{
		using super = Transform;

	public:
		vector<CurveSplinePtr> splines;

	protected:
		Curve();
		~Curve() override;
	public:
		msDefinePool(Curve);

		static shared_ptr<Curve> create(std::istream& is);

		Type getType() const override;
		bool isGeometry() const override;
		void serialize(std::ostream& os) const override;
		void deserialize(std::istream& is) override;
		void detach() override;
		//void setupDataFlags() override;

		bool isUnchanged() const override;
		//bool isTopologyUnchanged() const override;
		//bool strip(const Entity& base) override;
		//bool merge(const Entity& base) override;
		//bool diff(const Entity& e1, const Entity& e2) override;
		//bool lerp(const Entity& e1, const Entity& e2, float t) override;
		//void updateBounds() override;

		void clear() override;
		uint64_t hash() const override;
		uint64_t checksumGeom() const override;
		//uint64_t vertexCount() const override;
		//EntityPtr clone(bool detach = false) override;
	};
	msSerializable(Curve);
} // namespace ms
