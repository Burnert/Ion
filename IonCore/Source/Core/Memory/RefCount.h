#pragma once

#include "MemoryCore.h"
#include "Core/Error/Error.h"

namespace Ion
{
	REGISTER_LOGGER(RefCountLogger, "Core::Memory::RefCount", ELoggerFlags::DisabledByDefault);

#pragma region Intrusive

#pragma region Templates (type traits)

	template<typename TFrom, typename TTo>
	struct TIsRefCompatible : TIsConvertible<TFrom*, TTo*> { };

	template<typename TFrom, typename TTo>
	static constexpr bool TIsRefCompatibleV = TIsRefCompatible<TFrom, TTo>::value;

#pragma endregion

#pragma region RefCountable

	class NOVTABLE RefCountable
	{
	protected:
		RefCountable();

	public:
		virtual ~RefCountable();

		size_t RefCount() const;

		RefCountable(const RefCountable&) = delete;
		RefCountable(RefCountable&&) = delete;
		RefCountable& operator=(const RefCountable&) = delete;
		RefCountable& operator=(RefCountable&&) = delete;

	private:
		size_t m_Count;

		size_t IncRef() noexcept;
		size_t DecRef() noexcept;

		void DeleteThis() noexcept;

		template<typename T>
		friend class TRef;
	};

	#pragma region RefCountable Implementation

	inline RefCountable::RefCountable() :
		m_Count(0)
	{
	}

	inline RefCountable::~RefCountable()
	{
	}

	FORCEINLINE size_t RefCountable::RefCount() const
	{
		return m_Count;
	}

	inline size_t RefCountable::IncRef() noexcept
	{
		++m_Count;
		RefCountLogger.Debug("RefCountable {{{}}} incremented the ref count by 1. Current ref count: {}", (void*)this, m_Count);
		return m_Count;
	}

	inline size_t RefCountable::DecRef() noexcept
	{
		ionassert(m_Count > 0);

		--m_Count;
		RefCountLogger.Debug("RefCountable {{{}}} decremented the ref count by 1. Current ref count: {}", (void*)this, m_Count);
		if (m_Count == 0)
		{
			RefCountLogger.Trace("RefCountable {{{}}} ref count reached 0.", (void*)this);

			RefCountLogger.Trace("RefCountable {{{}}} deleting ref countable object...", (void*)this);
			DeleteThis();

			return 0;
		}
		return m_Count;
	}

	inline void RefCountable::DeleteThis() noexcept
	{
		void* _this = this;
		delete this;
		RefCountLogger.Debug("RefCountable {{{}}} deleted ref countable object.", (void*)_this);
	}

	#pragma endregion

#pragma endregion

#pragma region TRef

	template<typename T>
	class TRef
	{
	public:
		using TElement = T;

		/**
		 * @brief Construct a null ref
		 */
		TRef();

		/**
		 * @brief Construct a null ref
		 */
		TRef(nullptr_t);

		/**
		 * @brief Construct a ref that owns ptr
		 * 
		 * @param ptr Pointer to take the ownership of (must be derived from RefCountable)
		 */
		TRef(T* ptr);

		/**
		 * @brief Copy another ref
		 * 
		 * @param other Ref to make a copy of
		 */
		TRef(const TRef& other);

		/**
		 * @brief Copy a ref of another type
		 * 
		 * @param other Ref to make a copy of
		 */
		template<typename T0, TEnableIfT<TIsRefCompatibleV<T0, T>>* = 0>
		TRef(const TRef<T0>& other);

		/**
		 * @brief Move another ref
		 * 
		 * @param other Ref to move
		 */
		TRef(TRef&& other) noexcept;

		/**
		 * @brief Move a ref of another type
		 * 
		 * @param other Ref to move
		 */
		template<typename T0, TEnableIfT<TIsRefCompatibleV<T0, T>>* = 0>
		TRef(TRef<T0>&& other) noexcept;

		~TRef();

		/**
		 * @brief Copy another ref
		 * 
		 * @param other Ref to make a copy of
		 */
		TRef& operator=(const TRef& other);

		/**
		 * @brief Copy a ref of another type
		 * 
		 * @param other Ref to make a copy of
		 */
		template<typename T0, TEnableIfT<TIsRefCompatibleV<T0, T>>* = 0>
		TRef& operator=(const TRef<T0>& other);

		/**
		 * @brief Move another ref
		 * 
		 * @param other Ref to move
		 */
		TRef& operator=(TRef&& other) noexcept;

		/**
		 * @brief Move a ref of another type
		 * 
		 * @param other Ref to move
		 */
		template<typename T0, TEnableIfT<TIsRefCompatibleV<T0, T>>* = 0>
		TRef& operator=(TRef<T0>&& other) noexcept;

		/**
		 * @brief Assign a null value
		 */
		TRef& operator=(nullptr_t);

		/**
		 * @brief Check if two refs point to the same object
		 * 
		 * @param other Other ref
		 */
		template<typename T0>
		bool operator==(const TRef<T0>& other);

		/**
		 * @brief Check if two refs don't point to the same object
		 * 
		 * @param other Other ref
		 */
		template<typename T0>
		bool operator!=(const TRef<T0>& other);

		void Swap(TRef& other);

		size_t RefCount() const;

		T* Raw() const;
		T& operator*() const;
		T* operator->() const;

		/**
		 * @return true if the ref is not null
		 */
		operator bool() const;

	private:
		template<typename T0>
		void CopyConstruct(const TRef<T0>& other);

		template<typename T0>
		void MoveConstruct(TRef<T0>&& other) noexcept;

		template<typename T0>
		void CastConstruct(const TRef<T0>& other);

		template<typename T0>
		void CastConstruct(TRef<T0>&& other) noexcept;

		void Delete();

		void IncRef() noexcept;
		void DecRef() noexcept;

	private:
		T* m_Object;

		template<typename T0>
		friend class TRef;

		template<typename T0, typename... Args>
		friend TRef<T0> MakeRef(Args&&... args);

		template<typename T0, typename T1>
		friend TRef<T0> RefCast(const TRef<T1>& other);

		template<typename T0, typename T1>
		friend TRef<T0> RefCast(TRef<T1>&& other);
	};

	#pragma region TRef Implementation

	template<typename T>
	inline TRef<T>::TRef() :
		TRef(nullptr)
	{
	}

	template<typename T>
	inline TRef<T>::TRef(nullptr_t) :
		m_Object(nullptr)
	{
		RefCountLogger.Debug("TRef {{{}}} has been null constructed.", (void*)m_Object);
	}

	template<typename T>
	inline TRef<T>::TRef(T* ptr) :
		m_Object(ptr)
	{
		static_assert(TIsBaseOfV<RefCountable, T>, "Object cannot be used with TRef if it's not derived from RefCountable.");

		RefCountLogger.Debug("TRef {{{}}} has been constructed.", (void*)m_Object);
		IncRef();
	}

	template<typename T>
	inline TRef<T>::TRef(const TRef& other)
	{
		CopyConstruct(other);
	}

	template<typename T>
	template<typename T0, TEnableIfT<TIsRefCompatibleV<T0, T>>*>
	inline TRef<T>::TRef(const TRef<T0>& other)
	{
		CopyConstruct(other);
	}

	template<typename T>
	inline TRef<T>::TRef(TRef&& other) noexcept
	{
		MoveConstruct(Move(other));
	}

	template<typename T>
	template<typename T0, TEnableIfT<TIsRefCompatibleV<T0, T>>*>
	inline TRef<T>::TRef(TRef<T0>&& other) noexcept
	{
		MoveConstruct(Move(other));
	}

	template<typename T>
	inline TRef<T>::~TRef()
	{
		RefCountLogger.Debug("TRef {{{}}} has been destroyed.", (void*)m_Object);
		Delete();
	}

	template<typename T>
	inline TRef<T>& TRef<T>::operator=(const TRef& other)
	{
		RefCountLogger.Debug("TRef {{{}}} has been copy assigned to TRef {{{}}}.", (void*)other.m_Object, (void*)m_Object);
		TRef<T>(other).Swap(*this);
		return *this;
	}

	template<typename T>
	template<typename T0, TEnableIfT<TIsRefCompatibleV<T0, T>>*>
	inline TRef<T>& TRef<T>::operator=(const TRef<T0>& other)
	{
		RefCountLogger.Debug("TRef {{{}}} has been copy assigned to TRef {{{}}}.", (void*)other.m_Object, (void*)m_Object);
		TRef<T>(other).Swap(*this);
		return *this;
	}

