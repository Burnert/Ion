#pragma once

#include "Core/Math/Math.h"

namespace Ion
{
	class Rotator
	{
	public:
		Rotator() :
			m_Angles(Vector3()),
			m_Quaternion(Quaternion())
		{ }

		Rotator(const Vector3& angles) :
			m_Angles(angles),
			m_Quaternion(Quaternion(glm::radians(angles)))
		{ }

		Rotator(const Quaternion& quaternion) :
			m_Quaternion(quaternion),
			m_Angles(glm::degrees(glm::eulerAngles(quaternion)))
		{ }

		void Rotate(const Quaternion& quaternion)
		{
			m_Quaternion += quaternion;
			m_Angles = glm::degrees(glm::eulerAngles(m_Quaternion));
		}

		inline void Rotate(const Vector3& angles)
		{
			Rotate(Quaternion(glm::radians(angles)));
		}

		inline void Rotate(const Rotator& rotator)
		{
			Rotate(rotator.Quat());
		}


		inline Vector3 Forward() const
		{
			return glm::rotate(m_Quaternion, Vector3(0.0f, 0.0f, -1.0f));
		}
		inline Vector3 Right() const
		{
			return glm::rotate(m_Quaternion, Vector3(1.0f, 0.0f, 0.0f));
		}
		inline Vector3 Up() const
		{
			return glm::rotate(m_Quaternion, Vector3(0.0f, 1.0f, 0.0f));
		}

		void SetPitch(float pitch)
		{
			m_Angles.x = pitch;
			RecalcQuaternion();
		}
		void SetYaw(float yaw)
		{
			m_Angles.y = yaw;
			RecalcQuaternion();
		}
		void SetRoll(float roll)
		{
			m_Angles.z = roll;
			RecalcQuaternion();
		}

		inline float Pitch() const { return m_Angles.x; }
		inline float Yaw() const { return m_Angles.y; }
		inline float Roll() const { return m_Angles.z; }

		inline Quaternion Quat() const { return m_Quaternion; }

		inline Rotator Inverse() const
		{
			return Rotator(glm::inverse(m_Quaternion));
		}

		Rotator operator+(const Rotator& other) const
		{
			Rotator rot = *this;
			rot.Rotate(other);
			return rot;
		}

		Rotator& operator+=(const Rotator& other)
		{
			Rotate(other);
			return *this;
		}

		Rotator operator-(const Rotator& other) const
		{
			Rotator rot = *this;
			rot.Rotate(-other);
			return rot;
		}

		Rotator& operator-=(const Rotator& other)
		{
			Rotate(-other);
			return *this;
		}

		Rotator operator-() const
		{
			return Inverse();
		}

		/* Returns true even if some of the components are not exactly the same, 
		   but effectively the same (e.g. 360 and 0 degrees) */
		bool operator==(const Rotator& other) const
		{
			return m_Quaternion == other.m_Quaternion;
		}

		bool operator!=(const Rotator& other) const
		{
			return !(*this == other);
		}

	private:
		void RecalcQuaternion()
		{
			m_Quaternion = Quaternion(glm::radians(m_Angles));
		}

	private:
		Vector3 m_Angles;
		Quaternion m_Quaternion;
	};
}
