#pragma once

#define DECLARE_COMPONENT_SERIALCALL(func, ...) \
DECLARE_THASFUNCTION(func); \
namespace ComponentSerialCall \
{ \
	namespace Private \
	{ \
		/* Function signature */ \
		using func##FPtr = void(*)(void*, __VA_ARGS__); \
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
	inline void func(ComponentTypeID typeId, void* container, Args&&... args) \
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
		static auto CallEach##func = [](void* containerPtr, __VA_ARGS__) \
		{ \
			/* Make sure the type actually has the function, just in case. */ \
			if constexpr (THas##func<ThisComponentClass>) \
			{ \
				TRACE_SCOPE(func##TracerName); \
				func##Container& container = *(func##Container*)containerPtr; \
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
static inline ComponentTypeID s_TypeID = -1; \
protected: \
public: \
static inline ComponentTypeID GetTypeID() \
{ \
	if (s_TypeID == -1) \
		s_TypeID = ComponentRegistry::RegisterComponentClass<className>(); \
	return s_TypeID; \
} \
static inline const String ClassName = #className; \
static inline const String ClassDisplayName = displayName; \
virtual const String& GetClassName() const override { return ClassName; } \
virtual const String& GetClassDisplayName() const override { return ClassDisplayName; } \
friend class ComponentRegistry;

/* Put in the component's .cpp file (one per file) */
#define DECLARE_ENTITY_COMPONENT_CLASS(component) \
namespace ComponentSerialCall \
{ \
	namespace Private \
	{ \
		using ThisComponentClass = component; \
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

namespace Ion
{
	using ComponentTypeID = int16;

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
		// Overriden in final classes by ENTITY_COMPONENT_CLASS_BODY
		virtual const String& GetClassName() const = 0; // @TODO: Get rid of these Windows macros

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

	// @TODO: Change this to something more efficient
	/* Container for Components of type T */
	template<typename T>
	using TCompContainer = THashMap<GUID, T>;

	class ION_API ComponentRegistry
	{
		struct StaticComponentTypeData
		{
			THashSet<ComponentTypeID> RegisteredTypes;
			ComponentTypeID ComponentIDCounter = 0;
		};

	public:
		using ComponentContainer = void;

		ComponentRegistry(World* worldContext);
		~ComponentRegistry();

		template<typename CompT, typename... Args>
		CompT* CreateComponent(Args&&... args);

		template<typename CompT>
		void DestroyComponent(CompT* component);

		template<>
		void DestroyComponent(Component* component);

		// Does nothing
		template<typename CompT>
		void BindComponentToEntity(Entity* entity, CompT* component) {}
		// Does nothing
		template<typename CompT>
		void UnbindComponentFromEntity(Entity* entity, CompT* component) {}

		template<typename CompT>
		static ComponentTypeID RegisterComponentClass();

		void Update(float deltaTime);
		void BuildRendererData(RRendererData& data);

	private:
		template<typename CompT>
		void InitializeComponentContainter();

		template<typename ContainerT>
		ContainerT& GetContainer();

		static StaticComponentTypeData* GetComponentTypeData();

	private:
		THashMap<ComponentTypeID, ComponentContainer*> m_Containers;
		THashSet<ComponentTypeID> m_InitializedTypes;

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
		using CompContainer = TCompContainer<CompT>;

		// Initialize the array if it's the first component of that type
		if (m_InitializedTypes.find(CompT::s_TypeID) == m_InitializedTypes.end())
		{
			InitializeComponentContainter<CompT>();
		}

		CompContainer& container = GetContainer<CompContainer>();

		CompT component(Forward<Args>(args)...);
		auto [it, _] = container.emplace(component.GetGUID(), Move(component));

		CompT* componentPtr = &(*it).second;
		componentPtr->m_WorldContext = m_WorldContext;
		componentPtr->m_Name = CompT::ClassDisplayName;

		componentPtr->OnCreate();

		return componentPtr;
	}

	template<typename CompT>
	inline void ComponentRegistry::DestroyComponent(CompT* component)
	{
		static_assert(TIsBaseOfV<Component, CompT>);
		using CompContainer = TCompContainer<CompT>;
		const ComponentTypeID TypeID = CompT::s_TypeID;

		if (component)
		{
			CompContainer& container = GetContainer<CompContainer>();

			GUID guid = component->GetGUID();

			auto it = container.find(guid);
			if (it == container.end())
			{
				LOG_WARN("Component does not exist in the registry.\nGUID = {0}", component->GetGUID());
				return;
			}
			CompT* componentPtr = &(*it).second;

			component->OnDestroy();

			container.erase(guid);
		}
	}

	template<typename CompT>
	inline ComponentTypeID ComponentRegistry::RegisterComponentClass()
	{
		static_assert(TIsBaseOfV<Component, CompT>);

		StaticComponentTypeData* componentTypeData = GetComponentTypeData();

		CompT::s_TypeID = componentTypeData->ComponentIDCounter;
		//componentTypeData->RegisteredTypes.insert(s_ComponentIDCounter);

		return componentTypeData->ComponentIDCounter++;
	}

	inline ComponentRegistry::StaticComponentTypeData* ComponentRegistry::GetComponentTypeData()
	{
		// I don't know why, but the set has to by dynamically allocated
		// or the insert crashes the thing... Don't ask.
		static StaticComponentTypeData* c_ComponentTypeData = new StaticComponentTypeData;
		return c_ComponentTypeData;
	}

	template<typename CompT>
	inline void ComponentRegistry::InitializeComponentContainter()
	{
		static_assert(TIsBaseOfV<Component, CompT>);
		using CompContainer = TCompContainer<CompT>;

		m_InitializedTypes.insert(CompT::s_TypeID);
		CompContainer* container = new CompContainer;
		m_Containers.insert({ CompT::s_TypeID, container });
	}

	template<typename ContainerT>
	ContainerT& ComponentRegistry::GetContainer()
	{
		using CompT = ContainerT::value_type::second_type;
		static_assert(TIsBaseOfV<Component, CompT>);

		ComponentContainer* containerPtr = m_Containers.at(CompT::s_TypeID);
		return *(ContainerT*)containerPtr;
	}
}