	template<typename T>
	inline TRef<T>& TRef<T>::operator=(TRef&& other) noexcept
	{
		RefCountLogger.Debug("TRef {{{}}} has been move assigned to TRef {{{}}}.", (void*)other.m_Object, (void*)m_Object);
		TRef<T>(Move(other)).Swap(*this);
		return *this;
	}

	template<typename T>
	template<typename T0, TEnableIfT<TIsRefCompatibleV<T0, T>>*>
	inline TRef<T>& TRef<T>::operator=(TRef<T0>&& other) noexcept
	{
		RefCountLogger.Debug("TRef {{{}}} has been move assigned to TRef {{{}}}.", (void*)other.m_Object, (void*)m_Object);
		TRef<T>(Move(other)).Swap(*this);
		return *this;
	}

	template<typename T>
	inline TRef<T>& TRef<T>::operator=(nullptr_t)
	{
		RefCountLogger.Debug("Null has been assigned to TRef {{{}}}.", (void*)m_Object);
		Delete();
		return *this;
	}

	template<typename T>
	template<typename T0>
	inline bool TRef<T>::operator==(const TRef<T0>& other)
	{
		return m_Object == other.m_Object;
	}

	template<typename T>
	template<typename T0>
	inline bool TRef<T>::operator!=(const TRef<T0>& other)
	{
		return m_Object != other.m_Object;
	}

	template<typename T>
	inline void TRef<T>::Swap(TRef& other)
	{
		std::swap(m_Object, other.m_Object);
	}

	template<typename T>
	inline size_t TRef<T>::RefCount() const
	{
		if (!m_Object)
			return 0;

		return m_Object->RefCount();
	}

	template<typename T>
	inline typename T* TRef<T>::Raw() const
	{
		return m_Object;
	}

	template<typename T>
	inline typename T& TRef<T>::operator*() const
	{
		ionassert(m_Object);
		return *m_Object;
	}

	template<typename T>
	inline typename T* TRef<T>::operator->() const
	{
		ionassert(m_Object);
		return m_Object;
	}

	template<typename T>
	inline TRef<T>::operator bool() const
	{
		return (bool)m_Object;
	}

	template<typename T>
	inline void TRef<T>::Delete()
	{
		DecRef();
		m_Object = nullptr;
	}

	template<typename T>
	template<typename T0>
	inline void TRef<T>::CopyConstruct(const TRef<T0>& other)
	{
		m_Object = other.m_Object;
		RefCountLogger.Debug("TRef {{{}}} has been copy constructed.", (void*)m_Object);
		IncRef();
	}

	template<typename T>
	template<typename T0>
	inline void TRef<T>::MoveConstruct(TRef<T0>&& other) noexcept
	{
		m_Object = other.m_Object;
		RefCountLogger.Debug("TRef {{{}}} has been move constructed.", (void*)m_Object);

		other.m_Object = nullptr;
	}

	template<typename T>
	template<typename T0>
	inline void TRef<T>::CastConstruct(const TRef<T0>& other)
	{
		m_Object = static_cast<T*>(other.m_Object);
		RefCountLogger.Debug("TRef {{{}}} has been copy cast constructed.", (void*)m_Object);
		IncRef();
	}

	template<typename T>
	template<typename T0>
	inline void TRef<T>::CastConstruct(TRef<T0>&& other) noexcept
	{
		m_Object = static_cast<T*>(other.m_Object);
		RefCountLogger.Debug("TRef {{{}}} has been move cast constructed.", (void*)m_Object);

		other.m_Object = nullptr;
	}

	template<typename T>
	inline void TRef<T>::IncRef() noexcept
	{
		if (m_Object)
		{
			((RefCountable*)m_Object)->IncRef();
		}
	}

	template<typename T>
	inline void TRef<T>::DecRef() noexcept
	{
		if (m_Object)
		{
			((RefCountable*)m_Object)->DecRef();
		}
	}

	#pragma endregion

#pragma endregion

#pragma region MakeRef / RefCast / DynamicRefCast

#define FRIEND_MAKE_REF \
	template<typename T, typename... Args> \
	friend Ion::TRef<T> Ion::MakeRef(Args&&... args)

	template<typename T, typename... Args>
	inline TRef<T> MakeRef(Args&&... args)
	{
		static_assert(TIsBaseOfV<RefCountable, T>, "Object cannot be used with TRef if it's not derived from RefCountable.");

		return TRef<T>(new T(Forward<Args>(args)...));
	}

	template<typename T0, typename T1>
	inline TRef<T0> RefCast(const TRef<T1>& other)
	{
		TRef<T0> ref;
		ref.CastConstruct(other);
		return ref;
	}

	template<typename T0, typename T1>
	inline TRef<T0> RefCast(TRef<T1>&& other)
	{
		TRef<T0> ref;
		ref.CastConstruct(Move(other));
		return ref;
	}

	template<typename T0, typename T1>
	inline TRef<T0> DynamicRefCast(const TRef<T1>& other)
	{
		if (!dynamic_cast<T0*>(other.Raw()))
			return TRef<T0>();

		return RefCast<T0>(other);
	}

	template<typename T0, typename T1>
	inline TRef<T0> DynamicRefCast(TRef<T1>&& other)
	{
		if (!dynamic_cast<T0*>(other.Raw()))
			return TRef<T0>();

		return RefCast<T0>(Move(other));
	}

#pragma endregion

	int RefCountTest();

#pragma endregion

#pragma region Non-Intrusive

#pragma region Templates (type traits)

	template<typename T>
	class TPtrBase;

	template<typename T>
	class TSharedPtr;

	template<typename T>
	class TWeakPtr;

	template<typename T>
	class TEnableSFT;

	/**
	 * @brief Checks if T is a ref-count pointer type
	 * 
	 * @tparam T Type to check
	 */
	template<typename T>
	struct TIsRefCountPtr
	{
		static constexpr inline bool Value = false;
		static constexpr inline bool value = false;
	};

	template<typename T>
	struct TIsRefCountPtr<TPtrBase<T>>
	{
		static constexpr inline bool Value = true;
		static constexpr inline bool value = true;
	};

	template<typename T>
	struct TIsRefCountPtr<TSharedPtr<T>>
	{
		static constexpr inline bool Value = true;
		static constexpr inline bool value = true;
	};

	template<typename T>
	struct TIsRefCountPtr<TWeakPtr<T>>
	{
		static constexpr inline bool Value = true;
		static constexpr inline bool value = true;
	};

	/**
	 * @brief Checks if T is a TSharedPtr or TWeakPtr
	 * 
	 * @tparam T Type to check
	 */
	template<typename T>
	static constexpr inline bool TIsRefCountPtrV = TIsRefCountPtr<T>::Value;

	template<typename TFrom, typename TTo>
	struct TIsPtrCompatible : TIsConvertible<TFrom*, TTo*> { };

	template<typename TFrom, typename TTo>
	static constexpr bool TIsPtrCompatibleV = TIsPtrCompatible<TFrom, TTo>::value;

	template<typename T, typename = void>
	struct TCanEnableSFT : TBool<false> { };
	template<typename T>
	struct TCanEnableSFT<T, std::void_t<typename T::TEnableSFTType>> : TIsConvertible<std::remove_cv_t<T>*, typename T::TEnableSFTType*> { };

	template<typename T>
	static constexpr bool TCanEnableSFTV = TCanEnableSFT<T>::value;

#pragma endregion

#pragma region RefCountBase

	/**
	 * @brief Ref counting block base for shared pointers
	 */
	class RefCountBase
	{
	public:
		RefCountBase();

		/**
		 * @brief Increments the strong reference count
		 *
		 * @return uint32 Reference count after incrementing
		 */
		uint32 IncRef();

		/**
		 * @brief Decrements the strong reference count
		 *
		 * @details Deletes the owned pointer if the reference count is 0
		 * and decrements the weak reference count.
		 *
		 * @return uint32 Reference count after decrementing
		 */
		uint32 DecRef();

		/**
		 * @brief Increments the weak reference count
		 *
		 * @return uint32 Weak reference count after incrementing
		 */
		uint32 IncWeak();

		/**
		 * @brief Decrements the weak reference count
		 *
		 * @details Destroys this control block if the reference count is 0.
		 *
		 * @return uint32 Weak reference count after decrementing
		 */
		uint32 DecWeak();

		uint32 RefCount() const;
		uint32 WeakRefCount() const;

	protected:
		/**
		 * @brief Deletes the owned object.
		 */
		virtual void Destroy() noexcept = 0;
		/**
		 * @brief Destroys this control block.
		 */
		virtual void DestroySelf() noexcept = 0;

