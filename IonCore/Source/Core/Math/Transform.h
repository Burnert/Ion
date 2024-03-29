#pragma once

#include "Core/Math/Math.h"
#include "Rotator.h"

namespace Ion
{
	class Transform
	{
	public:
		Transform(const Vector3& location = Vector3(0.0f), const Rotator& rotation = Rotator(), const Vector3 scale = Vector3(1.0f)) :
			m_Location(location),
			m_Rotation(rotation),
			m_Scale(scale)
		{
			RebuildMatrix();
		}

		Transform(const Matrix4& matrix)
		{
			Quaternion rotation;
			Vector3 skew;
			Vector4 perspective;
			glm::decompose(matrix, m_Scale, rotation, m_Location, skew, perspective);
			m_Rotation = Rotator(rotation);

			RebuildMatrix();
		}

		void SetLocation(const Vector3& location)
		{
			m_Location = location;
			RebuildMatrix();
		}
		void SetRotation(const Rotator& rotation)
		{
			m_Rotation = rotation;
			RebuildMatrix();
		}
		void SetScale(const Vector3& scale)
		{
			m_Scale = scale;
			RebuildMatrix();
		}

		inline const Vector3& GetLocation() const { return m_Location; }
		inline const Rotator& GetRotation() const { return m_Rotation; }
		inline const Vector3& GetScale() const    { return m_Scale; }

		inline const Matrix4& GetMatrix() const   { return m_Matrix; }

		inline Vector3 GetForwardVector() const   { return m_Rotation.Forward(); }
		inline Vector3 GetRightVector() const     { return m_Rotation.Right(); }
		inline Vector3 GetUpVector() const        { return m_Rotation.Up(); }

		// Transform operators

		Transform& operator*=(const Transform& other)
		{
			m_Matrix = other.GetMatrix() * m_Matrix;

			Quaternion rotation;
			Vector3 skew;
			Vector4 perspective;
			glm::decompose(m_Matrix, m_Scale, rotation, m_Location, skew, perspective);

			m_Rotation = Rotator(rotation);

			return *this;
		}

		Transform operator*(const Transform& other) const
		{
			Transform transform = *this;
			return transform *= other;
		}

		// Location operators

		Transform& operator+=(const Vector3& location)
		{
			m_Location += location;
			RebuildMatrix();

			return *this;
		}

		Transform operator+(const Vector3& location) const
		{
			Transform transform = *this;
			return transform += location;
		}

		// Rotation operators

		Transform& operator+=(const Rotator& rotator)
		{
			m_Rotation += rotator;
			RebuildMatrix();

			return *this;
		}

		Transform operator+(const Rotator& rotator) const
		{
			Transform transform = *this;
			return transform += rotator;
		}

		// Scale operators

		Transform& operator*=(const Vector3& scale)
		{
			m_Scale *= scale;
			RebuildMatrix();

			return *this;
		}

		Transform operator*(const Vector3& scale) const
		{
			Transform transform = *this;
			return transform *= scale;
		}

		// Comparison operators

		bool operator==(const Transform& other) const
		{
			return 
				m_Location == other.m_Location && 
				m_Rotation == other.m_Rotation && 
				m_Scale == other.m_Scale;
		}

		bool operator!=(const Transform& other) const
		{
			return !(*this == other);
		}

	private:
		void RebuildMatrix()
		{
			m_Matrix = Matrix4(1.0f);
			m_Matrix *= Math::Translate(m_Location);
			m_Matrix *= Math::ToMat4(m_Rotation.Quat());
			m_Matrix *= Math::Scale(m_Scale);
		}

	private:
		Vector3 m_Location;
		Rotator m_Rotation;
		Vector3 m_Scale;

		Matrix4 m_Matrix;
	};
}
