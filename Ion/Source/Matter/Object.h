#pragma once

#include "Core.h"

#include "Reflection.h"

namespace Ion
{
#pragma region Matter Object base class

	class ION_API MObject
	{
	public:
		MCLASS(MObject)

		template<typename T, TEnableIfT<TIsConvertibleV<T*, MObject*>>* = 0>
		static T* New();

		MClass* GetClass() const;

		void SetName(const String& name);
		const String& GetName() const;
		const GUID& GetGuid() const;

	protected:
		virtual void Serialize(Archive& ar);

		MObject();
		virtual ~MObject();

	private:
		MClass* m_Class;
		String m_Name;
		GUID m_Guid;
		
		friend class MReflection;
		friend class MClass;
		friend class MObjectSerializer;

	public:
		friend Archive& operator<<(Archive& ar, MObject*& object);
	};

	template<typename T, TEnableIfT<TIsConvertibleV<T*, MObject*>>*>
	FORCEINLINE T* MObject::New()
	{
		MClass* mClass = T::StaticClass();

		T* object = static_cast<T*>(mClass->Instantiate());

		return object;
	}

	FORCEINLINE MClass* MObject::GetClass() const
	{
		return m_Class;
	}

	FORCEINLINE void MObject::SetName(const String& name)
	{
		m_Name = name;
	}

	FORCEINLINE const String& MObject::GetName() const
	{
		return m_Name;
	}

	FORCEINLINE const GUID& MObject::GetGuid() const
	{
		return m_Guid;
	}

#pragma endregion

#pragma region Matter Object Serialization

	/**
	 * @brief Helper function for passing an MObject pointer reference to an archive.
	 * 
	 * @tparam TMObject Type that inherits from MObject
	 * @param object MObject non-const pointer reference
	 * @return Pointer reference cast to MObject*&
	 */
	template<typename TObject>
	FORCEINLINE MObject*& SerializeMObject(TObject*& object)
	{
		return reinterpret_cast<MObject*&>(object);
	}

#pragma endregion
}

namespace Ion::Test { void MatterTest(); }