	private:
		/**
		 * @brief Current reference count
		 */
		uint32 m_Count;
		/**
		 * @brief Current weak reference count
		 *
		 * @details It is 1 if there are no weak pointers.
		 * The ref counting block kind of is a weak reference.
		 */
		uint32 m_WeakCount;

		template<typename T0>
		friend class TPtrBase;
	};

	#pragma region RefCountBase Implementation

	inline RefCountBase::RefCountBase() :
		m_Count(1),
		m_WeakCount(1)
	{
	}

	inline uint32 RefCountBase::IncRef()
	{
		++m_Count;
		RefCountLogger.Debug("RefCountBase {{{}}} Incremented the ref count by 1. Current ref count: {}", (void*)this, m_Count);
		return m_Count;
	}

	inline uint32 RefCountBase::DecRef()
	{
		--m_Count;
		RefCountLogger.Debug("RefCountBase {{{}}} Decremented the ref count by 1. Current ref count: {}", (void*)this, m_Count);
		if (m_Count == 0)
		{
			RefCountLogger.Trace("RefCountBase {{{}}} Strong ref count reached 0.", (void*)this);
			Destroy();
			DecWeak();
		}
		return m_Count;
	}

	inline uint32 RefCountBase::IncWeak()
	{
		++m_WeakCount;
		RefCountLogger.Debug("RefCountBase {{{}}} Incremented the weak ref count by 1. Current weak ref count: {}", (void*)this, m_WeakCount);
		return m_WeakCount;
	}

	inline uint32 RefCountBase::DecWeak()
	{
		uint32 count = --m_WeakCount;
		RefCountLogger.Debug("RefCountBase {{{}}} Decremented the weak ref count by 1. Current weak ref count: {}", (void*)this, m_WeakCount);
		if (m_WeakCount == 0)
		{
			RefCountLogger.Trace("RefCountBase {{{}}} Weak ref count reached 0.", (void*)this);
			DestroySelf();
		}
		return count;
	}

	inline uint32 RefCountBase::RefCount() const
	{
		return m_Count;
	}

	inline uint32 RefCountBase::WeakRefCount() const
	{
		return m_WeakCount;
	}

	#pragma endregion

#pragma endregion

#pragma region TPtrRefCountBlock

	template<typename T>
	class TPtrRefCountBlock : public RefCountBase
	{
	public:
		static_assert(!TIsReferenceV<T>);

		/**
		 * @brief Construct a new RefCount object that owns the pointer
		 * 
		 * @param ptr Pointer to take the ownership of.
		 */
		TPtrRefCountBlock(T* ptr);

	private:
		virtual void Destroy() noexcept override;
		virtual void DestroySelf() noexcept override;

	private:
		/**
		 * @brief Owned pointer
		 */
		T* m_Ptr;
	};

	#pragma region TPtrRefCountBlock Implementation

	template<typename T>
	inline TPtrRefCountBlock<T>::TPtrRefCountBlock(T* ptr) :
		m_Ptr(ptr)
	{
		RefCountLogger.Trace("TPtrRefCountBlock {{{}}} has been constructed with a pointer to {} {{{}}}.", (void*)this, typeid(T).name(), (void*)ptr);
	}

	template<typename T>
	inline void TPtrRefCountBlock<T>::Destroy() noexcept
	{
		if (m_Ptr)
		{
			RefCountLogger.Trace("TPtrRefCountBlock {{{}}} is deleting the owned object {} {{{}}}...", (void*)this, typeid(T).name(), (void*)m_Ptr);
			delete m_Ptr;
			RefCountLogger.Debug("TPtrRefCountBlock {{{}}} has deleted the owned object {} {{{}}}.", (void*)this, typeid(T).name(), (void*)m_Ptr);
		}
	}

	template<typename T>
	inline void TPtrRefCountBlock<T>::DestroySelf() noexcept
	{
		void* ptr = this;
		RefCountLogger.Trace("TPtrRefCountBlock {{{}}} is deleting itself...", ptr);
		delete this;
		RefCountLogger.Debug("TPtrRefCountBlock {{{}}} has deleted itself.", ptr);
	}

	#pragma endregion

#pragma endregion

#pragma region TPtrDeleterRefCountBlock

	template<typename T, typename FDeleter>
	class TPtrDeleterRefCountBlock : public RefCountBase
	{
	public:
		static_assert(!TIsReferenceV<T>);

		/**
		 * @brief Construct a ref count block with a custom deleter.
		 * 
		 * @param ptr Pointer to take the ownership of.
		 */
		TPtrDeleterRefCountBlock(T* ptr, FDeleter deleter) noexcept;

	private:
		virtual void Destroy() noexcept override;
		virtual void DestroySelf() noexcept override;

	private:
		/**
		 * @brief Owned pointer
		 */
		T* m_Ptr;
		FDeleter m_Deleter;
	};

	#pragma region TPtrDeleterRefCountBlock Implementation

	template<typename T, typename FDeleter>
	inline TPtrDeleterRefCountBlock<T, FDeleter>::TPtrDeleterRefCountBlock(T* ptr, FDeleter deleter) noexcept :
		m_Ptr(ptr),
		m_Deleter(deleter)
	{
		RefCountLogger.Trace("TPtrDeleterRefCountBlock {{{}}} has been constructed with a pointer to {} {{{}}} and a custom deleter.", (void*)this, typeid(T).name(), (void*)ptr);
	}

	template<typename T, typename FDeleter>
	inline void TPtrDeleterRefCountBlock<T, FDeleter>::Destroy() noexcept
	{
		if (m_Ptr)
		{
			RefCountLogger.Trace("TPtrDeleterRefCountBlock {{{}}} is deleting the owned object {} {{{}}} using a custom deleter...", (void*)this, typeid(T).name(), (void*)m_Ptr);
			m_Deleter(m_Ptr);
			RefCountLogger.Debug("TPtrDeleterRefCountBlock {{{}}} has deleted the owned object {} {{{}}} using a custom deleter.", (void*)this, typeid(T).name(), (void*)m_Ptr);
		}
	}

	template<typename T, typename FDeleter>
	inline void TPtrDeleterRefCountBlock<T, FDeleter>::DestroySelf() noexcept
	{
		void* ptr = this;
		RefCountLogger.Trace("TPtrDeleterRefCountBlock {{{}}} is deleting itself...", ptr);
		delete this;
		RefCountLogger.Debug("TPtrDeleterRefCountBlock {{{}}} has deleted itself.", ptr);
	}

	#pragma endregion

#pragma endregion

#pragma region TObjectRefCountBlock

	namespace _Memory_Detail
	{
		template<typename T>
		struct TWrapper
		{
			T Object;
		};
	}

	template<typename T>
	class TObjectRefCountBlock : public RefCountBase
	{
	public:
		static_assert(!TIsReferenceV<T>);

		/**
		 * @brief Create a ref counter that has the element constructed in place.
		 */
		template<typename... Args>
		TObjectRefCountBlock(Args&&... args);

		~TObjectRefCountBlock() { }

	private:
		virtual void Destroy() noexcept override;
		virtual void DestroySelf() noexcept override;

	private:
		union
		{
			/** Owned object */
			_Memory_Detail::TWrapper<T> m_Object;
		};

		template<typename T0>
		friend class TPtrBase;
	};

	#pragma region TObjectRefCountBlock Implementation

	template<typename T>
	template<typename... Args>
	inline TObjectRefCountBlock<T>::TObjectRefCountBlock(Args&&... args)
	{
		new((void*)&m_Object.Object) T(Forward<Args>(args)...);
		RefCountLogger.Trace("TObjectRefCountBlock {{{}}} has been constructed in place with an object of type {}.", (void*)this, typeid(T).name());
	}

	template<typename T>
	inline void TObjectRefCountBlock<T>::Destroy() noexcept
	{
		RefCountLogger.Trace("TObjectRefCountBlock {{{}}} is calling the object destructor...", (void*)this);
		m_Object.Object.~T();
		RefCountLogger.Debug("TObjectRefCountBlock {{{}}} has destroyed the object.", (void*)this);
	}

	template<typename T>
	inline void TObjectRefCountBlock<T>::DestroySelf() noexcept
	{
		void* ptr = this;
		RefCountLogger.Trace("TObjectRefCountBlock {{{}}} is deleting itself...", ptr);
		delete this;
		RefCountLogger.Debug("TObjectRefCountBlock {{{}}} has deleted itself.", ptr);
	}

	#pragma endregion

#pragma endregion

#pragma region TObjectDestroyRefCountBlock

