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

		/**
		 * @brief Used to create and register an MObject instance at runtime.
		 * Don't call this in MObject constructors, use ConstructDefault instead.
		 * 
		 * @see MObject::ConstructDefault
		 * 
		 * @tparam T Class derived from MObject
		 * @return TObjectPtr<T> Newly created instance
		 */
		template<typename T, TEnableIfT<TIsConvertibleV<T*, MObject*>>* = 0>
		static TObjectPtr<T> New();

		/**
		 * @brief Used to create a default MObject instance in other MObject's constructor
		 * (mainly, to initialize an MObjectPtr field for the CDO). Its fields can then be set manually.
		 * To create an MObject instance at runtime use New.
		 * 
		 * @see MObject::New
		 * 
		 * @tparam T Class derived from MObject
		 * @return TObjectPtr<T> Default instance
		 */
		template<typename T, TEnableIfT<TIsConvertibleV<T*, MObject*>>* = 0>
		static TObjectPtr<T> ConstructDefault();

		/**
		 * @brief Get the reflection MClass of this MObject.
		 */
		MClass* GetClass() const;

		void SetName(const String& name);
		MMETHOD(SetName, const String&)

		const String& GetName() const;
		MMETHOD(GetName)

		const GUID& GetGuid() const;
		MMETHOD(GetGuid)

		bool IsTickEnabled() const;
		void SetTickEnabled(bool bEnabled);

	protected:
		/**
		 * @brief Called after a new MObject instance gets registered.
		 * Override in child classes.
		 */
		virtual void OnCreate() { }

		/**
		 * @brief Called right before destruction.
		 * Override in child classes.
		 */
		virtual void OnDestroy() { }

		/**
		 * @brief Called each frame, if tick is enabled.
		 * Override in child classes.
		 * 
		 * @see MObject::SetTickEnabled
		 * 
		 * @param deltaTime Time in between frames (in seconds).
		 */
		virtual void Tick(float deltaTime) { }

		/**
		 * @brief Two-way serialization function used mainly to automatically
		 * serialize the object's MFields.
		 */
		virtual void Serialize(Archive& ar);

		MObject();

	private:
		static void Register(const MObjectPtr& object);

	public:
		virtual ~MObject();

	private:
		MClass* m_Class;
		String m_Name;
		GUID m_Guid;

		uint8 m_bTickEnabled : 1;

		friend class MReflection;
		friend class MClass;
		friend class MObjectSerializer;
		friend class Engine;

	public:
		friend Archive& operator&=(Archive& ar, MObjectPtr& object);
	};

	template<typename T, TEnableIfT<TIsConvertibleV<T*, MObject*>>*>
	FORCEINLINE TObjectPtr<T> MObject::New()
	{
		MClass* mClass = T::StaticClass();
		ionassert(mClass);

		TObjectPtr<T> object = PtrCast<T>(mClass->Instantiate());

		MObject::Register(object);

		object->OnCreate();

		return object;
	}

	template<typename T, TEnableIfT<TIsConvertibleV<T*, MObject*>>*>
	FORCEINLINE TObjectPtr<T> MObject::ConstructDefault()
	{
		MClass* mClass = T::StaticClass();
		ionassert(mClass);

		return PtrCast<T>(mClass->Instantiate());
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

	FORCEINLINE bool MObject::IsTickEnabled() const
	{
		return m_bTickEnabled;
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
	 * @tparam T Type that inherits from MObject
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
