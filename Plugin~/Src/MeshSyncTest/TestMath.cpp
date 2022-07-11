#include "pch.h"
#include "Test.h"
#include "Common.h"

using namespace ms;

//VF1(comp)

template
<typename T>
bool compare(T val, T expected) {
	auto difference = val - expected;

	for (int i = 0; i < difference.vector_length; i++) {
		if (fabs(difference[i]) > muEpsilon) {
			return false;
		}
	}

	return true;
}

bool compare(float val, float expected) {
	auto difference = val - expected;

	return fabs(difference) < muEpsilon;
}

TestCase(TestRoundToDecimal)
{
	// 2 places:
	// Round down
	assert(compare(roundToDecimals(mu::tvec3<float>{ 1.121f, 2.342f, 3.453 }), { 1.12f, 2.34f, 3.45f }) && "Error rounding tvec3<float> down to 2 decimal places");

	// Round up
	assert(compare(roundToDecimals(mu::tvec3<float>{ 1.125f, 2.346f, 3.457 }), { 1.13f, 2.35f, 3.46f }) && "Error rounding tvec3<float> up to 2 decimal places");

	// 5 places:
	// Round down
	assert(compare(roundToDecimals(mu::tvec2<float>{ 1.12111111111f, 2.3422222222f}, 5), { 1.12111f, 2.34222f }) && "Error rounding tvec2<float> down to 5 decimal places");

	// Round up
	assert(compare(roundToDecimals(mu::tvec2<float>{ 1.12555555555f, 2.3466666666f }, 5), { 1.12556f, 2.34667f }) && "Error rounding tvec2<float> up to 5 decimal places");


	// 10 places:
	// Round down
	assert(compare(mu::roundToDecimals(1.12111111111111f, 1), 1.1f) && "Error rounding float down to 1 decimal place");

	// Round up
	assert(compare(mu::roundToDecimals(1.1775555f, 1), 1.2f) && "Error rounding float up to 1 decimal place");
}
