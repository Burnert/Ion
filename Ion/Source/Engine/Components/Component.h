#pragma once

#define DECLARE_COMPONENT_CALLBACK(func, ...) \
DECLARE_THASFUNCTION(func); \
namespace ComponentStaticCallbacks \
{ \
	using func##FPtr = void(*)(void*, __VA_ARGS__); \
	/* Components which implement this callback function */ \
	inline TArray<func##FPtr> g_##func; \
	inline func##FPtr Get##func##FPtr(ComponentTypeID id) \
	{ \
		if (g_##func.size() <= id) \
			return nullptr; \
		return g_##func[id]; \
	} \
	/* Called in ENTITY_COMPONENT_STATIC_CALLBACK_FUNC_HELPER */ \
	inline int32 func##_AddComponentClass(func##FPtr fn, ComponentTypeID id) \
	{ \
		if (g_##func.size() <= id) \
			g_##func.resize(id + 1, nullptr); \
		g_##func[id] = fn; \
		return 0; \
	} \
}

#define ENTITY_COMPONENT_STATIC_CALLBACK_FUNC_HELPER_EX(func, forEachBody, ...) \
namespace _EntityPrivate \
{ \
	using _##func##THashMap = THashMap<GUID, _ComponentClass_>; \
	static auto _##func = [](void* containerPtr, __VA_ARGS__) \
	{ \
		if constexpr (THas##func<_ComponentClass_>) \
		{ \
			_##func##THashMap& container = *(_##func##THashMap*)containerPtr; \
			for (auto& [k, comp] : container) \
			{ \
				forEachBody; \
			} \
		} \
	}; \
	static int32 _Dummy_##func = ComponentStaticCallbacks::func##_AddComponentClass(_##func, _ComponentClass_::TypeID); \
};

#define ENTITY_COMPONENT_STATIC_CALLBACK_FUNC_HELPER(func) \
ENTITY_COMPONENT_STATIC_CALLBACK_FUNC_HELPER_EX(func, comp.func())

// Put in the component's .cpp file after the DECLARE_ENTITY_COMPONENT_CLASS, if the component implements OnCreate
#define ENTITY_COMPONENT_STATIC_CALLBACK_ONCREATE_FUNC() \
ENTITY_COMPONENT_STATIC_CALLBACK_FUNC_HELPER(OnCreate)

// Put in the component's .cpp file after the DECLARE_ENTITY_COMPONENT_CLASS, if the component implements OnDestroy
#define ENTITY_COMPONENT_STATIC_CALLBACK_ONDESTROY_FUNC() \
ENTITY_COMPONENT_STATIC_CALLBACK_FUNC_HELPER(OnDestroy)

// Put in the component's .cpp file after the DECLARE_ENTITY_COMPONENT_CLASS, if the component implements Tick
#define ENTITY_COMPONENT_STATIC_CALLBACK_TICK_FUNC() \
ENTITY_COMPONENT_STATIC_CALLBACK_FUNC_HELPER_EX(Tick, \
if (comp.IsTickEnabled()) \
	comp.Tick(deltaTime); \
, float deltaTime)

// Put in the component's .cpp file after the DECLARE_ENTITY_COMPONENT_CLASS, if the component implements BuildRendererData
#define ENTITY_COMPONENT_STATIC_CALLBACK_BUILDRENDERERDATA_FUNC() \
ENTITY_COMPONENT_STATIC_CALLBACK_FUNC_HELPER_EX(BuildRendererData, \
if (comp.IsSceneComponent()) \
	comp.BuildRendererData(data); \
, RRendererData& data)

// Put in the component's class in the .h file
#define ENTITY_COMPONENT_CLASS_BODY() \
public: \
friend class ComponentRegistry; \
inline static ComponentTypeID TypeID;

// Put in the component's .cpp file
#define DECLARE_ENTITY_COMPONENT_CLASS(component) \
namespace _EntityPrivate \
{ \
	/* Can't have function calls out in the open, unless they're assignments. */ \
	static ComponentTypeID _Dummy_##component##TypeID = component::TypeID = ComponentRegistry::RegisterComponentClass<component>(); \
	using _ComponentClass_ = component; \
}

// For documentation purposes
#define COMPCALLBACKFUNC

namespace Ion
{
	using ComponentTypeID = uint16;

	DECLARE_COMPONENT_CALLBACK(OnCreate)
	DECLARE_COMPONENT_CALLBACK(OnDestroy)
	DECLARE_COMPONENT_CALLBACK(Tick, float)
	DECLARE_COMPONENT_CALLBACK(BuildRendererData, RRendererData&)

	struct SceneComponentData
	{
		Transform Transform;
		uint8 bVisible : 1;
		uint8 bVisibleInGame : 1;
	};

	class ION_API Component
	{
	public:
		// Component Callback methods

		void COMPCALLBACKFUNC OnCreate() { }
		void COMPCALLBACKFUNC OnDestroy() { }
		void COMPCALLBACKFUNC Tick(float deltaTime) { }

		// End of Component Callback methods

		void SetTickEnabled(bool bTick);
		bool IsTickEnabled() const;

		bool IsSceneComponent() const;

		void SetName(const String& name);
		const String& GetName() const;

		/* Returns the GUID of the Component.
		   A GUID is initiated at the creation of the Component. */
		const GUID& GetGUID() const;

		/* Returns a pointer to the World the Component is currently in. */
		World* GetWorldContext() const;

		bool operator==(const Component& other) const;
		bool operator!=(const Component& other) const;

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
	};

	template<typename T>
	using TCompContainer = THashMap<GUID, T>;

	class ION_API ComponentRegistry
	{
	public:
		using ComponentContainerPtr = void*;

		ComponentRegistry(World* worldContext);
		~ComponentRegistry();

		template<typename CompT, typename... Args>
		CompT* CreateComponent(Args&&... args);

		template<typename CompT>
		void DestroyComponent(CompT* component);

		template<typename CompT>
		void BindComponentToEntity(Entity* entity, CompT* component) {}

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

	private:
		THashMap<ComponentTypeID, ComponentContainerPtr> m_ComponentArrays;
		THashSet<ComponentTypeID> m_InitializedTypes;

		World* m_WorldContext;

		static THashSet<ComponentTypeID> s_RegisteredTypes;
		static ComponentTypeID s_ComponentIDCounter;
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

		CompT::TypeID = s_ComponentIDCounter;
		s_RegisteredTypes.insert(s_ComponentIDCounter);

		return s_ComponentIDCounter++;
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