	template<typename T, typename FOnDestroy>
	class TObjectDestroyRefCountBlock : public RefCountBase
	{
	public:
		static_assert(!TIsReferenceV<T>);

		/**
		 * @brief Create a ref counter that has the element constructed in place
		 * and has a destroy callback function.
		 */
		template<typename... Args>
		TObjectDestroyRefCountBlock(FOnDestroy onDestroy, Args&&... args);

		~TObjectDestroyRefCountBlock() { }

	private:
		virtual void Destroy() noexcept override;
		virtual void DestroySelf() noexcept override;

	private:
		union
		{
			/** Owned object */
			_Memory_Detail::TWrapper<T> m_Object;
		};
		FOnDestroy m_OnDestroy;

		template<typename T0>
		friend class TPtrBase;
	};

	#pragma region TObjectDestroyRefCountBlock Implementation

	template<typename T, typename FOnDestroy>
	template<typename... Args>
	inline TObjectDestroyRefCountBlock<T, FOnDestroy>::TObjectDestroyRefCountBlock(FOnDestroy onDestroy, Args&&... args) :
		m_OnDestroy(onDestroy)
	{
		new((void*)&m_Object.Object) T(Forward<Args>(args)...);
		RefCountLogger.Trace("TObjectDestroyRefCountBlock {{{}}} has been constructed in place with an object of type {} and a destroy callback.", (void*)this, typeid(T).name());
	}

	template<typename T, typename FOnDestroy>
	inline void TObjectDestroyRefCountBlock<T, FOnDestroy>::Destroy() noexcept
	{
		static_assert(std::is_invocable_v<FOnDestroy, TRemoveRef<T>&>);

		RefCountLogger.Trace("TObjectDestroyRefCountBlock {{{}}} is calling the object destructor...", (void*)this);
		m_OnDestroy(m_Object.Object);
		m_Object.Object.~T();
		RefCountLogger.Debug("TObjectDestroyRefCountBlock {{{}}} has destroyed the object.", (void*)this);
	}

	template<typename T, typename FOnDestroy>
	inline void TObjectDestroyRefCountBlock<T, FOnDestroy>::DestroySelf() noexcept
	{
		void* ptr = this;
		RefCountLogger.Trace("TObjectDestroyRefCountBlock {{{}}} is deleting itself...", ptr);
		delete this;
		RefCountLogger.Debug("TObjectDestroyRefCountBlock {{{}}} has deleted itself.", ptr);
	}

	#pragma endregion

#pragma endregion

#pragma region TPtrBase

	template<typename T>
	class TPtrBase
	{
	public:
		using TElement  = T;
		using TThis     = TPtrBase<T>;

		uint32 RefCount() const;
		uint32 WeakRefCount() const;

	protected:
		/**
		 * @brief Construct a null pointer base
		 */
		TPtrBase();

		/**
		 * @brief Construct a null pointer base
		 */
		TPtrBase(nullptr_t);

		/**
		 * @brief Constructs a ref count control block that owns the pointer.
		 * 
		 * @tparam T0 Element type
		 * @param ptr Pointer to take the ownership of.
		 */
		template<typename T0>
		void ConstructShared(T0* ptr);

		/**
		 * @brief Constructs a ref count control block with an element of type T constructed in place.
		 */
		template<typename... Args>
		void ConstructSharedInPlace(Args&&... args);

		/**
		 * @brief Constructs a ref count control block with a custom deleter that owns the pointer.
		 *
		 * @tparam T0 Element type
		 * @tparam FDeleter Deleter function type - void(T0*)
		 * 
		 * @param ptr Pointer to take the ownership of.
		 * @param deleter Deleter function, called when the ref count reached zero.
		 */
		template<typename T0, typename FDeleter>
		void ConstructSharedWithDeleter(T0* ptr, FDeleter deleter);

		/**
		 * @brief Constructs a ref count control block with an element of type T constructed in place
		 * and a custom destroy callback, that will be called right before the object is destroyed.
		 * 
		 * @tparam FOnDestroy Destroy callback type - void(T&)
		 * 
		 * @param onDestroy Destroy callback function, called before the object is destroyed.
		 */
		template<typename FOnDestroy, typename... Args>
		void ConstructSharedInPlaceWithDestroyCallback(FOnDestroy onDestroy, Args&&... args);

		/**
		 * @brief Makes this pointer a shared pointer that points
		 * to the same object as the weak pointer.
		 * 
		 * @tparam T0 Element type
		 * @param ptr Weak pointer
		 */
		template<typename T0>
		void ConstructSharedFromWeak(const TWeakPtr<T0>& ptr);

		/**
		 * @brief Makes this pointer a weak pointer that points
		 * to the same object as the shared pointer.
		 * 
		 * @tparam T0 Element type
		 * @param ptr Shared pointer
		 */
		template<typename T0>
		void ConstructWeak(const TSharedPtr<T0>& ptr);

		/**
		 * @brief Copies the other shared pointer to this pointer
		 * 
		 * @tparam T0 Element type
		 * @param other Other shared pointer
		 */
		template<typename T0>
		void CopyConstructShared(const TSharedPtr<T0>& other);

		/**
		 * @brief Copies the other weak pointer to this pointer
		 * 
		 * @tparam T0 Element type
		 * @param other Other weak pointer
		 */
		template<typename T0>
		void CopyConstructWeak(const TWeakPtr<T0>& other);

		/**
		 * @brief Moves the other pointer to this pointer
		 * 
		 * @tparam TPtr Pointer type
		 * @param other Other pointer
		 */
		template<typename TPtr>
		void MoveConstruct(TPtr&& other);

		/**
		 * @brief Makes this pointer point to the specified raw pointer,
		 * but use the control block of the other pointer
		 * 
		 * @details This is mostly used for pointer type casting.
		 * 
		 * @tparam T0 Other pointer element type
		 * @param other Other pointer
		 * @param ptr Raw pointer to point to.
		 */
		template<typename T0>
		void AliasConstructShared(const TSharedPtr<T0>& other, TElement* ptr);

		/**
		 * @brief Makes this pointer point to the specified raw pointer,
		 * but use the control block of the other pointer
		 *
		 * @details This is mostly used for pointer type casting.
		 *
		 * @tparam T0 Other pointer element type
		 * @param other Other pointer
		 * @param ptr Raw pointer to point to.
		 */
		template<typename T0>
		void AliasConstructShared(TSharedPtr<T0>&& other, TElement* ptr);

		template<typename T0>
		void SetPtrRepEnableSFT(T0* ptr, RefCountBase* rep);

		void Swap(TPtrBase& other);

		/**
		 * @brief Delete the shared pointer
		 * 
		 * @details Decrements the strong reference count.
		 */
		void DeleteShared();

		/**
		 * @brief Delete the weak pointer
		 * 
		 * @details Decrements the weak reference count.
		 */
		void DeleteWeak();

	private:
		T* m_Ptr;
		RefCountBase* m_Rep;

		template<typename T0>
		friend class TSharedPtr;

		template<typename T0>
		friend class TWeakPtr;

		template<typename T0>
		friend class TPtrBase;

		template<typename T0, typename... Args>
		friend TSharedPtr<T0> MakeShared(Args&&... args);

		template<typename T, typename FOnDestroy, typename... Args>
		friend TEnableIfT<std::is_invocable_v<FOnDestroy, TRemoveRef<T>&>, TSharedPtr<T>> MakeSharedDC(FOnDestroy onDestroy, Args&&... args);
	};

	#pragma region TPtrBase Implementation

	template<typename T>
	inline uint32 TPtrBase<T>::RefCount() const
	{
		if (!m_Rep)
			return 0;

		return m_Rep->RefCount();
	}

	template<typename T>
	inline uint32 TPtrBase<T>::WeakRefCount() const
	{
		if (!m_Rep)
			return 0;

		return m_Rep->WeakRefCount();
	}

	template<typename T>
	inline TPtrBase<T>::TPtrBase() :
		TPtrBase(nullptr)
	{
	}

	template<typename T>
	inline TPtrBase<T>::TPtrBase(nullptr_t) :
		m_Ptr(nullptr),
		m_Rep(nullptr)
	{
	}

	template<typename T>
	template<typename T0>
	inline void TPtrBase<T>::ConstructShared(T0* ptr)
	{
		ionassert(ptr);

		SetPtrRepEnableSFT(ptr, new TPtrRefCountBlock(ptr));
	}

	template<typename T>
	template<typename... Args>
	void TPtrBase<T>::ConstructSharedInPlace(Args&&... args)
	{
		TObjectRefCountBlock<T>* block = new TObjectRefCountBlock<T>(Forward<Args>(args)...);

		SetPtrRepEnableSFT(&block->m_Object.Object, block);
	}

