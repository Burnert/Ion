#pragma once

/* Do not create custom components with constructors that take parameters.
 * This will not work, because the components are emplaced directly
 * in their containers using the default constructors.
 */

/* Used by the engine to declare the core serialcall functions. */
#define DECLARE_COMPONENT_SERIALCALL(func, ...) \
DECLARE_THASFUNCTION(func); \
namespace ComponentSerialCall::Private \
{ \
	/* Function signature */ \
	using func##FPtr = void(*)(Ion::IComponentContainer*, __VA_ARGS__); \
	/* Components which implement this serialcall */ \
	inline THashMap<ComponentTypeID, func##FPtr> g_##func##Functions; \
	/* Get the function that calls each component's serialcall function. */ \
	inline func##FPtr Get##func##Function(ComponentTypeID id) \
	{ \
		return g_##func##Functions[id]; \
	} \
	/* Called in DECLARE_COMPONENT_SERIALCALL_HELPER_EX */ \
	inline void func##_AddFunctionForType(ComponentTypeID id, const String& typeClassName, func##FPtr fn) \
	{ \
		g_##func##Functions[id] = fn; \
		ComponentLogger.Debug(#func"_AddFunctionForType called for {0} ({1:x})", typeClassName, id); \
	} \
} \
namespace ComponentSerialCall \
{ \
	/* Call all the serialcall functions of components which are in the specified container. */ \
	template<typename... Args> \
	inline void func(ComponentTypeID typeId, Ion::IComponentContainer* container, Args&&... args) \
	{ \
		Private::func##FPtr fn = Private::Get##func##Function(typeId); \
		checked_call(fn, container, args...); \
	} \
}

/* Generates the code used to call serialcall functions on all components. */
#define DECLARE_COMPONENT_SERIALCALL_HELPER_EX(func, forEachBody, ...) \
namespace ComponentSerialCall::Private \
{ \
	/* Type of the this component container */ \
	using func##Container = TCompContainer<ThisComponentClass>; /* THashMap<GUID, _ComponentClass_> */ \
	static String func##TracerName = "DECLARE_COMPONENT_SERIALCALL_HELPER_EX("#func") - " + ThisComponentClass::ClassDisplayName; \
	/* Function that calls all the components' serialcall functions in the specified component container.
		It has to be a lambda to avoid symbol redefinition. */ \
	static auto CallEach##func = [](Ion::IComponentContainer* containerPtr, __VA_ARGS__) \
	{ \
		/* Make sure the type actually has the function, just in case. */ \
		if constexpr (THas##func<ThisComponentClass>) \
		{ \
			TRACE_SCOPE(func##TracerName.c_str()); \
			func##Container& container = *(func##Container*)containerPtr->GetRawContainer(); \
			/* Call the serialcall functions */ \
			for (auto& [k, comp] : container) \
			{ \
				/* Don't call the function if the component is to be destroyed */ \
				if (comp.IsPendingKill()) return; \
				forEachBody; \
			} \
		} \
	}; \
	CALL_OUTSIDE({ \
		s_OnRegisterListeners.Add([] \
		{ \
			func##_AddFunctionForType(ThisComponentClass::GetTypeID(), ThisComponentClass::ClassName, CallEach##func); \
		}); \
	}, func##Implement); \
}

/* Generates the code used to call serialcall functions on all components. */
#define DECLARE_COMPONENT_SERIALCALL_HELPER(func) \
DECLARE_COMPONENT_SERIALCALL_HELPER_EX(func, comp.func())

/* Put in the component's .h file. */
#define ENTITY_COMPONENT_CLASS_HEADER(className) \
template<> \
struct FInstantiateComponent<class className> \
{ \
	static ComponentOld* Call(ComponentRegistry* registry); \
}; \
template<> \
struct FInstantiateComponentContainer<class className> \
{ \
	static IComponentContainer* Call(ComponentRegistry* registry); \
}; \
template<> \
struct FRegisterSerialCallForComponent<class className> \
{ \
	static void Call(); \
}; \
template<> \
struct FRegisterPropertiesForComponent<class className> \
{ \
	static void Call(); \
};

/* Put in the component's class in the .h file */
#define ENTITY_COMPONENT_CLASS_BODY(className, displayName) \
private: \
static inline ComponentTypeID s_TypeID = THash<String>()(#className); \
protected: \
public: \
using ComponentType = className; \
using RawComponentContainerType = TCompContainer<className>; \
static ComponentTypeID GetTypeID() \
{ \
	/*ionassert(s_TypeID != InvalidComponentTypeID, "["#className"] has not been registered."); */ \
	return s_TypeID; \
} \
/* Container wrapper that will be inserted into the Component Registry */ \
class ION_API ComponentContainerImpl : public IComponentContainer \
{ \
public: \
	using Type          = ComponentContainerImpl; \
	using ComponentT    = className; \
	using RawContainerT = RawComponentContainerType; \
	virtual void Erase(ComponentOld* component) override; \
	virtual ComponentOld* Find(ComponentOld* component) const override; \
	virtual ComponentTypeID GetTypeID() const override; \
	virtual void* GetRawContainer() override; \
	virtual ComponentOld* ForEachHelper(void*& inOutGenericIt) override; \
	RawComponentContainerType Container; \
}; \
static inline const String ClassName = #className; \
static inline const String ClassDisplayName = displayName; \
virtual ComponentTypeID GetFinalTypeID() const override { return GetTypeID(); } \
virtual const ComponentDatabase::TypeInfo& GetFinalTypeInfo() const override \
{ \
	return ComponentRegistry::GetComponentTypeDatabase()->GetTypeInfo(GetFinalTypeID()); \
} \
/* @TODO: Add GetTypeData */ \
virtual const String& GetClassName() const override { return ClassName; } \
virtual const String& GetClassDisplayName() const override { return ClassDisplayName; } \
virtual ComponentOld* Duplicate_Internal(ComponentRegistry& registry) const override; \
friend class ComponentRegistry;

/* Put in the component's .cpp file (one per file). */
#define DECLARE_ENTITY_COMPONENT_CLASS(className) \
using ComponentType = className; \
namespace ComponentSerialCall::Private \
{ \
	using ThisComponentClass = className; \
	/* OnRegister called in DECLARE_COMPONENT_SERIALCALL_HELPER_EX */ \
	struct OnRegisterListeners \
	{ \
		using FPtr = void(*)(); \
		using CollectionType = THashSet<FPtr>; \
		CollectionType& GetListeners() \
		{ \
			if (!s_Listeners) \
				s_Listeners = new CollectionType; \
			return *s_Listeners; \
		} \
		void Add(FPtr fptr) \
		{ \
			GetListeners().insert(fptr); \
		} \
	private: \
		CollectionType* s_Listeners = nullptr; \
	}; \
	static OnRegisterListeners s_OnRegisterListeners; \
} \
void className::ComponentContainerImpl::Erase(ComponentOld* component) \
{ \
	ionassert(Container.find(component->GetGUID()) != Container.end()); \
	Container.erase(component->GetGUID()); \
} \
ComponentOld* className::ComponentContainerImpl::Find(ComponentOld* component) const \
{ \
	auto it = Container.find(component->GetGUID()); \
	if (it == Container.end()) \
		return nullptr; \
	const className* componentPtr = &it->second; \
	ionassert(componentPtr == component); \
	return component; \
} \
ComponentTypeID className::ComponentContainerImpl::GetTypeID() const \
{ \
	return className::GetTypeID(); \
} \
void* className::ComponentContainerImpl::GetRawContainer() \
{ \
	return &Container; \
} \
ComponentOld* className::ComponentContainerImpl::ForEachHelper(void*& inOutGenericIt) \
{ \
	RawComponentContainerType::iterator*& typedIt = (RawComponentContainerType::iterator*&)inOutGenericIt; \
	if (!typedIt) \
		typedIt = new RawComponentContainerType::iterator(Container.begin()); \
	if (*typedIt != Container.end()) \
		return &((*typedIt)++)->second; \
	/* Cleanup */ \
	delete typedIt; \
	typedIt = nullptr; \
	return nullptr; \
} \
/* ComponentOld class overrides */ \
ComponentOld* className::Duplicate_Internal(ComponentRegistry& registry) const \
{ \
	return (ComponentOld*)registry.DuplicateComponent(this); \
} \
/* Entity Component Property setup */ \
using NCPropsContianerType = THashSet<INCProperty*>; \
static NCPropsContianerType& _GetNCPropContainer() \
{ \
	static NCPropsContianerType* _NCProperties = new NCPropsContianerType; \
	return *_NCProperties; \
} \
static INCProperty* _FindNCProperty(INCProperty* prop) \
{ \
	return _GetNCPropContainer().find(prop) != _GetNCPropContainer().end() ? prop : nullptr; \
} \
static void _AddNCProperty(INCProperty* prop) \
{ \
	ionassert(!_FindNCProperty(prop), \
		"Property {0} already exists.", _FindNCProperty(prop)->GetDisplayName()); \
	_GetNCPropContainer().insert(prop); \
} \
/* Component instantiate function for runtime creation (type not known at compile-time). */ \
ComponentOld* FInstantiateComponent<className>::Call(ComponentRegistry* registry) \
{ \
	ionassert(registry); \
	IComponentContainer* icontainer = registry->GetContainer(className::GetTypeID()); \
	ionassert(icontainer); \
	void* raw = icontainer->GetRawContainer(); \
	/*ionassert(dynamic_cast<className::RawComponentContainerType*>(raw));*/ \
	className::RawComponentContainerType* container = (className::RawComponentContainerType*)raw; \
	return &ComponentRegistry::EmplaceComponentInContainer(*container); \
} \
/* Initializes the component container for the specified component registry. */ \
IComponentContainer* FInstantiateComponentContainer<className>::Call(ComponentRegistry* registry) \
{ \
	ionassert(registry); \
	return new className::ComponentContainerImpl; \
} \
/* Registers serialcall functions for this component type. */ \
void FRegisterSerialCallForComponent<className>::Call() \
{ \
	using namespace ComponentSerialCall::Private; \
	for (OnRegisterListeners::FPtr onRegister : s_OnRegisterListeners.GetListeners()) \
	{ \
		onRegister(); \
	} \
} \
/* Registers Entity Component Properties for this component type */ \
void FRegisterPropertiesForComponent<className>::Call() \
{ \
	ComponentDatabase* database = ComponentRegistry::GetComponentTypeDatabase_Internal(); \
	ComponentDatabase::TypeInfo& typeInfo = database->GetTypeInfo_Internal(className::GetTypeID()); \
	for (INCProperty* prop : _GetNCPropContainer()) \
	{ \
		typeInfo.EditableProperties.insert(prop); \
		prop->OnRegister(); \
	} \
}

/* Put in the component's .cpp file after the DECLARE_ENTITY_COMPONENT_CLASS, if the component implements Tick */
#define DECLARE_COMPONENT_SERIALCALL_TICK() \
DECLARE_COMPONENT_SERIALCALL_HELPER_EX(Tick, \
if (comp.IsTickEnabled()) \
	comp.Tick(deltaTime); \
, float deltaTime)

/* Put in the component's .cpp file after the DECLARE_ENTITY_COMPONENT_CLASS, if the component implements BuildRendererData */
#define DECLARE_COMPONENT_SERIALCALL_BUILDRENDERERDATA() \
DECLARE_COMPONENT_SERIALCALL_HELPER_EX(BuildRendererData, \
if (comp.IsSceneComponent()) \
	comp.BuildRendererData(data); \
, RRendererData& data)

/* For documentation purposes */
#define SERIALCALL

/* Declare Entity Component Property in the component class in the .h file */
#define DECLARE_NCPROPERTY(type, propName) \
type propName = type(); \
static TNCProperty<type, ComponentType> NCProperty_##propName;

/* Define Entity Component Property in the component .cpp file */
#define DEFINE_NCPROPERTY(propName, displayName) \
TNCProperty<decltype(ComponentType::propName), ComponentType> ComponentType::NCProperty_##propName = \
TNCProperty<decltype(ComponentType::propName), ComponentType>(#propName, displayName, &ComponentType::propName); \
CALL_OUTSIDE({ \
	_AddNCProperty(&ComponentType::NCProperty_##propName); \
}, propName##0);


/* Set optional property parameter (param names in TNCPropertyOptionalParams) */
#define NCPROPERTY_PARAM(propName, param, value) \
DECLARE_THASFIELD_EX(bUse##param, Use##param##propName); \
template<typename T> \
struct FNCPropertySetUse##param##propName \
{ \
	static void Call() \
	{ \
		if constexpr (THasField##Use##param##propName<TNCPropertyOptionalParams<T>>) \
		{ \
			ComponentType::NCProperty_##propName.OptionalParams.bUse##param = true; \
		} \
	} \
}; \
CALL_OUTSIDE({ \
	ComponentType::NCProperty_##propName.OptionalParams.param = value; \
	FNCPropertySetUse##param##propName<decltype(ComponentType::NCProperty_##propName)::Type>::Call(); \
}, propName##param);

namespace Ion
{
	REGISTER_LOGGER(ComponentLogger, "Engine::ECS::Component");

	DECLARE_THASFUNCTION(GetTypeID);

	// The hash of the Component class name
	using ComponentTypeID = size_t;
	constexpr ComponentTypeID InvalidComponentTypeID = (ComponentTypeID)-1;

	template<typename T>
	static constexpr bool TIsComponentTypeFinal = THasGetTypeID<T>;

	class ComponentOld;
	class ComponentRegistry;
	class IComponentContainer;

	// @TODO: Change this to something more efficient
	/* Container for Components of type T */
	template<typename T>
	using TCompContainer = THashMap<GUID, T>;

	using InstantiateComponentFPtr          = ComponentOld*(*)(ComponentRegistry*);
	using InstantiateComponentContainerFPtr = IComponentContainer*(*)(ComponentRegistry*);

	template<typename CompT>
	struct FInstantiateComponent
	{
		static ComponentOld* Call(ComponentRegistry*) { return nullptr; }
	};

	template<typename CompT>
	struct FInstantiateComponentContainer
	{
		static IComponentContainer* Call(ComponentRegistry*) { return nullptr; }
	};

	template<typename CompT>
	struct FRegisterSerialCallForComponent
	{
		static void Call() { }
	};

	template<typename CompT>
	struct FRegisterPropertiesForComponent
	{
		static void Call() { }
	};

	// Entity ComponentOld Property:

	enum class ENCPropertyType : uint8
	{
		None = 0,
		Bool,
		Int32,
		Int64,
		Float,
		Vector2,
		Vector3,
		Vector4,
	};

	template<typename T>
	constexpr ENCPropertyType NCPropertyTypeAsEnum = ENCPropertyType::None;

	template<> constexpr ENCPropertyType NCPropertyTypeAsEnum<bool>    = ENCPropertyType::Bool;
	template<> constexpr ENCPropertyType NCPropertyTypeAsEnum<int32>   = ENCPropertyType::Int32;
	template<> constexpr ENCPropertyType NCPropertyTypeAsEnum<int64>   = ENCPropertyType::Int64;
	template<> constexpr ENCPropertyType NCPropertyTypeAsEnum<float>   = ENCPropertyType::Float;
	template<> constexpr ENCPropertyType NCPropertyTypeAsEnum<Vector2> = ENCPropertyType::Vector2;
	template<> constexpr ENCPropertyType NCPropertyTypeAsEnum<Vector3> = ENCPropertyType::Vector3;
	template<> constexpr ENCPropertyType NCPropertyTypeAsEnum<Vector4> = ENCPropertyType::Vector4;

	template<typename T>
	using TNCPropertyOnChangeFunc = void(const T&);

	template<typename T>
	struct TNCPropertyOptionalParams
	{
		T DefaultValue;
		T MinValue;
		T MaxValue;
		union
		{
			uint64 PackedFlags;
			struct
			{
				uint64 bUseMinValue : 1;
				uint64 bUseMaxValue : 1;
			};
		};

		TNCPropertyOptionalParams() :
			DefaultValue(T()),
			MinValue(T()),
			MaxValue(T()),
			bUseMinValue(false),
			bUseMaxValue(false)
		{
		}
	};

	/* Entity ComponentOld Property interface */
	class INCProperty
	{
	public:
		virtual const String& GetMemberName() const = 0;
		virtual const String& GetDisplayName() const = 0;
		virtual ENCPropertyType GetType() const = 0;

	protected:
		virtual void OnRegister() = 0;

	private:
		virtual void Init(ComponentOld& component) = 0;
		
		template<typename T>
		friend struct FRegisterPropertiesForComponent;
		friend struct ComponentDatabase;
	};

	template<typename T>
	class TINCPropertyTyped : public INCProperty
	{
	public:
		virtual void Set(ComponentOld& object, const T& value) = 0;
		virtual const T& Get(ComponentOld& object) const = 0;

		virtual const TNCPropertyOptionalParams<T>& GetParams() const = 0;

		virtual ENCPropertyType GetType() const override
		{
			return NCPropertyTypeAsEnum<T>;
		}
	};

	/* Entity ComponentOld Property template
	   Instantiated once per class, not per object. */
	template<typename T, typename CompT>
	class TNCProperty : public TINCPropertyTyped<T>
	{
	public:
		using Type = T;
		using TOnChangeFunc = TFunction<TNCPropertyOnChangeFunc<T>>;

		TOnChangeFunc OnChanged;
		String MemberName;
		String DisplayName;
		T CompT::* MemberPtr;
		TNCPropertyOptionalParams<T> OptionalParams;

		TNCProperty(const String& memberName, const String& displayName, T CompT::* memberPtr) :
			MemberName(memberName),
			DisplayName(displayName),
			MemberPtr(memberPtr),
			OptionalParams()
		{
			//if constexpr (TIsConvertibleV<Lambda, TOnChangeFunc>)
			//{
			//	OnChanged = onChanged;
			//}
		}

		virtual void Set(ComponentOld& object, const T& value) override
		{
			//LOG_DEBUG("TNCProperty::Update [{0}::{1}]", CompT::ClassName, MemberName);
			T& member = ((CompT&)object).*MemberPtr = value;
			ClampValue(member);
			if (OnChanged)
				OnChanged(value);
		}

		virtual const T& Get(ComponentOld& object) const override
		{
			ionassert(dynamic_cast<CompT*>(&object));
			return ((CompT&)object).*MemberPtr;
		}

		virtual const String& GetMemberName() const override
		{
			return MemberName;
		}

		virtual const String& GetDisplayName() const override
		{
			return DisplayName;
		}

		virtual const TNCPropertyOptionalParams<T>& GetParams() const override
		{
			return OptionalParams;
		}

	protected:
		virtual void OnRegister() override
		{
			ComponentLogger.Debug("TNCProperty::OnRegister [{0}::{1}]", CompT::ClassName, MemberName);
		}

	private:
		/* Clamp the value only if the type can be compared */
		template<typename U, TEnableIfT<TTestOperatorLT<U, U>>* = 0>
		bool ClampValue(U& value)
		{
			if (OptionalParams.bUseMinValue && OptionalParams.bUseMaxValue &&
				OptionalParams.MaxValue < OptionalParams.MinValue)
			{
				//LOG_ERROR("TNCProperty<{0}, {1}>::Init -> MaxValue ({2}) is less than MinValue ({3}). Value will not be set.",
				//	TypeToString<T>, CompT::ClassName, OptionalParams.MaxValue, OptionalParams.MinValue);
				return false;
			}

			if (OptionalParams.bUseMinValue &&
				value < OptionalParams.MinValue)
			{
				//LOG_WARN("TNCProperty<{0}, {1}>::Init -> DefaultValue ({2}) is less than MinValue ({3}). Clamping.",
				//	TypeToString<T>, CompT::ClassName, OptionalParams.DefaultValue, OptionalParams.MinValue);
				value = OptionalParams.MinValue;
			}
			else if (OptionalParams.bUseMaxValue &&
				OptionalParams.MaxValue < value)
			{
				//LOG_WARN("TNCProperty<{0}, {1}>::Init -> DefaultValue ({2}) is greater than MaxValue ({3}). Clamping.",
				//	TypeToString<T>, CompT::ClassName, OptionalParams.DefaultValue, OptionalParams.MaxValue);
				value = OptionalParams.MaxValue;
			}
			return true;
		}
		template<typename U, TEnableIfT<!TTestOperatorLT<U, U>>* = 0>
		bool ClampValue(U& value)
		{
			return true;
		}

		virtual void Init(ComponentOld& component) override
		{
			T& member = ((CompT&)component).*MemberPtr;

			T defaultValue = OptionalParams.DefaultValue;
			if (ClampValue(defaultValue))
			{
				member = defaultValue;
			}
		}

		template<typename T>
		friend struct FRegisterPropertiesForComponent;
		friend struct ComponentDatabase;
	};

	// ComponentOld Database:

	struct ComponentDatabase
	{
		struct TypeInfo
		{
			ComponentTypeID ID;
			String ClassName;
			String ClassDisplayName;

			THashSet<INCProperty*> EditableProperties;

			union
			{
				uint64 PackedFlags;
				struct
				{
					uint64 bIsSceneComponent : 1;
				};
			};

			TypeInfo() :
				ID(InvalidComponentTypeID),
				ClassName("NULL"),
				ClassDisplayName("Unknown Component"),
				m_InstantiateType(nullptr),
				m_InstantiateContainer(nullptr),
				PackedFlags(0)
			{
			}

			template<typename T>
			bool Is() const
			{
				static_assert(TIsBaseOfV<ComponentOld, T> && TIsComponentTypeFinal<T>);
				return ID == T::GetTypeID();
			}

		private:
			InstantiateComponentFPtr m_InstantiateType;
			InstantiateComponentContainerFPtr m_InstantiateContainer;

			friend class ComponentRegistry;
		};

		THashMap<ComponentTypeID, TypeInfo> RegisteredTypes;

		void RegisterType(const TypeInfo& info)
		{
			RegisteredTypes.insert({ info.ID, info });
		}

		bool IsTypeRegistered(ComponentTypeID id) const
		{
			return RegisteredTypes.find(id) != RegisteredTypes.end();
		}

		const TypeInfo& GetTypeInfo(ComponentTypeID id) const
		{
			ionverify(IsTypeRegistered(id));
			return RegisteredTypes.find(id)->second;
		}

	private:
		TypeInfo& GetTypeInfo_Internal(ComponentTypeID id)
		{
			ionverify(IsTypeRegistered(id));
			return RegisteredTypes.find(id)->second;
		}

		void InitProperties(ComponentTypeID id, ComponentOld& component)
		{
			TypeInfo& typeInfo = GetTypeInfo_Internal(id);
			for (INCProperty* prop : typeInfo.EditableProperties)
			{
				prop->Init(component);
			}
		}

		template<typename T>
		friend struct FRegisterPropertiesForComponent;
		friend class ComponentRegistry;
	};

	DECLARE_COMPONENT_SERIALCALL(Tick, float);
	DECLARE_COMPONENT_SERIALCALL(BuildRendererData, RRendererData&);

	/* Abstract class */
	class ION_API ComponentOld
	{
	public:
		/* Called by the ComponentRegistry */
		virtual void OnCreate();
		/* Called by the ComponentRegistry */
		virtual void OnDestroy();

		ComponentOld* Duplicate() const;

		/* If bReparent is true, the children will get
		   reparented to the parent of this component.
		   Else they will get destroyed.
		   bReparent is unused on non-scene components. */
		void Destroy(bool bReparent = true);

		bool IsPendingKill() const;

		void SetTickEnabled(bool bTick);
		bool IsTickEnabled() const;

		bool IsSceneComponent() const;

		void SetName(const String& name);
		const String& GetName() const;

		/* Returns the Entity that owns the ComponentOld. */
		EntityOld* GetOwner() const;

		/* Returns the GUID of the ComponentOld.
		   A GUID is initiated at the creation of the ComponentOld. */
		const GUID& GetGUID() const;

		/* Returns a pointer to the World the ComponentOld is currently in. */
		World* GetWorldContext() const;

		bool operator==(const ComponentOld& other) const;
		bool operator!=(const ComponentOld& other) const;

		// Overriden in final classes by ENTITY_COMPONENT_CLASS_BODY

		virtual const String& GetClassDisplayName() const = 0;
		virtual const String& GetClassName() const = 0;
		virtual ComponentTypeID GetFinalTypeID() const = 0;
		virtual const ComponentDatabase::TypeInfo& GetFinalTypeInfo() const = 0;
		virtual ComponentOld* Duplicate_Internal(ComponentRegistry& registry) const = 0;

		// End of overriden in final classes ...

		template<typename T>
		bool IsOfType() const;
		bool IsOfType(ComponentTypeID id) const;

	protected:
		ComponentOld();

		void InitAsSceneComponent();

	public:
		template<typename CompT>
		struct Hasher
		{
			size_t operator()(const CompT& comp) const noexcept
			{
				return THash<GUID>()(comp.GetGUID());
			}
		};

	protected:
		GUID m_GUID;

	private:
		EntityOld* m_OwningEntity;
		World* m_WorldContext;

		uint8 m_bTickEnabled : 1;
		uint8 m_bIsSceneComponent : 1;
		uint8 m_bPendingKill : 1;

		String m_Name;

		friend class ComponentRegistry;
		friend class EntityOld;
	};

	class IComponentContainer
	{
	public:
		// No Insert function, only the ComponentOld Registry can
		// emplace components because the type has to be known.
		// FInstantiateComponent is used for that.

		virtual void Erase(ComponentOld* component) = 0;
		virtual ComponentOld* Find(ComponentOld* component) const = 0;

		virtual ComponentTypeID GetTypeID() const = 0;
		virtual void* GetRawContainer() = 0;

		template<typename Lambda>
		void ForEach(Lambda forEach);

	private:
		virtual ComponentOld* ForEachHelper(void*& inOutGenericIt) = 0;
	};

	/** Lambda type must be void(ComponentOld*) */
	template<typename Lambda>
	inline void IComponentContainer::ForEach(Lambda forEach)
	{
		// For Each lambda type check
		static_assert(TIsConvertibleV<Lambda, TFunction<void(ComponentOld*)>>);
		
		void* genericIt = nullptr;
		while (ComponentOld* comp = ForEachHelper(genericIt))
		{
			forEach(comp);
		}
	}

	class ION_API ComponentRegistry
	{
	public:
		ComponentRegistry(World* worldContext);
		~ComponentRegistry();

		static void RegisterComponents();
		template<typename CompT>
		static ComponentTypeID RegisterComponentClass();

		template<typename CompT, typename... Args>
		CompT* CreateComponent(Args&&... args);
		template<typename... Args>
		ComponentOld* CreateComponent(ComponentTypeID id, Args&&... args);

		template<typename CompT>
		CompT* DuplicateComponent(const CompT* other);

		template<typename CompT, TEnableIfT<TIsComponentTypeFinal<CompT>>* = 0>
		void DestroyComponent(CompT* component);
		template<typename CompT, TEnableIfT<!TIsComponentTypeFinal<CompT>>* = 0>
		void DestroyComponent(CompT* component);

		ComponentOld* FindComponentByGUID(const GUID& guid) const;

		/** Lambda type must be void(ComponentOld*) */
		template<typename Lambda>
		void ForEachComponentOfType(ComponentTypeID id, Lambda forEach);

		/** Sequentially calls ForEachComponentOfType for every component type
		  * that has its container initialized.
		  * Lambda type must be void(ComponentTypeID, ComponentOld*) */
		template<typename Lambda>
		void ForEachComponent(Lambda forEach);

		/** Sequentially calls ForEachComponentOfType for every
		  * scene component type that has its container initialized.
		  * Lambda type must be void(SceneComponent*) */
		template<typename Lambda>
		void ForEachSceneComponent(Lambda forEach);

		void MarkForDestroy(ComponentOld* component);

		void DestroyInvalidComponents();

		void Update(float deltaTime);
		void BuildRendererData(RRendererData& data);

		static const ComponentDatabase* GetComponentTypeDatabase();

		static const THashMap<ComponentTypeID, ComponentDatabase::TypeInfo>& GetRegisteredTypes();

	private:
		/* For internal use */
		template<typename CompT, typename... Args>
		static CompT& EmplaceComponentInContainer(TCompContainer<CompT>& container, Args&&... args);
		template<typename CompT>
		static CompT& EmplaceDuplicateComponentInContainer(TCompContainer<CompT>& container, const CompT* component);

	private:
		template<typename CompT>
		void InitializeComponentContainter();
		void InitializeComponentContainter(ComponentTypeID id);

		template<typename CompT>
		bool IsContainerInitialized() const;
		bool IsContainerInitialized(ComponentTypeID id) const ;

		template<typename CompT>
		typename CompT::RawComponentContainerType& GetRawContainer();

		IComponentContainer* GetContainer(ComponentTypeID id);

		static ComponentDatabase* GetComponentTypeDatabase_Internal();

	private:
		THashMap<ComponentTypeID, IComponentContainer*> m_Containers;
		THashMap<ComponentTypeID, TArray<ComponentOld*>> m_InvalidComponents;
		THashMap<GUID, ComponentOld*> m_ComponentsByGUID;

		World* m_WorldContext;

		template<typename T>
		friend struct FInstantiateComponent;
		template<typename T>
		friend struct FInstantiateComponentContainer;
		template<typename T>
		friend struct FRegisterSerialCallForComponent;
		template<typename T>
		friend struct FRegisterPropertiesForComponent;
	};

	// ComponentOld inline definitions

	inline bool ComponentOld::IsTickEnabled() const
	{
		return m_bTickEnabled;
	}
	
	inline bool ComponentOld::IsSceneComponent() const
	{
		return m_bIsSceneComponent;
	}

	inline void ComponentOld::SetName(const String& name)
	{
		m_Name = name;
	}

	inline const String& ComponentOld::GetName() const
	{
		return m_Name;
	}

	inline EntityOld* ComponentOld::GetOwner() const
	{
		return m_OwningEntity;
	}

	inline const GUID& ComponentOld::GetGUID() const
	{
		return m_GUID;
	}

	inline World* ComponentOld::GetWorldContext() const
	{
		return m_WorldContext;
	}

	inline bool ComponentOld::IsPendingKill() const
	{
		return m_bPendingKill;
	}

	inline bool ComponentOld::operator==(const ComponentOld& other) const
	{
		return m_GUID == other.m_GUID;
	}

	inline bool ComponentOld::operator!=(const ComponentOld& other) const
	{
		return m_GUID != other.m_GUID;
	}

	// ComponentRegistry inline definitions

	template<typename CompT>
	inline ComponentTypeID ComponentRegistry::RegisterComponentClass()
	{
		static_assert(TIsBaseOfV<ComponentOld, CompT> && TIsComponentTypeFinal<CompT>);

		ComponentLogger.Debug("ComponentRegistry::RegisterComponentClass<{0}>", CompT::ClassName);

		ComponentDatabase* typeDB = GetComponentTypeDatabase_Internal();
		auto it = typeDB->RegisteredTypes.find(CompT::GetTypeID());
		// Don't register the same component twice.
		if (it != typeDB->RegisteredTypes.end())
		{
			ionverify(it->second.ClassName == CompT::ClassName,
				"The class name hash already exists. Use a different class name."); // If it ever happens
			return CompT::GetTypeID();
		}

		ComponentDatabase::TypeInfo typeInfo;
		typeInfo.ID = CompT::GetTypeID();
		typeInfo.ClassName = CompT::ClassName;
		typeInfo.ClassDisplayName = CompT::ClassDisplayName;
		typeInfo.bIsSceneComponent = TIsConvertibleV<CompT*, SceneComponent*>;
		typeInfo.m_InstantiateType = FInstantiateComponent<CompT>::Call;
		typeInfo.m_InstantiateContainer = FInstantiateComponentContainer<CompT>::Call;

		typeDB->RegisterType(typeInfo);

		FRegisterSerialCallForComponent<CompT>::Call();
		FRegisterPropertiesForComponent<CompT>::Call();

		return CompT::GetTypeID();
	}

	/* Compile-time version */
	template<typename CompT, typename... Args>
	inline CompT* ComponentRegistry::CreateComponent(Args&&... args)
	{
		static_assert(TIsBaseOfV<ComponentOld, CompT>);
		using CompContainer = typename CompT::RawComponentContainerType;

		// Initialize the array if it's the first component of that type
		if (!IsContainerInitialized<CompT>())
		{
			InitializeComponentContainter<CompT>();
		}

		CompContainer& container = GetRawContainer<CompT>();

		ComponentDatabase* database = GetComponentTypeDatabase_Internal();

		CompT& component = EmplaceComponentInContainer(container, Forward<Args>(args)...);

		m_ComponentsByGUID[component.GetGUID()] = &component;

		component.m_WorldContext = m_WorldContext;
		component.m_Name = CompT::ClassDisplayName;

		database->InitProperties(CompT::GetTypeID(), component);

		component.OnCreate();

		return &component;
	}

	/* Runtime version */
	template<typename... Args>
	inline ComponentOld* ComponentRegistry::CreateComponent(ComponentTypeID id, Args&&... args)
	{
		// Initialize the array if it's the first component of that type
		if (!IsContainerInitialized(id))
		{
			InitializeComponentContainter(id);
		}
		
		ComponentDatabase* database = GetComponentTypeDatabase_Internal();

		InstantiateComponentFPtr instantiateFPtr = database->GetTypeInfo(id).m_InstantiateType;
		ionverify(instantiateFPtr);
		// This will add the component to the registry container
		ComponentOld* componentPtr = instantiateFPtr(this);
		ionverify(componentPtr);

		m_ComponentsByGUID[componentPtr->GetGUID()] = componentPtr;

		componentPtr->m_WorldContext = m_WorldContext;
		componentPtr->m_Name = componentPtr->GetClassDisplayName();

		database->InitProperties(id, *componentPtr);

		componentPtr->OnCreate();

		return componentPtr;
	}

	template<typename CompT>
	inline CompT* ComponentRegistry::DuplicateComponent(const CompT* other)
	{
		static_assert(TIsBaseOfV<ComponentOld, CompT>);
		using CompContainer = typename CompT::RawComponentContainerType;

		CompContainer& container = GetRawContainer<CompT>();

		ComponentDatabase* database = GetComponentTypeDatabase_Internal();

		CompT& component = EmplaceDuplicateComponentInContainer(container, other);

		m_ComponentsByGUID[component.GetGUID()] = &component;

		return &component;
	}

	/* Compile-time version */
	template<typename CompT, TEnableIfT<TIsComponentTypeFinal<CompT>>*>
	inline void ComponentRegistry::DestroyComponent(CompT* component)
	{
		static_assert(TIsBaseOfV<ComponentOld, CompT>);
		using CompContainer = CompT::RawComponentContainerType;

		if (component)
		{
			ionassert(IsContainerInitialized<CompT>());

			CompContainer& container = GetRawContainer<CompT>();

			GUID guid = component->GetGUID();

			auto it = container.find(guid);
			if (it == container.end())
			{
				ComponentLogger.Error("Component does not exist in the registry.\nGUID = {0}", component->GetGUID());
				return;
			}
			CompT* componentPtr = &it->second;
			ionassert(componentPtr == component);
			component->OnDestroy();

			container.erase(guid);
			m_ComponentsByGUID.erase(guid);
		}
	}

	/* Runtime version (if the component type is not known at compile time) */
	template<typename CompT, TEnableIfT<!TIsComponentTypeFinal<CompT>>*>
	inline void ComponentRegistry::DestroyComponent(CompT* component)
	{
		static_assert(TIsBaseOfV<ComponentOld, CompT>);

		if (component)
		{
			ComponentTypeID typeId = component->GetFinalTypeID();

			ionassert(IsContainerInitialized(typeId));

			IComponentContainer* container = GetContainer(typeId);
			if (ComponentOld* found = container->Find(component))
			{
				ionassert(found == component);
				component->OnDestroy();

				m_ComponentsByGUID.erase(component->GetGUID());
				container->Erase(component);
			}
			else
			{
				ComponentLogger.Error("Component does not exist in the registry.\nGUID = {0}", component->GetGUID());
			}
		}
	}

	template<typename Lambda>
	inline void ComponentRegistry::ForEachComponentOfType(ComponentTypeID id, Lambda forEach)
	{
		if (!IsContainerInitialized(id))
			return;

		return GetContainer(id)->ForEach(forEach);
	}

	template<typename Lambda>
	inline void ComponentRegistry::ForEachComponent(Lambda forEach)
	{
		static_assert(TIsConvertibleV<Lambda, TFunction<void(ComponentTypeID, ComponentOld*)>>);

		const ComponentDatabase* database = GetComponentTypeDatabase();
		for (auto& [id, type] : database->RegisteredTypes)
		{
			ForEachComponentOfType(id, [id, &forEach](ComponentOld* component)
			{
				forEach(id, component);
			});
		}
	}

	template<typename Lambda>
	inline void ComponentRegistry::ForEachSceneComponent(Lambda forEach)
	{
		static_assert(TIsConvertibleV<Lambda, TFunction<void(SceneComponent*)>>);

		const ComponentDatabase* database = GetComponentTypeDatabase();
		for (auto& [id, type] : database->RegisteredTypes)
		{
			if (!type.bIsSceneComponent)
				continue;

			ForEachComponentOfType(id, [id, &forEach](ComponentOld* component)
			{
				forEach((SceneComponent*)component);
			});
		}
	}

	inline ComponentDatabase* ComponentRegistry::GetComponentTypeDatabase_Internal()
	{
		static ComponentDatabase* c_Database = new ComponentDatabase;
		return c_Database;
	}

	inline const ComponentDatabase* ComponentRegistry::GetComponentTypeDatabase()
	{
		return GetComponentTypeDatabase_Internal();
	}

	template<typename CompT>
	inline void ComponentRegistry::InitializeComponentContainter()
	{
		static_assert(TIsBaseOfV<ComponentOld, CompT>);
		using CompContainer = CompT::ComponentContainerImpl;

		ComponentLogger.Debug("ComponentRegistry::InitializeComponentContainter<{0}>", CompT::ClassName);

		ionassert(!IsContainerInitialized<CompT>());

		CompContainer* container = new CompContainer;
		m_Containers.insert({ CompT::GetTypeID(), container });
		m_InvalidComponents.insert({ CompT::GetTypeID(), TArray<ComponentOld*>() });
	}

	inline void ComponentRegistry::InitializeComponentContainter(ComponentTypeID id)
	{
		ionassert(!IsContainerInitialized(id));

		ComponentDatabase* database = GetComponentTypeDatabase_Internal();
		ionassert(database);

		ComponentLogger.Debug("ComponentRegistry::InitializeComponentContainter({0}) <- Runtime", database->GetTypeInfo(id).ClassName);

		InstantiateComponentContainerFPtr instantiateContianerFPtr = database->GetTypeInfo(id).m_InstantiateContainer;
		ionverify(instantiateContianerFPtr);
		IComponentContainer* containerPtr = instantiateContianerFPtr(this);
		ionverify(containerPtr);

		m_Containers.insert({ containerPtr->GetTypeID(), containerPtr });
		m_InvalidComponents.insert({ containerPtr->GetTypeID(), TArray<ComponentOld*>() });
	}

	template<typename CompT>
	inline bool ComponentRegistry::IsContainerInitialized() const
	{
		return IsContainerInitialized(CompT::GetTypeID());
	}

	inline bool ComponentRegistry::IsContainerInitialized(ComponentTypeID id) const
	{
		return m_Containers.find(id) != m_Containers.end();
	}

	template<typename CompT>
	typename CompT::RawComponentContainerType& ComponentRegistry::GetRawContainer()
	{
		static_assert(TIsBaseOfV<ComponentOld, CompT>);
		using ContainerT = CompT::ComponentContainerImpl;
		using RawContainerT = CompT::RawComponentContainerType;

		ionassert(IsContainerInitialized<CompT>());

		ContainerT* containerPtr = (ContainerT*)m_Containers.at(CompT::GetTypeID());
		return containerPtr->Container;
	}

	inline IComponentContainer* ComponentRegistry::GetContainer(ComponentTypeID id)
	{
		ionassert(IsContainerInitialized(id));
		return m_Containers.at(id);
	}

	inline const THashMap<ComponentTypeID, ComponentDatabase::TypeInfo>& ComponentRegistry::GetRegisteredTypes()
	{
		const ComponentDatabase* database = GetComponentTypeDatabase();
		return database->RegisteredTypes;
	}

	template<typename CompT, typename... Args>
	CompT& ComponentRegistry::EmplaceComponentInContainer(TCompContainer<CompT>& container, Args&&... args)
	{
		CompT component(Forward<Args>(args)...);
		auto [it, bUnique] = container.emplace(component.GetGUID(), Move(component));
		ionassert(bUnique, "GUID collision?");
		return it->second;
	}

	template<typename CompT>
	CompT& ComponentRegistry::EmplaceDuplicateComponentInContainer(TCompContainer<CompT>& container, const CompT* component)
	{
		CompT newComponent(*component);
		// Make sure the new component has a different GUID
		newComponent.m_GUID = GUID();
		auto [it, bUnique] = container.emplace(newComponent.GetGUID(), Move(newComponent));
		ionassert(bUnique, "GUID collision?");
		return it->second;
	}

	template<typename T>
	inline bool ComponentOld::IsOfType() const
	{
		static_assert(TIsBaseOfV<ComponentOld, T> && TIsComponentTypeFinal<T>);
		return GetFinalTypeID() == T::GetTypeID();
	}

	inline bool ComponentOld::IsOfType(ComponentTypeID id) const
	{
		return GetFinalTypeID() == id;
	}
}
