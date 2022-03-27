#pragma once

#define DECLARE_COMPONENT_SERIALCALL(func, ...) \
DECLARE_THASFUNCTION(func); \
namespace ComponentStaticCallbacks \
{ \
	using func##FPtr = void(*)(void*, __VA_ARGS__); \
	/* Components which implement this serialcall */ \
	inline TArray<func##FPtr> g_##func; \
	/* Get the function that calls each component's func serially. */ \
	inline func##FPtr Get##func##FPtr(ComponentTypeID id) \
	{ \
		if (g_##func.size() <= id) \
			return nullptr; \
		return g_##func[id]; \
	} \
	/* Called in ENTITY_COMPONENT_SERIALCALL_HELPER */ \
	inline int32 func##_AddComponentClass(func##FPtr fn, ComponentTypeID id) \
	{ \
		if (g_##func.size() <= id) \
			g_##func.resize(id + 1, nullptr); \
		g_##func[id] = fn; \
		return 0; \
	} \
}

/* Generates the function used to call every component's *func* function statically. */
#define DECLARE_COMPONENT_SERIALCALL_HELPER_EX(func, forEachBody, ...) \
namespace _EntityPrivate \
{ \
	using _##func##THashMap = TCompContainer<_ComponentClass_>; /* THashMap<GUID, _ComponentClass_> */ \
	static char _##func##TracerName[256] = "DECLARE_COMPONENT_SERIALCALL_HELPER_EX("#func") - "; \
	static errno_t _Dummy_##func##TracerName = strcat_s(_##func##TracerName, ComponentTypeDefaults<_ComponentClass_>::Name); \
	static auto _##func = [](void* containerPtr, __VA_ARGS__) \
	{ \
		TRACE_SCOPE(_##func##TracerName); \
		if constexpr (THas##func<_ComponentClass_>) \
		{ \
			_##func##THashMap& container = *(_##func##THashMap*)containerPtr; \
			/* Call the serialcall functions */ \
			for (auto& [k, comp] : container) \
			{ \
				forEachBody; \
			} \
		} \
	}; \
	static int32 _Dummy_##func = ComponentStaticCallbacks::func##_AddComponentClass(_##func, _ComponentClass_::TypeID); \
};

#define ENTITY_COMPONENT_SERIALCALL_HELPER(func) \
DECLARE_COMPONENT_SERIALCALL_HELPER_EX(func, comp.func())

// Put in the component's class in the .h file
#define ENTITY_COMPONENT_CLASS_BODY(displayName) \
public: \
friend class ComponentRegistry; \
static ComponentTypeID TypeID; \
static inline const String ClassDisplayName = displayName; \
virtual const String& GetClassDisplayName() const override { return ClassDisplayName; }

// Put in the component's .cpp file
#define DECLARE_ENTITY_COMPONENT_CLASS(component) \
ComponentTypeID component::TypeID = ComponentRegistry::RegisterComponentClass<component>(); \
namespace _EntityPrivate \
{ \
	/* Can't have function calls out in the open, unless they're assignments. */ \
	/*static ComponentTypeID _Dummy_##component##TypeID */ \
	using _ComponentClass_ = component; \
}

// Put in the component's .cpp file after the DECLARE_ENTITY_COMPONENT_CLASS, if the component implements Tick
#define DECLARE_COMPONENT_SERIALCALL_TICK() \
DECLARE_COMPONENT_SERIALCALL_HELPER_EX(Tick, \
if (comp.IsTickEnabled()) \
	comp.Tick(deltaTime); \
, float deltaTime)

// Put in the component's .cpp file after the DECLARE_ENTITY_COMPONENT_CLASS, if the component implements BuildRendererData
#define DECLARE_COMPONENT_SERIALCALL_BUILDRENDERERDATA() \
DECLARE_COMPONENT_SERIALCALL_HELPER_EX(BuildRendererData, \
if (comp.IsSceneComponent()) \
	comp.BuildRendererData(data); \
, RRendererData& data)

// For documentation purposes
#define SERIALCALL

namespace Ion
{
	template<typename CompT>
	struct ComponentTypeDefaults
	{
		static constexpr const char* Name = "Unknown Component";
	};

	using ComponentTypeID = uint16;

	DECLARE_COMPONENT_SERIALCALL(OnCreate)
	DECLARE_COMPONENT_SERIALCALL(OnDestroy)
	DECLARE_COMPONENT_SERIALCALL(Tick, float)
	DECLARE_COMPONENT_SERIALCALL(BuildRendererData, RRendererData&)

	/* Abstract class */
	class ION_API Component
	{
	public:
		virtual void OnCreate() { }
		virtual void OnDestroy() { }

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

		virtual const String& GetClassDisplayName() const = 0;

	protected:
		Component();

		void InitAsSceneComponent();

	public:
		template<typename CompT>
		struct Hasher
		{
			size_t operator()(const CompT& comp) const noexcept {

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

	/* Container for Components of type T
	   Used to call Component Callback functions */
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
		using ComponentContainerPtr = void*;

		ComponentRegistry(World* worldContext);
		~ComponentRegistry();

		template<typename CompT, typename... Args>
		CompT* CreateComponent(Args&&... args);

		template<typename CompT>
		void DestroyComponent(CompT* component);
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
		THashMap<ComponentTypeID, ComponentContainerPtr> m_ComponentArrays;
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
		const ComponentTypeID TypeID = CompT::TypeID;

		// Initialize the array if it's the first component of that type
		if (m_InitializedTypes.find(TypeID) == m_InitializedTypes.end())
		{
			InitializeComponentContainter<CompT>();
		}

		CompContainer& container = GetContainer<CompContainer>();

		CompT component(Forward<Args>(args)...);
		auto& [it, _] = container.emplace(component.GetGUID(), Move(component));

		CompT* componentPtr = &(*it).second;
		componentPtr->m_WorldContext = m_WorldContext;
		componentPtr->m_Name = ComponentTypeDefaults<CompT>::Name;

		if constexpr (THasOnCreate<CompT>)
		{
			componentPtr->OnCreate();
		}
		return componentPtr;
	}

	template<typename CompT>
	inline void ComponentRegistry::DestroyComponent(CompT* component)
	{
		static_assert(TIsBaseOfV<Component, CompT>);
		using CompContainer = TCompContainer<CompT>;
		const ComponentTypeID TypeID = CompT::TypeID;

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
			if constexpr (THasOnDestroy<CompT>)
			{
				component->OnDestroy();
			}
			container.erase(guid);
		}
	}

	template<typename CompT>
	inline ComponentTypeID ComponentRegistry::RegisterComponentClass()
	{
		static_assert(TIsBaseOfV<Component, CompT>);

		StaticComponentTypeData* componentTypeData = GetComponentTypeData();

		CompT::TypeID = componentTypeData->ComponentIDCounter;
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
		const ComponentTypeID TypeID = CompT::TypeID;

		m_InitializedTypes.insert(TypeID);
		CompContainer* container = new CompContainer;
		m_ComponentArrays.insert({ TypeID, container });
	}

	template<typename ContainerT>
	ContainerT& ComponentRegistry::GetContainer()
	{
		using CompT = ContainerT::value_type::second_type;
		static_assert(TIsBaseOfV<Component, CompT>);

		ComponentContainerPtr containerPtr = m_ComponentArrays.at(CompT::TypeID);
		return *(ContainerT*)containerPtr;
	}
}