	template<typename T>
	template<typename T0, typename FDeleter>
	inline void TPtrBase<T>::ConstructSharedWithDeleter(T0* ptr, FDeleter deleter)
	{
		static_assert(std::is_nothrow_invocable_v<FDeleter, T0*>);
		ionassert(ptr);

		SetPtrRepEnableSFT(ptr, new TPtrDeleterRefCountBlock(ptr, deleter));
	}

	template<typename T>
	template<typename FOnDestroy, typename... Args>
	inline void TPtrBase<T>::ConstructSharedInPlaceWithDestroyCallback(FOnDestroy onDestroy, Args&&... args)
	{
		auto block = new TObjectDestroyRefCountBlock<T, FOnDestroy>(onDestroy, Forward<Args>(args)...);

		SetPtrRepEnableSFT(&block->m_Object.Object, block);
	}

	template<typename T>
	template<typename T0>
	inline void TPtrBase<T>::ConstructSharedFromWeak(const TWeakPtr<T0>& ptr)
	{
		static_assert(TIsBaseOfV<TElement, T0>);

		if (ptr.IsValid())
		{
			m_Ptr = ptr.m_Ptr;
			m_Rep = ptr.m_Rep;

			m_Rep->IncRef();
		}
	}

	template<typename T>
	template<typename T0>
	inline void TPtrBase<T>::ConstructWeak(const TSharedPtr<T0>& ptr)
	{
		static_assert(TIsBaseOfV<TElement, T0>);

		if (ptr.m_Rep)
		{
			ionassert(ptr.m_Ptr);

			m_Ptr = ptr.m_Ptr;
			m_Rep = ptr.m_Rep;

			m_Rep->IncWeak();
		}
	}

	template<typename T>
	template<typename T0>
	inline void TPtrBase<T>::CopyConstructShared(const TSharedPtr<T0>& other)
	{
		static_assert(TIsBaseOfV<TElement, T0>);

		m_Ptr = other.m_Ptr;
		m_Rep = other.m_Rep;

		if (m_Rep)
		{
			ionassert(m_Ptr);

			m_Rep->IncRef();
		}
	}

	template<typename T>
	template<typename T0>
	inline void TPtrBase<T>::CopyConstructWeak(const TWeakPtr<T0>& other)
	{
		static_assert(TIsBaseOfV<TElement, T0>);

		m_Ptr = other.m_Ptr;
		m_Rep = other.m_Rep;

		if (m_Rep)
		{
			ionassert(m_Ptr);

			m_Rep->IncWeak();
		}
	}

	template<typename T>
	template<typename TPtr>
	inline void TPtrBase<T>::MoveConstruct(TPtr&& other)
	{
		static_assert(TIsRefCountPtrV<TPtr>);
		static_assert(TIsBaseOfV<TElement, typename TPtr::TElement>);

		m_Ptr = other.m_Ptr;
		m_Rep = other.m_Rep;

		other.m_Ptr = nullptr;
		other.m_Rep = nullptr;
	}

	template<typename T>
	template<typename T0>
	inline void TPtrBase<T>::AliasConstructShared(const TSharedPtr<T0>& other, TElement* ptr)
	{
		m_Ptr = ptr;
		m_Rep = other.m_Rep;

		if (m_Rep)
		{
			ionassert(m_Ptr);

			m_Rep->IncRef();
		}
	}

	template<typename T>
	template<typename T0>
	inline void TPtrBase<T>::AliasConstructShared(TSharedPtr<T0>&& other, TElement* ptr)
	{
		m_Ptr = ptr;
		m_Rep = other.m_Rep;

		other.m_Ptr = nullptr;
		other.m_Rep = nullptr;
	}

	template<typename T>
	template<typename T0>
	inline void TPtrBase<T>::SetPtrRepEnableSFT(T0* ptr, RefCountBase* rep)
	{
		m_Ptr = ptr;
		m_Rep = rep;
		if constexpr (TCanEnableSFTV<T0>)
		{
			m_Ptr->m_WeakSFT = TSharedPtr<T0>(*static_cast<TSharedPtr<T>*>(this), ptr);
		}
	}

	template<typename T>
	inline void TPtrBase<T>::Swap(TPtrBase<T>& other)
	{
		std::swap(m_Ptr, other.m_Ptr);
		std::swap(m_Rep, other.m_Rep);
	}

	template<typename T>
	inline void TPtrBase<T>::DeleteShared()
	{
		if (m_Rep)
		{
			m_Rep->DecRef();
		}

		m_Ptr = nullptr;
		m_Rep = nullptr;
	}

	template<typename T>
	inline void TPtrBase<T>::DeleteWeak()
	{
		if (m_Rep)
		{
			m_Rep->DecWeak();
		}

		m_Ptr = nullptr;
		m_Rep = nullptr;
	}

	#pragma endregion

#pragma endregion

#pragma region TSharedPtr

	template<typename T>
	class TSharedPtr : public TPtrBase<T>
	{
	public:
		using TBase = TPtrBase<T>;

		/**
		 * @brief Construct a null shared pointer
		 */
		TSharedPtr();

		/**
		 * @brief Construct a null shared pointer
		 */
		TSharedPtr(nullptr_t);

		/**
		 * @brief Construct a shared pointer that owns the ptr
		 * 
		 * @param ptr Pointer to take the ownership of.
		 */
		template<typename T0, TEnableIfT<TIsPtrCompatibleV<T0, T>>* = 0>
		explicit TSharedPtr(T0* ptr);

		/**
		 * @brief Construct a shared pointer with a custom deleter that owns the pointer.
		 *
		 * @tparam T0 Element type
		 * @tparam FDeleter Deleter function type - void(T0*)
		 *
		 * @param ptr Pointer to take the ownership of.
		 * @param deleter Deleter function, called when the ref count reached zero.
		 */
		template<typename T0, typename FDeleter,
			TEnableIfT<TIsPtrCompatibleV<T0, T>>* = 0,
			TEnableIfT<std::is_nothrow_invocable_v<FDeleter, T*>>* = 0>
		TSharedPtr(T0* ptr, FDeleter deleter);

		/**
		 * @brief Make a copy of another shared pointer.
		 * 
		 * @param other Other shared pointer
		 */
		TSharedPtr(const TSharedPtr& other);

		/**
		 * @brief Make a copy of another shared pointer.
		 * 
		 * @tparam T0 Other pointer element type
		 * @param other Other shared pointer
		 */
		template<typename T0, TEnableIfT<TIsPtrCompatibleV<T0, T>>* = 0>
		TSharedPtr(const TSharedPtr<T0>& other);

		/**
		 * @brief Move another shared pointer.
		 * 
		 * @param other Other shared pointer
		 */
		TSharedPtr(TSharedPtr&& other) noexcept;

		/**
		 * @brief Move another shared pointer.
		 * 
		 * @tparam T0 Other pointer element type
		 * @param other Other shared pointer
		 */
		template<typename T0, TEnableIfT<TIsPtrCompatibleV<T0, T>>* = 0>
		TSharedPtr(TSharedPtr<T0>&& other) noexcept;

		/**
		 * @brief Create an alias pointer of another shared pointer.
		 * 
		 * @details Points to the ptr, while using the same control block
		 * as the other shared pointer. Mainly used for pointer type casting.
		 * 
		 * @tparam T0 Other pointer element type
		 * @param other Other shared pointer
		 * @param ptr Raw pointer to point to
		 */
		template<typename T0>
		TSharedPtr(const TSharedPtr<T0>& other, TElement* ptr);

		/**
		 * @brief Create an alias pointer of another shared pointer.
		 *
		 * @details Points to the ptr, while using the same control block
		 * as the other shared pointer. Mainly used for pointer type casting.
		 *
		 * @tparam T0 Other pointer element type
		 * @param other Other shared pointer
		 * @param ptr Raw pointer to point to
		 */
		template<typename T0>
		TSharedPtr(TSharedPtr<T0>&& other, TElement* ptr);

		~TSharedPtr();

		/**
		 * @brief Copy assign other shared pointer to this pointer
		 * 
		 * @param other Other shared pointer
		 */
		TSharedPtr& operator=(const TSharedPtr& other);

		/**
		 * @brief Copy assign other shared pointer to this pointer
		 * 
		 * @tparam T0 Other pointer element type
		 * @param other Other shared pointer
		 */
		template<typename T0, TEnableIfT<TIsPtrCompatibleV<T0, T>>* = 0>
		TSharedPtr& operator=(const TSharedPtr<T0>& other);

