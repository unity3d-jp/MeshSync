#include "pch.h"
#include "Test.h"
#include "Utility/TestUtility.h"
#include "Utility/MeshGenerator.h"
#include "MeshSync/SceneGraph/msTransform.h"

#include "MeshSync/SceneGraph/msEntityConverter.h"

using namespace mu;

const float3 startingPosition = { 1, 2, 3 };
const mu::quatf startingRotation = { 4,5,6, 7 };
const float3 startingScale = { 1, 2, 3 };

const std::shared_ptr<ms::Transform> getTestTransform()
{
	const std::shared_ptr<ms::Transform> transform = ms::Transform::create();
	transform->path = "/";

	transform->position = startingPosition;
	transform->rotation = startingRotation;
	transform->scale = startingScale;

	return transform;
}

bool checkTransformMatchesStartingTransform(const std::shared_ptr<ms::Transform> transform)
{
	return
		transform->position == startingPosition &&
		transform->rotation == startingRotation &&
		transform->scale == startingScale;
}

template <typename T>
void TestConverter(const int iterations)
{
	auto converter = T::create();

	// Validate that applying the converter the set number of times, gets back to the start transform:
	const auto transform = getTestTransform();

	for (size_t i = 1; i <= iterations; i++)
	{
		converter->convertTransform(*transform);

		// As long as the iteration count is less than the final one, the transform should be different:
		assert(i < iterations != checkTransformMatchesStartingTransform(transform));
	}
}

TestCase(Test_Undoing_FlipYZ_ZUpCorrector) {
	TestConverter<ms::FlipYZ_ZUpCorrector>(4);
}

TestCase(Test_Undoing_RotateX_ZUpCorrector) {
	TestConverter<ms::RotateX_ZUpCorrector>(8);
}