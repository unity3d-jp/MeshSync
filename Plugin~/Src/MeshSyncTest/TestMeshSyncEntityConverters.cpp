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
    transform->position = { 1,2,3 };
    transform->rotation = { 4,5,6, 7 };
    transform->scale = { 8, 9, 10 };

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

    assert(transform->position[0] == 1 &&
        transform->position[1] == 2 &&
        transform->position[2] == 3);

    assert(transform->rotation[0] == 4 &&
        transform->rotation[1] == 5 &&
        transform->rotation[2] == 6 &&
        transform->rotation[3] == 7);

    assert(transform->scale[0] == 8 &&
        transform->scale[1] == 9 &&
        transform->scale[2] == 10);
}

TestCase(Test_Undoing_RotateX_ZUpCorrector) {
    auto converter = ms::RotateX_ZUpCorrector::create();

    // Validate that applying the converter 4 times, gets back to the start transform:
    const auto transform = ms::Transform::create();
    transform->path = "/";
    transform->position = { 1,2,3 };
    transform->rotation = { 4,5,6, 7 };
    transform->scale = { 8, 9, 10 };

    for (int i = 0; i < 8; i++) {
        converter->convertTransform(*transform);
    }

    assert(transform->position[0] == 1 &&
        transform->position[1] == 2 &&
        transform->position[2] == 3);

    assert(transform->rotation[0] == 4 &&
        transform->rotation[1] == 5 &&
        transform->rotation[2] == 6 &&
        transform->rotation[3] == 7);

    assert(transform->scale[0] == 8 &&
        transform->scale[1] == 9 &&
        transform->scale[2] == 10);
}