#include "pch.h"
#include "Test.h"
#include "Utility/TestUtility.h"
#include "Utility/MeshGenerator.h"
#include "MeshSync/SceneGraph/msTransform.h"

#include "MeshSync/SceneGraph/msEntityConverter.h"

using namespace mu;


TestCase(Test_Undoing_FlipYZ_ZUpCorrector) {
    auto converter = ms::FlipYZ_ZUpCorrector::create();

    // Validate that applying the converter 4 times, gets back to the start transform:
    const auto transform = ms::Transform::create();
    transform->path = "/";

    float3 startingPosition = { 1, 2, 3 };
    mu::quatf startingRotation = { 4,5,6, 7 };
    float3 startingScale = { 1, 2, 3 };

    transform->position = startingPosition;
    transform->rotation = startingRotation;
    transform->scale = startingScale;

    // 1
    converter->convertTransform(*transform);

    assert(transform->position[0] == 1 &&
        transform->position[1] == 3 &&
        transform->position[2] == -2);

    // 2
    converter->convertTransform(*transform);

    assert(transform->position[0] == 1 &&
        transform->position[1] == -2 &&
        transform->position[2] == -3);

    // 3
    converter->convertTransform(*transform);

    assert(transform->position[0] == 1 &&
        transform->position[1] == -3 &&
        transform->position[2] == 2);

    // 4
    converter->convertTransform(*transform);

    assert(transform->position[0] == startingPosition[0] &&
        transform->position[1] == startingPosition[1] &&
        transform->position[2] == startingPosition[2]);

    assert(transform->rotation[0] == startingRotation[0] &&
        transform->rotation[1] == startingRotation[1] &&
        transform->rotation[2] == startingRotation[2] &&
        transform->rotation[3] == startingRotation[3]);

    assert(transform->scale[0] == startingScale[0] &&
        transform->scale[1] == startingScale[1] &&
        transform->scale[2] == startingScale[2]);
}

TestCase(Test_Undoing_RotateX_ZUpCorrector) {
    auto converter = ms::RotateX_ZUpCorrector::create();

    // Validate that applying the converter 4 times, gets back to the start transform:
    const auto transform = ms::Transform::create();
    transform->path = "/";
    float3 startingPosition = { 1, 2, 3 };
    mu::quatf startingRotation = { 4,5,6, 7 };
    float3 startingScale = { 1, 2, 3 };

    transform->position = startingPosition;
    transform->rotation = startingRotation;
    transform->scale = startingScale;

    for (int i = 0; i < 8; i++) {
        converter->convertTransform(*transform);
    }

    assert(transform->position[0] == startingPosition[0] &&
        transform->position[1] == startingPosition[1] &&
        transform->position[2] == startingPosition[2]);

    assert(transform->rotation[0] == startingRotation[0] &&
        transform->rotation[1] == startingRotation[1] &&
        transform->rotation[2] == startingRotation[2] &&
        transform->rotation[3] == startingRotation[3]);

    assert(transform->scale[0] == startingScale[0] &&
        transform->scale[1] == startingScale[1] &&
        transform->scale[2] == startingScale[2]);
}