		/**
		 * @brief Move assign other shared pointer to this pointer
		 * 
		 * @param other Other shared pointer
		 */
		TSharedPtr& operator=(TSharedPtr&& other);

		/**
		 * @brief Move assign other shared pointer to this pointer
		 * 
		 * @tparam T0 Other pointer element type
		 * @param other Other shared pointer
		 */
		template<typename T0, TEnableIfT<TIsPtrCompatibleV<T0, T>>* = 0>
		TSharedPtr& operator=(TSharedPtr<T0>&& other);

		/**
		 * @brief Make this a null pointer
		 */
		TSharedPtr& operator=(nullptr_t);

		/**
		 * @brief Check if two pointers point to the same object
		 *
		 * @param other Other shared pointer
		 */
		template<typename T0>
		bool operator==(const TSharedPtr<T0>& other);

		/**
		 * @brief Check if two pointers don't point to the same object
		 *
		 * @param other Other shared pointer
		 */
		template<typename T0>
		bool operator!=(const TSharedPtr<T0>& other);

		/**
		 * @brief Get the Raw pointer
		 * 
		 * @return T* Raw pointer
		 */
		T* Raw() const;

		/**
		 * @brief Access the pointer
		 */
		T* operator->() const;

		/**
		 * @brief Dereference the pointer
		 */
		T& operator*() const;

		/**
		 * @brief Check if this pointer is not null.
		 */
		bool IsValid() const;

		/**
		 * @see IsValid()
		 */
		operator bool() const;
	};

	#pragma region TSharedPtr Implementation

	template<typename T>
	inline TSharedPtr<T>::TSharedPtr() :
		TSharedPtr(nullptr)
	{
	}

	template<typename T>
	inline TSharedPtr<T>::TSharedPtr(nullptr_t) :
		TBase(nullptr)
	{
		RefCountLogger.Debug("TSharedPtr {{{}}} has been null constructed.", (void*)m_Rep);
	}

	template<typename T>
	template<typename T0, TEnableIfT<TIsPtrCompatibleV<T0, T>>*>
	inline TSharedPtr<T>::TSharedPtr(T0* ptr)
	{
		ConstructShared(ptr);
		RefCountLogger.Debug("TSharedPtr {{{}}} has been constructed with a pointer to {} {{{}}}.", (void*)m_Rep, typeid(T0).name(), (void*)ptr);
	}

	template<typename T>
	template<typename T0, typename FDeleter,
		TEnableIfT<TIsPtrCompatibleV<T0, T>>*,
		TEnableIfT<std::is_nothrow_invocable_v<FDeleter, T*>>*>
	inline TSharedPtr<T>::TSharedPtr(T0* ptr, FDeleter deleter)
	{
		ConstructSharedWithDeleter(ptr, deleter);
		RefCountLogger.Debug("TSharedPtr {{{}}} has been constructed with a pointer to {} {{{}}} and a custom deleter.", (void*)m_Rep, typeid(T0).name(), (void*)ptr);
	}

	template<typename T>
	inline TSharedPtr<T>::TSharedPtr(const TSharedPtr& other)
	{
		RefCountLogger.Debug("TSharedPtr {{{}}} has been copy constructed.", (void*)other.m_Rep);
		CopyConstructShared(other);
	}

	template<typename T>
	template<typename T0, TEnableIfT<TIsPtrCompatibleV<T0, T>>*>
	inline TSharedPtr<T>::TSharedPtr(const TSharedPtr<T0>& other)
	{
		RefCountLogger.Debug("TSharedPtr {{{}}} has been copy constructed.", (void*)other.m_Rep);
		CopyConstructShared(other);
	}

	template<typename T>
	inline TSharedPtr<T>::TSharedPtr(TSharedPtr&& other) noexcept
	{
		RefCountLogger.Debug("TSharedPtr {{{}}} has been move constructed.", (void*)other.m_Rep);
		MoveConstruct(Move(other));
	}

	template<typename T>
	template<typename T0, TEnableIfT<TIsPtrCompatibleV<T0, T>>*>
	inline TSharedPtr<T>::TSharedPtr(TSharedPtr<T0>&& other) noexcept
	{
		RefCountLogger.Debug("TSharedPtr {{{}}} has been move constructed.", (void*)other.m_Rep);
		MoveConstruct(Move(other));
	}

	template<typename T>
	template<typename T0>
	inline TSharedPtr<T>::TSharedPtr(const TSharedPtr<T0>& other, TElement* ptr)
	{
		RefCountLogger.Debug("TSharedPtr {{{}}} has been alias constructed with a pointer to {} {{{}}}.", (void*)other.m_Rep, typeid(TElement).name(), (void*)ptr);
		AliasConstructShared(other, ptr);
	}

	template<typename T>
	template<typename T0>
	inline TSharedPtr<T>::TSharedPtr(TSharedPtr<T0>&& other, TElement* ptr)
	{
		RefCountLogger.Debug("TSharedPtr {{{}}} has been alias move constructed with a pointer to {} {{{}}}.", (void*)other.m_Rep, typeid(TElement).name(), (void*)ptr);
		AliasConstructShared(Move(other), ptr);
	}

	template<typename T>
	inline TSharedPtr<T>::~TSharedPtr()
	{
		RefCountLogger.Debug("TSharedPtr {{{}}} has been destroyed.", (void*)m_Rep);
		DeleteShared();
	}

	template<typename T>
	inline TSharedPtr<T>& TSharedPtr<T>::operator=(const TSharedPtr& other)
	{
		RefCountLogger.Debug("TSharedPtr {{{}}} has been copy assigned to TSharedPtr {{{}}}.", (void*)other.m_Rep, (void*)m_Rep);
		TSharedPtr(other).Swap(*this);
		return *this;
	}

	template<typename T>
	template<typename T0, TEnableIfT<TIsPtrCompatibleV<T0, T>>*>
	inline TSharedPtr<T>& TSharedPtr<T>::operator=(const TSharedPtr<T0>& other)
	{
		RefCountLogger.Debug("TSharedPtr {{{}}} has been copy assigned to TSharedPtr {{{}}}.", (void*)other.m_Rep, (void*)m_Rep);
		TSharedPtr(other).Swap(*this);
		return *this;
	}

	template<typename T>
	inline TSharedPtr<T>& TSharedPtr<T>::operator=(TSharedPtr&& other)
	{
		RefCountLogger.Debug("TSharedPtr {{{}}} has been move assigned to TSharedPtr {{{}}}.", (void*)other.m_Rep, (void*)m_Rep);
		TSharedPtr(Move(other)).Swap(*this);
		return *this;
	}

	template<typename T>
	template<typename T0, TEnableIfT<TIsPtrCompatibleV<T0, T>>*>
	inline TSharedPtr<T>& TSharedPtr<T>::operator=(TSharedPtr<T0>&& other)
	{
		RefCountLogger.Debug("TSharedPtr {{{}}} has been move assigned to TSharedPtr {{{}}}.", (void*)other.m_Rep, (void*)m_Rep);
		TSharedPtr(Move(other)).Swap(*this);
		return *this;
	}

	template<typename T>
	inline TSharedPtr<T>& TSharedPtr<T>::operator=(nullptr_t)
	{
		RefCountLogger.Debug("Null has been assigned to TSharedPtr {{{}}}.", (void*)m_Rep);
		DeleteShared();
		return *this;
	}

	template<typename T>
	template<typename T0>
	inline bool TSharedPtr<T>::operator==(const TSharedPtr<T0>& other)
	{
		return m_Ptr == other.m_Ptr;
	}

	template<typename T>
	template<typename T0>
	inline bool TSharedPtr<T>::operator!=(const TSharedPtr<T0>& other)
	{
		return m_Ptr != other.m_Ptr;
	}

	template<typename T>
	inline T* TSharedPtr<T>::Raw() const
	{
		return m_Ptr;
	}

	template<typename T>
	inline T* TSharedPtr<T>::operator->() const
	{
		ionassert(m_Ptr);
		return m_Ptr;
	}

	template<typename T>
	inline T& TSharedPtr<T>::operator*() const
	{
		ionassert(m_Ptr);
		return *m_Ptr;
	}

	template<typename T>
	inline bool TSharedPtr<T>::IsValid() const
	{
		return m_Rep && m_Ptr;
	}

	template<typename T>
	inline TSharedPtr<T>::operator bool() const
	{
		return IsValid();
	}

	#pragma endregion

#pragma endregion

#pragma region TWeakPtr

