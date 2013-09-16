#include "unity_fixture.h"
#include "hashfunctions.h"


TEST_GROUP(mix_jerkins);

double** matrix_test;

uint mix_test(uint state)
{
	state += state << 1;
	return state;
}

SETUP_DEPENDENCIES(mix_jerkins) { }

GIVEN(mix_jerkins) { }

WHEN(mix_jerkins)
{
    matrix_test = AvalancheMatrix(100, 5, 4, mix_test);
}

TEST_TEAR_DOWN(mix_jerkins) { }

TEST(mix_jerkins, then_avalanche_matrix_must_be_0_5) { }

TEST_GROUP_RUNNER(mix_jerkins)
{
    RUN_TEST_CASE(mix_jerkins, then_avalanche_matrix_must_be_0_5);
}


