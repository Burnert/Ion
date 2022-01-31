#include "IonPCH.h"

#include "Random.h"

namespace Ion
{
	Random::RNG::RNG()
	{
	}

	Random::RNG::RNG(uint64 seed) :
		m_MTERNG(seed)
	{
	}

	int32 Random::RNG::NextInt32(int32 range)
	{
		return Next(range);
	}

	int32 Random::RNG::NextInt32(int32 min, int32 max)
	{
		return Next(min, max);
	}

	float Random::RNG::NextFloat(float range)
	{
		return Next(range);
	}

	float Random::RNG::NextFloat(float min, float max)
	{
		return Next(min, max);
	}

	int32 Random::Int32(int32 range)
	{
		return Next(range);
	}

	int32 Random::Int32(int32 min, int32 max)
	{
		return Next(min, max);
	}

	float Random::Float(float range)
	{
		return Next(range);
	}

	float Random::Float(float min, float max)
	{
		return Next(min, max);
	}

	Random::RNG* Random::s_DefaultRNG = new Random::RNG(time(0));
}
