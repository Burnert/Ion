#pragma once

#include "Core.h"

#include "ObjectPtr.h"
#include "Reflection.h"

namespace Ion
{
#pragma region Matter Object base class

	class ION_API MObject : public TEnableSFT<MObject>
	{
	public:
		MATTER_DECLARE_CLASS(MObject)

		template<typename T, TEnableIfT<TIsConvertibleV<T*, MObject*>>* = 0>
		static TObjectPtr<T> New();

		MClass* GetClass() const;

		void SetName(const String& name);
		MMETHOD(SetName, const String&)

		const String& GetName() const;
		MMETHOD(GetName)

		const GUID& GetGuid() const;
		MMETHOD(GetGuid)

	protected:
		virtual void Serialize(Archive& ar);

		MObject();

	public:
		virtual ~MObject();

	private:
		MClass* m_Class;
		String m_Name;
		GUID m_Guid;
		
		friend class MReflection;
		friend class MClass;
		friend class MObjectSerializer;

	public:
		friend Archive& operator&=(Archive& ar, MObjectPtr& object);
	};

	template<typename T, TEnableIfT<TIsConvertibleV<T*, MObject*>>*>
	FORCEINLINE TObjectPtr<T> MObject::New()
	{
		MClass* mClass = T::StaticClass();

		TObjectPtr<T> object = PtrCast<T>(mClass->Instantiate());

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

#pragma region Templates - TIsMObject

	template<typename T>
	struct TIsMObject : TIsConvertible<T*, MObject*> { };

	template<typename T>
	static inline constexpr bool TIsMObjectV = TIsMObject<T>::value;

#pragma endregion

#pragma region Matter Object Serialization

	/**
	 * @brief Helper function for passing an MObject pointer reference to an archive.
	 * 
	 * @tparam TMObject Type that inherits from MObject
	 * @param object MObject non-const pointer reference
	 * @return Pointer reference cast to MObject*&
	 */
	template<typename T>
	FORCEINLINE MObjectPtr& SerializeMObject(TObjectPtr<T>& object)
	{
		return reinterpret_cast<MObjectPtr&>(object);
	}

#pragma endregion
}

namespace Ion::Test { void MatterTest(); }
