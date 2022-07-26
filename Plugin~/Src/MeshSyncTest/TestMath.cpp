#include "pch.h"
#include "Test.h"
#include "Common.h"

using namespace ms;

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

TestCase(TestCeilToDecimal)
{
	// 2 places:
	assert(compare(ceilToDecimals(mu::tvec3<float>{ 1.121f, 2.342f, 3.453 }), { 1.13f, 2.35f, 3.46f }) && "Error ceiling tvec3<float> to 2 decimal places");
	assert(compare(ceilToDecimals(mu::tvec3<float>{ 1.125f, 2.346f, 3.457 }), { 1.13f, 2.35f, 3.46f }) && "Error ceiling tvec3<float> to 2 decimal places");

	// 5 places:
	assert(compare(ceilToDecimals(mu::tvec2<float>{ 1.12111111111f, 2.3422222222f}, 5), { 1.12112f, 2.34223f }) && "Error ceiling tvec2<float> to 5 decimal places");
	assert(compare(ceilToDecimals(mu::tvec2<float>{ 1.12555555555f, 2.3466666666f }, 5), { 1.12556f, 2.34667f }) && "Error ceiling tvec2<float> to 5 decimal places");
	
	// 10 places:
	assert(compare(mu::ceilToDecimals(1.12111111111111f, 1), 1.2f) && "Error ceiling float to 1 decimal place");
	assert(compare(mu::ceilToDecimals(1.1775555f, 1), 1.2f) && "Error ceiling float to 1 decimal place");
}
