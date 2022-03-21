#pragma once

#include "Core/Math/Math.h"

namespace Ion
{
	class Rotator
	{
	public:
		inline Rotator() :
			m_Angles(Vector3()),
			m_Quaternion(Quaternion(1.0f, 0.0f, 0.0f, 0.0f))
		{ }

		inline Rotator(const Vector3& angles) :
			m_Angles(angles),
			m_Quaternion(Quaternion(Math::Radians(angles)))
		{ }

		inline Rotator(const Quaternion& quaternion) :
			m_Quaternion(quaternion),
			m_Angles(Math::Degrees(Math::Euler(quaternion)))
		{ }

		inline void Rotate(const Quaternion& quaternion)
		{
			m_Quaternion = quaternion * m_Quaternion;
			m_Angles = Math::Degrees(Math::Euler(m_Quaternion));
		}

		inline void Rotate(const Vector3& angles)
		{
			Rotate(Quaternion(Math::Radians(angles)));
		}

		inline void Rotate(const Rotator& rotator)
		{
			Rotate(rotator.Quat());
		}

		inline Vector3 Forward() const
		{
			return Math::Rotate(m_Quaternion, Vector3(0.0f, 0.0f, -1.0f));
		}
		inline Vector3 Right() const
		{
			return Math::Rotate(m_Quaternion, Vector3(1.0f, 0.0f, 0.0f));
		}
		inline Vector3 Up() const
		{
			return Math::Rotate(m_Quaternion, Vector3(0.0f, 1.0f, 0.0f));
		}

		inline void SetPitch(float pitch)
		{
			m_Angles.x = pitch;
			RecalcQuaternion();
		}
		inline void SetYaw(float yaw)
		{
			m_Angles.y = yaw;
			RecalcQuaternion();
		}
		inline void SetRoll(float roll)
		{
			m_Angles.z = roll;
			RecalcQuaternion();
		}

		inline Vector3 Angles() const { return m_Angles; }
		inline float Pitch() const    { return m_Angles.x; }
		inline float Yaw() const      { return m_Angles.y; }
		inline float Roll() const     { return m_Angles.z; }

		inline Quaternion Quat() const { return m_Quaternion; }

		inline Rotator Inverse() const
		{
			return Rotator(Math::Inverse(m_Quaternion));
		}

		inline Rotator operator+(const Rotator& other) const
		{
			Rotator rot = *this;
			rot.Rotate(other);
			return rot;
		}

		inline Rotator& operator+=(const Rotator& other)
		{
			Rotate(other);
			return *this;
		}

		inline Rotator operator-(const Rotator& other) const
		{
			Rotator rot = *this;
			rot.Rotate(-other);
			return rot;
		}

		inline Rotator& operator-=(const Rotator& other)
		{
			Rotate(-other);
			return *this;
		}

		inline Rotator operator-() const
		{
			return Inverse();
		}

		/* Returns true even if some of the components are not exactly the same, 
		   but effectively the same (e.g. 360 and 0 degrees) */
		inline bool operator==(const Rotator& other) const
		{
			return m_Quaternion == other.m_Quaternion;
		}

		inline bool operator!=(const Rotator& other) const
		{
			return !(*this == other);
		}

	private:
		inline void RecalcQuaternion()
		{
			m_Quaternion = Quaternion(Math::Radians(m_Angles));
		}

	private:
		Vector3 m_Angles;
		Quaternion m_Quaternion;
	};
}
