#pragma once

namespace Ion
{
	class ION_API Random
	{
	public:
		class RNG
		{
		public:
			RNG();
			RNG(uint64 seed);

			int32 NextInt32(int32 range);
			int32 NextInt32(int32 min, int32 max);
			float NextFloat(float range);
			float NextFloat(float min, float max);

			/* Generate a next random number in range [min - max] (inclusive). */
			template<typename T, typename TEnableIfT<TIsIntegralV<T>>* = 0>
			T Next(T min, T max);
			/* Generate a next random number in range [min - max] (inclusive). */
			template<typename T, typename TEnableIfT<TIsFloatingV<T>>* = 0>
			T Next(T min, T max);
			/* Generate a next random number in range [0 - max] (inclusive). */
			template<typename T>
			T Next(T range);

		private:
			std::mt19937_64 m_MTERNG;
		};

		Random() = delete;

		static int32 Int32(int32 range);
		static int32 Int32(int32 min, int32 max);
		static float Float(float range);
		static float Float(float min, float max);

		/* Generate a next random number in range [min - max] (inclusive). */
		template<typename T>
		static T Next(T min, T max);
		/* Generate a next random number in range [0 - max] (inclusive). */
		template<typename T>
		static T Next(T range);

	private:
		static RNG* s_DefaultRNG;
	};

	template<typename T, typename TEnableIfT<TIsIntegralV<T>>*>
	inline T Random::RNG::Next(T min, T max)
	{
		std::uniform_int_distribution dist(min, max);
		return dist(m_MTERNG);
	}

	template<typename T, typename TEnableIfT<TIsFloatingV<T>>*>
	inline T Random::RNG::Next(T min, T max)
	{
		std::uniform_real_distribution dist(min, max);
		return dist(m_MTERNG);
	}

	template<typename T>
	inline T Random::RNG::Next(T range)
	{
		return Next((T)0, range);
	}

	template<typename T>
	inline T Random::Next(T min, T max)
	{
		return s_DefaultRNG->Next(min, max);
	}

	template<typename T>
	inline T Random::Next(T range)
	{
		return s_DefaultRNG->Next(range);
	}
}