	template<typename T>
	class TWeakPtr : public TPtrBase<T>
	{
	public:
		using TBase = TPtrBase<T>;

		/**
		 * @brief Construct a null weak pointer
		 */
		TWeakPtr();

		/**
		 * @brief Construct a null weak pointer
		 */
		TWeakPtr(nullptr_t);

		/**
		 * @brief Construct a Weak pointer based on a Shared pointer
		 * 
		 * @tparam T0 Shared pointer element type
		 * @param ptr Shared pointer
		 */
		template<typename T0, TEnableIfT<TIsPtrCompatibleV<T0, T>>* = 0>
		TWeakPtr(const TSharedPtr<T0>& shared);

		/**
		 * @brief Make a copy of another weak pointer.
		 * 
		 * @param other Other weak pointer
		 */
		TWeakPtr(const TWeakPtr& other);

		/**
		 * @brief Make a copy of another weak pointer.
		 * 
		 * @tparam T0 Other pointer element type
		 * @param other Other weak pointer
		 */
		template<typename T0, TEnableIfT<TIsPtrCompatibleV<T0, T>>* = 0>
		TWeakPtr(const TWeakPtr<T0>& other);

		/**
		 * @brief Move another weak pointer.
		 * 
		 * @param other Other weak pointer
		 */
		TWeakPtr(TWeakPtr&& other);

		/**
		 * @brief Move another weak pointer.
		 * 
		 * @tparam T0 Other pointer element type
		 * @param other Other weak pointer
		 */
		template<typename T0, TEnableIfT<TIsPtrCompatibleV<T0, T>>* = 0>
		TWeakPtr(TWeakPtr<T0>&& other);

		~TWeakPtr();

		/**
		 * @brief Copy assign other weak pointer to this pointer
		 * 
		 * @param other Other weak pointer
		 */
		TWeakPtr& operator=(const TWeakPtr& other);

		/**
		 * @brief Copy assign other weak pointer to this pointer
		 * 
		 * @tparam T0 Other pointer element type
		 * @param other Other weak pointer
		 */
		template<typename T0, TEnableIfT<TIsPtrCompatibleV<T0, T>>* = 0>
		TWeakPtr& operator=(const TWeakPtr<T0>& other);

		/**
		 * @brief Move assign other weak pointer to this pointer
		 * 
		 * @param other Other weak pointer
		 */
		TWeakPtr& operator=(TWeakPtr&& other) noexcept;

		/**
		 * @brief Move assign other weak pointer to this pointer
		 * 
		 * @tparam T0 Other pointer element type
		 * @param other Other weak pointer
		 */
		template<typename T0, TEnableIfT<TIsPtrCompatibleV<T0, T>>* = 0>
		TWeakPtr& operator=(TWeakPtr<T0>&& other) noexcept;

		/**
		 * @brief Assign a shared pointer to this weak pointer.
		 *
		 * @tparam T0 Shared pointer element type
		 * @param shared Shared pointer
		 */
		template<typename T0, TEnableIfT<TIsPtrCompatibleV<T0, T>>* = 0>
		TWeakPtr& operator=(const TSharedPtr<T0>& shared);

		/**
		 * @brief Make this a null weak pointer
		 * 
		 * @return This pointer
		 */
		TWeakPtr& operator=(nullptr_t);

		/**
		 * @brief Make a shared pointer out of this weak pointer
		 * 
		 * @return If the weak pointer has not expired, a shared pointer
		 * that points to the same object, else a null shared pointer.
		 */
		TSharedPtr<T> Lock() const;

		/**
		 * @brief Get the Raw pointer. Make sure to call IsExpired() 
		 * to check if the pointer is valid before dereferencing it.
		 *
		 * @return T* Raw pointer
		 */
		T* Raw() const noexcept;

		/**
		 * @brief Checks if the weak pointer no longer points to a valid object
		 * It will return false if the pointer is null.
		 */
		bool IsExpired() const;

		/**
		 * @brief Check if this pointer is not null and not expired.
		 */
		bool IsValid() const;

		/**
		 * @see IsValid()
		 */
		operator bool() const;
	};

	#pragma region TWeakPtr Implementation

	template<typename T>
	inline TWeakPtr<T>::TWeakPtr() :
		TWeakPtr(nullptr)
	{
	}

	template<typename T>
	inline TWeakPtr<T>::TWeakPtr(nullptr_t) :
		TBase(nullptr)
	{
		RefCountLogger.Debug("TWeakPtr {{{}}} has been null constructed.", (void*)m_Ptr);
	}

	template<typename T>
	template<typename T0, TEnableIfT<TIsPtrCompatibleV<T0, T>>*>
	inline TWeakPtr<T>::TWeakPtr(const TSharedPtr<T0>& ptr)
	{
		ConstructWeak(ptr);
		RefCountLogger.Debug("TWeakPtr {{{}}} has been constructed from TSharedPtr {{{}}}.", (void*)m_Rep, (void*)ptr.m_Rep);
	}

	template<typename T>
	inline TWeakPtr<T>::TWeakPtr(const TWeakPtr& other)
	{
		RefCountLogger.Debug("TWeakPtr {{{}}} has been copy constructed.", (void*)other.m_Rep);
		CopyConstructWeak(other);
	}

	template<typename T>
	template<typename T0, TEnableIfT<TIsPtrCompatibleV<T0, T>>*>
	inline TWeakPtr<T>::TWeakPtr(const TWeakPtr<T0>& other)
	{
		RefCountLogger.Debug("TWeakPtr {{{}}} has been copy constructed.", (void*)other.m_Rep);
		CopyConstructWeak(other);
	}

	template<typename T>
	inline TWeakPtr<T>::TWeakPtr(TWeakPtr&& other)
	{
		RefCountLogger.Debug("TWeakPtr {{{}}} has been move constructed.", (void*)other.m_Rep);
		MoveConstruct(Move(other));
	}

	template<typename T>
	template<typename T0, TEnableIfT<TIsPtrCompatibleV<T0, T>>*>
	inline TWeakPtr<T>::TWeakPtr(TWeakPtr<T0>&& other)
	{
		RefCountLogger.Debug("TWeakPtr {{{}}} has been move constructed.", (void*)other.m_Rep);
		MoveConstruct(Move(other));
	}

	template<typename T>
	inline TWeakPtr<T>::~TWeakPtr()
	{
		RefCountLogger.Debug("TWeakPtr {{{}}} has been destroyed.", (void*)m_Rep);
		DeleteWeak();
	}

	template<typename T>
	inline TWeakPtr<T>& TWeakPtr<T>::operator=(const TWeakPtr& other)
	{
		RefCountLogger.Debug("TWeakPtr {{{}}} has been copy assigned to TWeakPtr {{{}}}.", (void*)other.m_Rep, (void*)m_Rep);
		TWeakPtr(other).Swap(*this);
		return *this;
	}

	template<typename T>
	template<typename T0, TEnableIfT<TIsPtrCompatibleV<T0, T>>*>
	inline TWeakPtr<T>& TWeakPtr<T>::operator=(const TWeakPtr<T0>& other)
	{
		RefCountLogger.Debug("TWeakPtr {{{}}} has been copy assigned to TWeakPtr {{{}}}.", (void*)other.m_Rep, (void*)m_Rep);
		TWeakPtr(other).Swap(*this);
		return *this;
	}

	template<typename T>
	inline TWeakPtr<T>& TWeakPtr<T>::operator=(TWeakPtr&& other) noexcept
	{
		RefCountLogger.Debug("TWeakPtr {{{}}} has been move assigned to TWeakPtr {{{}}}.", (void*)other.m_Rep, (void*)m_Rep);
		TWeakPtr(Move(other)).Swap(*this);
		return *this;
	}

	template<typename T>
	template<typename T0, TEnableIfT<TIsPtrCompatibleV<T0, T>>*>
	inline TWeakPtr<T>& TWeakPtr<T>::operator=(TWeakPtr<T0>&& other) noexcept
	{
		RefCountLogger.Debug("TWeakPtr {{{}}} has been move assigned to TWeakPtr {{{}}}.", (void*)other.m_Rep, (void*)m_Rep);
		TWeakPtr(Move(other)).Swap(*this);
		return *this;
	}

	template<typename T>
	template<typename T0, TEnableIfT<TIsPtrCompatibleV<T0, T>>*>
	inline TWeakPtr<T>& TWeakPtr<T>::operator=(const TSharedPtr<T0>& shared)
	{
		RefCountLogger.Debug("TSharedPtr {{{}}} has been assigned to TWeakPtr {{{}}}.", (void*)shared.m_Rep, (void*)m_Rep);
		TWeakPtr(shared).Swap(*this);
		return *this;
	}

