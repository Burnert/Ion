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
}

namespace Ion::Test { void MatterTest(); }
