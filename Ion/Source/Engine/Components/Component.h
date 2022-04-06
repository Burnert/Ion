#pragma once

#define DECLARE_COMPONENT_SERIALCALL(func, ...) \
DECLARE_THASFUNCTION(func); \
namespace ComponentSerialCall \
{ \
	namespace Private \
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
		/* Called in ENTITY_COMPONENT_SERIALCALL_HELPER */ \
		inline int32 func##_AddFunctionForType(ComponentTypeID id, func##FPtr fn) \
		{ \
			g_##func##Functions[id] = fn; \
			return 0; \
		} \
	} \
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
namespace ComponentSerialCall \
{ \
	namespace Private \
	{ \
		/* Type of the this component container */ \
		using func##Container = TCompContainer<ThisComponentClass>; /* THashMap<GUID, _ComponentClass_> */ \
		static char func##TracerName[256] = "DECLARE_COMPONENT_SERIALCALL_HELPER_EX("#func") - "; \
		CALL_OUTSIDE({ strcat_s(func##TracerName, ThisComponentClass::ClassDisplayName.c_str()); }, func##0); \
		/* Function that calls all the components' serialcall functions in the specified component container. 
		   It has to be a lambda to avoid symbol redefinition. */ \
		static auto CallEach##func = [](Ion::IComponentContainer* containerPtr, __VA_ARGS__) \
		{ \
			/* Make sure the type actually has the function, just in case. */ \
			if constexpr (THas##func<ThisComponentClass>) \
			{ \
				TRACE_SCOPE(func##TracerName); \
				func##Container& container = *(func##Container*)containerPtr->GetRawContainer(); \
				/* Call the serialcall functions */ \
				for (auto& [k, comp] : container) \
				{ \
					forEachBody; \
				} \
			} \
		}; \
		CALL_OUTSIDE({ func##_AddFunctionForType(ThisComponentClass::GetTypeID(), CallEach##func); }, func##1); \
	} \
}

/* Generates the code used to call serialcall functions on all components. */
#define ENTITY_COMPONENT_SERIALCALL_HELPER(func) \
DECLARE_COMPONENT_SERIALCALL_HELPER_EX(func, comp.func())

/* Put in the component's class in the .h file */
#define ENTITY_COMPONENT_CLASS_BODY(className, displayName) \
private: \
static inline ComponentTypeID s_TypeID = InvalidComponentTypeID; \
protected: \
public: \
static inline ComponentTypeID GetTypeID() \
{ \
	if (s_TypeID == InvalidComponentTypeID) \
		ComponentRegistry::RegisterComponentClass<className>(THash<String>()(#className)); \
	return s_TypeID; \
} \
using RawComponentContainerType = TCompContainer<className>; \
/* Container wrapper that will be inserted into the Component Registry */ \
class ION_API ComponentContainerImpl : public IComponentContainer \
{ \
public: \
	using Type          = ComponentContainerImpl; \
	using ComponentT    = className; \
	using RawContainerT = RawComponentContainerType; \
	virtual void Erase(Component* component) override; \
	virtual Component* Find(Component* component) const override; \
	virtual ComponentTypeID GetTypeID() const override; \
	virtual void* GetRawContainer() override; \
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
friend class ComponentRegistry;

DECLARE_THASFUNCTION(GetTypeID);

/* Put in the component's .cpp file (one per file) */
#define DECLARE_ENTITY_COMPONENT_CLASS(className) \
namespace ComponentSerialCall \
{ \
	namespace Private \
	{ \
		using ThisComponentClass = className; \
	} \
} \
void className::ComponentContainerImpl::Erase(Component* component) \
{ \
	ionassert(Container.find(component->GetGUID()) != Container.end()); \
	Container.erase(component->GetGUID()); \
} \
Component* className::ComponentContainerImpl::Find(Component* component) const \
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

namespace Ion
{
	// The hash of the Component class name
	using ComponentTypeID = size_t;
	constexpr ComponentTypeID InvalidComponentTypeID = (ComponentTypeID)-1;

	// @TODO: Change this to something more efficient
	/* Container for Components of type T */
	template<typename T>
	using TCompContainer = THashMap<GUID, T>;

	struct ComponentDatabase
	{
		struct TypeInfo
		{
			ComponentTypeID ID;
			String ClassName;
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
			ionassertnd(IsTypeRegistered(id));
			return RegisteredTypes.find(id)->second;
		}
	};

	DECLARE_COMPONENT_SERIALCALL(Tick, float);
	DECLARE_COMPONENT_SERIALCALL(BuildRendererData, RRendererData&);

	/* Abstract class */
	class ION_API Component
	{
	public:
		/* Called by the ComponentRegistry */
		virtual void OnCreate() { }
		/* Called by the ComponentRegistry */
		virtual void OnDestroy() { }

		/* If bReparent is true, the children will get
		   reparented to the parent of this component.
		   Else they will get destroyed.
		   bReparent is unused on non-scene components. */
		void Destroy(bool bReparent = true);

		void SetTickEnabled(bool bTick);
		bool IsTickEnabled() const;

		bool IsSceneComponent() const;

		void SetName(const String& name);
		const String& GetName() const;

		/* Returns the Entity that owns the Component. */
		Entity* GetOwner() const;

		/* Returns the GUID of the Component.
		   A GUID is initiated at the creation of the Component. */
		const GUID& GetGUID() const;

		/* Returns a pointer to the World the Component is currently in. */
		World* GetWorldContext() const;

		bool operator==(const Component& other) const;
		bool operator!=(const Component& other) const;

		// Overriden in final classes by ENTITY_COMPONENT_CLASS_BODY

		virtual const String& GetClassDisplayName() const = 0;
		virtual const String& GetClassName() const = 0; // @TODO: Get rid of these Windows macros
		virtual ComponentTypeID GetFinalTypeID() const = 0;
		virtual const ComponentDatabase::TypeInfo& GetFinalTypeInfo() const = 0;

		// End of overriden in final classes ...

	protected:
		Component();

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
		Entity* m_OwningEntity;
		World* m_WorldContext;

		uint8 m_bTickEnabled : 1;
		uint8 m_bIsSceneComponent : 1;
		//uint8 m_bUpdateSceneData : 1;

		String m_Name;

		friend class ComponentRegistry;
		friend class Entity;
	};

	class IComponentContainer
	{
	public:
		// No Insert function, only the Component Registry can
		// emplace components because the type has to be known

		virtual void Erase(Component* component) = 0;
		virtual Component* Find(Component* component) const = 0;

		virtual ComponentTypeID GetTypeID() const = 0;
		virtual void* GetRawContainer() = 0;
	};

	template<typename T>
	static constexpr bool TIsComponentTypeFinal = THasGetTypeID<T>;

	class ION_API ComponentRegistry
	{
	public:
		ComponentRegistry(World* worldContext);
		~ComponentRegistry();

		template<typename CompT, typename... Args>
		CompT* CreateComponent(Args&&... args);

		template<typename CompT, TEnableIfT<TIsComponentTypeFinal<CompT>>* = 0>
		void DestroyComponent(CompT* component);
		template<typename CompT, TEnableIfT<!TIsComponentTypeFinal<CompT>>* = 0>
		void DestroyComponent(CompT* component);

		template<typename CompT>
		static ComponentTypeID RegisterComponentClass(size_t classNameHash);

		void Update(float deltaTime);
		void BuildRendererData(RRendererData& data);

		static const ComponentDatabase* GetComponentTypeDatabase();

	private:
		template<typename CompT>
		void InitializeComponentContainter();

		template<typename CompT>
		bool IsContainerInitialized() const;
		bool IsContainerInitialized(ComponentTypeID id) const ;

		template<typename CompT>
		typename CompT::RawComponentContainerType& GetRawContainer();

		IComponentContainer* GetContainer(ComponentTypeID id);

		template<typename CompT, typename... Args>
		CompT& EmplaceComponentInContainer(TCompContainer<CompT>& container, Args&&... args);

		static ComponentDatabase* GetComponentTypeDatabase_Internal();

	private:
		THashMap<ComponentTypeID, IComponentContainer*> m_Containers;

		World* m_WorldContext;
	};

	// Component inline definitions

	inline bool Component::IsTickEnabled() const
	{
		return m_bTickEnabled;
	}
	
	inline bool Component::IsSceneComponent() const
	{
		return m_bIsSceneComponent;
	}

	inline void Component::SetName(const String& name)
	{
		m_Name = name;
	}

	inline const String& Component::GetName() const
	{
		return m_Name;
	}

	inline Entity* Component::GetOwner() const
	{
		return m_OwningEntity;
	}

	inline const GUID& Component::GetGUID() const
	{
		return m_GUID;
	}

	inline World* Component::GetWorldContext() const
	{
		return m_WorldContext;
	}

	inline bool Component::operator==(const Component& other) const
	{
		return m_GUID == other.m_GUID;
	}

	inline bool Component::operator!=(const Component& other) const
	{
		return m_GUID != other.m_GUID;
	}

	// ComponentRegistry inline definitions

	template<typename CompT, typename... Args>
	inline CompT* ComponentRegistry::CreateComponent(Args&&... args)
	{
		static_assert(TIsBaseOfV<Component, CompT>);
		using CompContainer = typename CompT::RawComponentContainerType;

		// Initialize the array if it's the first component of that type
		if (!IsContainerInitialized<CompT>())
		{
			InitializeComponentContainter<CompT>();
		}

		CompContainer& container = GetRawContainer<CompT>();

		CompT& componentPtr = EmplaceComponentInContainer(container, Forward<Args>(args)...);
		componentPtr.m_WorldContext = m_WorldContext;
		componentPtr.m_Name = CompT::ClassDisplayName;

		componentPtr.OnCreate();

		return &componentPtr;
	}

	/* Compile-time version */
	template<typename CompT, TEnableIfT<TIsComponentTypeFinal<CompT>>*>
	inline void ComponentRegistry::DestroyComponent(CompT* component)
	{
		static_assert(TIsBaseOfV<Component, CompT>);
		using CompContainer = CompT::RawComponentContainerType;

		if (component)
		{
			ionassert(IsContainerInitialized<CompT>());

			CompContainer& container = GetRawContainer<CompT>();

			GUID guid = component->GetGUID();

			auto it = container.find(guid);
			if (it == container.end())
			{
				LOG_ERROR("Component does not exist in the registry.\nGUID = {0}", component->GetGUID());
				return;
			}
			CompT* componentPtr = &it->second;
			ionassert(componentPtr == component);
			component->OnDestroy();

			container.erase(guid);
		}
	}

	/* Runtime version (if the component type is not known at compile time) */
	template<typename CompT, TEnableIfT<!TIsComponentTypeFinal<CompT>>*>
	inline void ComponentRegistry::DestroyComponent(CompT* component)
	{
		static_assert(TIsBaseOfV<Component, CompT>);

		if (component)
		{
			ComponentTypeID typeId = component->GetFinalTypeID();

			ionassert(IsContainerInitialized(typeId));

			IComponentContainer* container = GetContainer(typeId);
			if (Component* found = container->Find(component))
			{
				ionassert(found == component);
				component->OnDestroy();

				container->Erase(component);
			}
			else
			{
				LOG_ERROR("Component does not exist in the registry.\nGUID = {0}", component->GetGUID());
			}
		}
	}

	template<typename CompT>
	inline ComponentTypeID ComponentRegistry::RegisterComponentClass(size_t classNameHash)
	{
		static_assert(TIsBaseOfV<Component, CompT>);

		ComponentDatabase* typeDB = GetComponentTypeDatabase_Internal();
		ionassertnd(typeDB->RegisteredTypes.find(classNameHash) == typeDB->RegisteredTypes.end(),
			"The class name hash already exists. Use a different class name."); // If it ever happens

		CompT::s_TypeID = classNameHash;

		ComponentDatabase::TypeInfo typeInfo;
		typeInfo.ID = classNameHash;
		typeInfo.ClassName = CompT::ClassName;

		typeDB->RegisterType(typeInfo);

		return classNameHash;
	}

	inline ComponentDatabase* ComponentRegistry::GetComponentTypeDatabase_Internal()
	{
		// I don't know why, but the set has to by dynamically allocated
		// or the insert crashes the thing... Don't ask.
		static ComponentDatabase* c_ComponentTypeData = new ComponentDatabase;
		return c_ComponentTypeData;
	}

	inline const ComponentDatabase* ComponentRegistry::GetComponentTypeDatabase()
	{
		return GetComponentTypeDatabase_Internal();
	}

	template<typename CompT>
	inline void ComponentRegistry::InitializeComponentContainter()
	{
		static_assert(TIsBaseOfV<Component, CompT>);
		using CompContainer = CompT::ComponentContainerImpl;

		ionassert(!IsContainerInitialized<CompT>());

		CompContainer* container = new CompContainer;
		m_Containers.insert({ CompT::GetTypeID(), container });
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
		static_assert(TIsBaseOfV<Component, CompT>);
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

	template<typename CompT, typename... Args>
	CompT& ComponentRegistry::EmplaceComponentInContainer(TCompContainer<CompT>& container, Args&&... args)
	{
		CompT component(Forward<Args>(args)...);
		auto [it, bUnique] = container.emplace(component.GetGUID(), Move(component));
		ionassert(bUnique, "GUID collision?");
		return it->second;
	}
}