	template<typename T>
	inline TWeakPtr<T>& TWeakPtr<T>::operator=(nullptr_t)
	{
		RefCountLogger.Debug("Null has been assigned to TWeakPtr {{{}}}.", (void*)m_Rep);
		DeleteWeak();
		return *this;
	}

	template<typename T>
	inline TSharedPtr<T> TWeakPtr<T>::Lock() const
	{
		if (IsExpired())
		{
			return TSharedPtr<T>();
		}

		RefCountLogger.Debug("TWeakPtr {{{}}} has been locked.", (void*)m_Rep);
		TSharedPtr<T> shared;
		shared.ConstructSharedFromWeak(*this);
		RefCountLogger.Debug("TSharedPtr {{{}}} has been constructed from TWeakPtr {{{}}}.", (void*)shared.m_Rep, (void*)m_Rep);
		return shared;
	}

	template<typename T>
	inline T* TWeakPtr<T>::Raw() const noexcept
	{
		return m_Ptr;
	}

	template<typename T>
	inline bool TWeakPtr<T>::IsExpired() const
	{
		return m_Rep && RefCount() == 0;
	}

	template<typename T>
	inline bool TWeakPtr<T>::IsValid() const
	{
		return m_Rep && m_Ptr && (RefCount() > 0);
	}

	template<typename T>
	inline TWeakPtr<T>::operator bool() const
	{
		return IsValid();
	}

	#pragma endregion

#pragma endregion

#pragma region Enable Shared From This

	template<typename T>
	class TEnableSFT
	{
	public:
		using TEnableSFTType = TEnableSFT;

		TSharedPtr<T> SharedFromThis();
		TSharedPtr<const T> SharedFromThis() const;
		TWeakPtr<T> WeakFromThis() noexcept;
		TWeakPtr<const T> WeakFromThis() const noexcept;

	protected:
		TEnableSFT() noexcept;
		TEnableSFT(const TEnableSFT&) noexcept;
		~TEnableSFT() = default;
		TEnableSFT& operator=(const TEnableSFT&) noexcept;

	private:
		mutable TWeakPtr<T> m_WeakSFT;

		template<typename T0>
		friend class TPtrBase;
	};

	#pragma region TEnableSFT Implementation

	template<typename T>
	inline TSharedPtr<T> TEnableSFT<T>::SharedFromThis()
	{
		return m_WeakSFT.Lock();
	}

	template<typename T>
	inline TSharedPtr<const T> TEnableSFT<T>::SharedFromThis() const
	{
		return m_WeakSFT.Lock();
	}

	template<typename T>
	inline TWeakPtr<T> TEnableSFT<T>::WeakFromThis() noexcept
	{
		return m_WeakSFT;
	}

	template<typename T>
	inline TWeakPtr<const T> TEnableSFT<T>::WeakFromThis() const noexcept
	{
		return m_WeakSFT;
	}

	template<typename T>
	inline TEnableSFT<T>::TEnableSFT() noexcept :
		m_WeakSFT()
	{
	}

	template<typename T>
	inline TEnableSFT<T>::TEnableSFT(const TEnableSFT&) noexcept :
		m_WeakSFT()
	{
	}

	template<typename T>
	inline TEnableSFT<T>& TEnableSFT<T>::operator=(const TEnableSFT&) noexcept
	{
		return *this;
	}

	#pragma endregion

#pragma endregion

#pragma region PtrCast / DynamicPtrCast

	/**
	 * @brief Cast a shared pointer to another type.
	 * 
	 * @tparam T1 Type to cast to
	 * @tparam T2 Type to cast from
	 * @param other Other shared pointer
	 * @return A new, statically cast shared pointer
	 */
	template<typename T1, typename T2>
	inline TSharedPtr<T1> PtrCast(const TSharedPtr<T2>& other)
	{
		T2* rawPtr = other.Raw();
		RefCountLogger.Debug("TSharedPtr {{{}}} has been copy cast to {}.", (void*)rawPtr, typeid(T1).name());
		return TSharedPtr<T1>(other, static_cast<T1*>(rawPtr));
	}

	/**
	 * @brief Cast a shared pointer to another type.
	 * 
	 * @tparam T1 Type to cast to
	 * @tparam T2 Type to cast from
	 * @param other Other shared pointer
	 * @return A new, statically cast shared pointer
	 */
	template<typename T1, typename T2>
	inline TSharedPtr<T1> PtrCast(TSharedPtr<T2>&& other)
	{
		T2* rawPtr = other.Raw();
		RefCountLogger.Debug("TSharedPtr {{{}}} has been move cast to {}.", (void*)rawPtr, typeid(T1).name());
		return TSharedPtr<T1>(Move(other), static_cast<T1*>(rawPtr));
	}

	/**
	 * @brief Dynamically cast a shared pointer to another type.
	 *
	 * @tparam T1 Type to cast to
	 * @tparam T2 Type to cast from
	 * @param other Other shared pointer
	 * @return On a succeeded cast, a statically cast shared pointer; null shared pointer otherwise
	 */
	template<typename T1, typename T2>
	inline TSharedPtr<T1> DynamicPtrCast(const TSharedPtr<T2>& other)
	{
		if (!dynamic_cast<T1*>(other.Raw()))
			return TSharedPtr<T1>();

		return PtrCast<T1>(other);
	}

	/**
	 * @brief Dynamically cast a shared pointer to another type.
	 *
	 * @tparam T1 Type to cast to
	 * @tparam T2 Type to cast from
	 * @param other Other shared pointer
	 * @return On a succeeded cast, a statically cast shared pointer; null shared pointer otherwise
	 */
	template<typename T1, typename T2>
	inline TSharedPtr<T1> DynamicPtrCast(TSharedPtr<T2>&& other)
	{
		if (!dynamic_cast<T1*>(other.Raw()))
			return TSharedPtr<T1>();

		return PtrCast<T1>(Move(other));
	}

#pragma endregion

#pragma region MakeShared / MakeSharedDC

	/**
	 * @brief Make a shared pointer with an element constructed in-place.
	 */
	template<typename T, typename... Args>
	inline TSharedPtr<T> MakeShared(Args&&... args)
	{
		TSharedPtr<T> ptr;
		ptr.ConstructSharedInPlace(Forward<Args>(args)...);
		RefCountLogger.Debug("TSharedPtr {{{}}} has been constructed in place with an object of type {}.", (void*)ptr.m_Rep, typeid(T).name());
		return ptr;
	}

	/**
	 * @brief Make a shared pointer with an element of type T constructed in place
	 * and a custom destroy callback, that will be called right before the object is destroyed.
	 *
	 * @tparam FOnDestroy Destroy callback type - void(T&)
	 *
	 * @param onDestroy Destroy callback function, called before the object is destroyed.
	 */
	template<typename T, typename FOnDestroy, typename... Args>
	inline TEnableIfT<std::is_invocable_v<FOnDestroy, TRemoveRef<T>&>, TSharedPtr<T>> MakeSharedDC(FOnDestroy onDestroy, Args&&... args)
	{
		TSharedPtr<T> ptr;
		ptr.ConstructSharedInPlaceWithDestroyCallback(onDestroy, Forward<Args>(args)...);
		RefCountLogger.Debug("TSharedPtr {{{}}} has been constructed in place with an object of type {} and a destroy callback.", (void*)ptr.m_Rep, typeid(T).name());
		return ptr;
	}

#pragma endregion

#pragma region MakeSharedFrom

	/**
	 * @brief Make a shared pointer from a raw pointer and take the ownership of it.
	 * 
	 * @tparam T Pointer type
	 * @param ptr Raw pointer
	 */
	template<typename T>
	inline TSharedPtr<T> MakeSharedFrom(T* ptr)
	{
		return TSharedPtr<T>(ptr);
	}

	/**
	 * @brief Make a shared pointer with a custom deleter from a raw pointer and take the ownership of it.
	 * 
	 * @tparam T Pointer type
	 * @tparam FDeleter Deleter function type - void(T*)
	 * 
	 * @param ptr Raw pointer
	 * @param deleter Deleter function, called when the ref count reached zero.
	 */
	template<typename T, typename FDeleter, TEnableIfT<std::is_nothrow_invocable_v<FDeleter, T*>>* = 0>
	inline TSharedPtr<T> MakeSharedFrom(T* ptr, FDeleter deleter)
	{
		return TSharedPtr<T>(ptr, deleter);
	}

#pragma endregion

	int RefCountPtrTest();

#pragma endregion
}
