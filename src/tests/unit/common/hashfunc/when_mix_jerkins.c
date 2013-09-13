#include "unity_fixture.h"
#include "hashfunctions.h"


TEST_GROUP(when_mix_jerkins);

double** matrix_test;

uint mix_test(uint state)
{
	state += state << 1;
	return state;
}

TEST_SETUP(when_mix_jerkins)
{
    matrix_test = AvalancheMatrix(100, 5, 4, mix_test);
}

TEST_TEAR_DOWN(when_mix_jerkins)
{
	
}

TEST(when_mix_jerkins, then_avalanche_matrix_must_be_0_5)
{
    
}

TEST_GROUP_RUNNER(when_mix_jerkins)
{
    RUN_TEST_CASE(when_mix_jerkins, then_avalanche_matrix_must_be_0_5);
}


