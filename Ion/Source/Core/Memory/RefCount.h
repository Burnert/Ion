#pragma once

#include "MemoryCore.h"

namespace Ion
{
	REGISTER_LOGGER(RefCountLogger, "Core::Memory::RefCount", /*ELoggerFlags::DisabledByDefault*/);

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

		size_t GetRefCount() const;

		RefCountable(const RefCountable&) = delete;
		RefCountable(RefCountable&&) = delete;
		RefCountable& operator=(const RefCountable&) = delete;
		RefCountable& operator=(RefCountable&&) = delete;

	private:
		size_t m_Count;

		size_t IncRef();
		size_t DecRef();

		void DeleteThis();

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

	FORCEINLINE size_t RefCountable::GetRefCount() const
	{
		return m_Count;
	}

	inline size_t RefCountable::IncRef()
	{
		++m_Count;
		RefCountLogger.Debug("RefCountable {{{}}} incremented the ref count by 1. Current ref count: {}", (void*)this, m_Count);
		return m_Count;
	}

	inline size_t RefCountable::DecRef()
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

	inline void RefCountable::DeleteThis()
	{
		delete this;
		RefCountLogger.Debug("RefCountable {{{}}} deleted ref countable object.", (void*)this);
	}

	#pragma endregion

#pragma endregion

#pragma region TRef

	template<typename T>
	class TRef
	{
	public:
		static_assert(TIsBaseOfV<RefCountable, T>, "Object cannot be used with TRef if it's not derived from RefCountable.");

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

		size_t GetRefCount() const;

		T* GetRaw() const;
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
		RefCountLogger.Debug("TRef {{{}}} has been constructed.", (void*)m_Object);
		if (m_Object)
		{
			m_Object->IncRef();
		}
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
	inline size_t TRef<T>::GetRefCount() const
	{
		if (!m_Object)
			return 0;

		return m_Object->GetRefCount();
	}

	template<typename T>
	inline typename T* TRef<T>::GetRaw() const
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
		if (m_Object)
		{
			m_Object->DecRef();
			m_Object = nullptr;
		}
	}

	template<typename T>
	template<typename T0>
	inline void TRef<T>::CopyConstruct(const TRef<T0>& other)
	{
		m_Object = other.m_Object;
		RefCountLogger.Debug("TRef {{{}}} has been copy constructed.", (void*)m_Object);
		if (m_Object)
		{
			m_Object->IncRef();
		}
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
		if (m_Object)
		{
			m_Object->IncRef();
		}
	}

	template<typename T>
	template<typename T0>
	inline void TRef<T>::CastConstruct(TRef<T0>&& other) noexcept
	{
		m_Object = static_cast<T*>(other.m_Object);
		RefCountLogger.Debug("TRef {{{}}} has been move cast constructed.", (void*)m_Object);

		other.m_Object = nullptr;
	}

	#pragma endregion

#pragma endregion

#pragma region MakeRef / RefCast / DynamicRefCast

	template<typename T, typename... Args>
	inline TRef<T> MakeRef(Args&&... args)
	{
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
		if (!dynamic_cast<T0*>(other.GetRaw()))
			return TRef<T0>();

		return RefCast<T0>(other);
	}

	template<typename T0, typename T1>
	inline TRef<T0> DynamicRefCast(TRef<T1>&& other)
	{
		if (!dynamic_cast<T0*>(other.GetRaw()))
			return TRef<T0>();

		return RefCast<T0>(Move(other));
	}

#pragma endregion

	int RefCountTest();

#pragma endregion

#pragma region Non-Intrusive

#pragma region Templates / Fwd

	template<typename T>
	class TSharedPtrBase;

	template<typename T>
	class TSharedPtr;

	template<typename T>
	class TWeakPtr;

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
	struct TIsRefCountPtr<TSharedPtrBase<T>>
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

#pragma endregion

#pragma region TRefCounter

	/**
	 * @brief Ref counting control block for shared pointers
	 * 
	 * @tparam T Type of the element that the ref counter owns.
	 */
	template<typename T>
	class TRefCounter
	{
		static_assert(!TIsReferenceV<T>);

		/**
		 * @brief Construct a new RefCount object that owns the pointer
		 * 
		 * @param ptr Pointer to take the ownership of.
		 */
		TRefCounter(T* ptr);

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

	private:
		/**
		 * @brief Deletes the owned object.
		 */
		void Delete();

		/**
		 * @brief Destroys this control block.
		 */
		void DeleteRefCounter();

	private:
		/**
		 * @brief Owned pointer
		 */
		T* m_Ptr;

		/**
		 * @brief Current reference count
		 */
		uint32 m_RefCount;
		/**
		 * @brief Current weak reference count
		 * 
		 * @details It is 1 if there are no weak pointers.
		 * The ref counting block kind of is a weak reference.
		 */
		uint32 m_WeakCount;

		template<typename T0>
		friend class TRefCountPtrBase;
	};

	#pragma region TRefCounter Implementation

	template<typename T>
	inline TRefCounter<T>::TRefCounter(T* ptr) :
		m_Ptr(ptr),
		m_RefCount(1),
		m_WeakCount(1)
	{
		RefCountLogger.Trace("{{{}}} Constructed a ref counting block.", (void*)m_Ptr);
	}

	template<typename T>
	inline uint32 TRefCounter<T>::IncRef()
	{
		uint32 count = ++m_RefCount;
		RefCountLogger.Debug("{{{}}} Incremented the ref count by 1. Current ref count: {}", (void*)m_Ptr, count);
		return count;
	}

	template<typename T>
	inline uint32 TRefCounter<T>::DecRef()
	{
		uint32 count = --m_RefCount;
		RefCountLogger.Debug("{{{}}} Decremented the ref count by 1. Current ref count: {}", (void*)m_Ptr, count);
		if (count == 0)
		{
			RefCountLogger.Trace("{{{}}} Strong ref count reached 0.", (void*)m_Ptr);
			Delete();
			DecWeak();
		}
		return count;
	}

	template<typename T>
	inline uint32 TRefCounter<T>::IncWeak()
	{
		uint32 count = ++m_WeakCount;
		RefCountLogger.Debug("{{{}}} Incremented the weak ref count by 1. Current weak ref count: {}", (void*)m_Ptr, count);
		return count;
	}

	template<typename T>
	inline uint32 TRefCounter<T>::DecWeak()
	{
		uint32 count = --m_WeakCount;
		RefCountLogger.Debug("{{{}}} Decremented the weak ref count by 1. Current weak ref count: {}", (void*)m_Ptr, count);
		if (count == 0)
		{
			RefCountLogger.Trace("{{{}}} Weak ref count reached 0.", (void*)m_Ptr);
			DeleteRefCounter();
		}
		return count;
	}

	template<typename T>
	inline void TRefCounter<T>::Delete()
	{
		RefCountLogger.Trace("Deleting owned object...");
		if (m_Ptr)
		{
			delete m_Ptr;
			RefCountLogger.Debug("{{{}}} Deleted owned object.", (void*)m_Ptr);
		}
	}

	template<typename T>
	inline void TRefCounter<T>::DeleteRefCounter()
	{
		RefCountLogger.Trace("Deleting ref counting block...");
		void* ptr = m_Ptr;
		delete this;
		RefCountLogger.Debug("{{{}}} Deleted ref counting block.", ptr);
	}

	#pragma endregion

#pragma endregion

#pragma region TRefCountPtrBase

	template<typename T>
	class TRefCountPtrBase
	{
	public:
		using TElement  = T;
		using TThis     = TRefCountPtrBase<T>;
		using TRep      = TRefCounter<T>;

		/**
		 * @brief Checks if the weak pointer no longer points to a valid object
		 */
		bool IsExpired() const;

		/**
		 * @brief Checks if the pointer is not null
		 */
		operator bool() const;

	protected:
		/**
		 * @brief Construct a null pointer base
		 */
		TRefCountPtrBase();

		/**
		 * @brief Construct a null pointer base
		 */
		TRefCountPtrBase(nullptr_t);

		/**
		 * @brief Makes this pointer own the ptr and constructs a ref count control block.
		 * 
		 * @param ptr Pointer to take the ownership of.
		 */
		void ConstructShared(T* ptr);

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

		void Swap(TRefCountPtrBase& other);

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
		TRep* m_RefCount;

		template<typename T0>
		friend class TSharedPtr;

		template<typename T0>
		friend class TWeakPtr;

		template<typename T0>
		friend class TRefCountPtrBase;
	};

	#pragma region TRefCountPtrBase Implementation

	template<typename T>
	inline bool TRefCountPtrBase<T>::IsExpired() const
	{
		ionassert(m_RefCount);
		return m_RefCount->m_RefCount == 0;
	}

	template<typename T>
	inline TRefCountPtrBase<T>::operator bool() const
	{
		return m_Ptr && m_RefCount;
	}

	template<typename T>
	inline TRefCountPtrBase<T>::TRefCountPtrBase() :
		TRefCountPtrBase(nullptr)
	{
	}

	template<typename T>
	inline TRefCountPtrBase<T>::TRefCountPtrBase(nullptr_t) :
		m_Ptr(nullptr),
		m_RefCount(nullptr)
	{
	}

	template<typename T>
	inline void TRefCountPtrBase<T>::ConstructShared(T* ptr)
	{
		ionassert(ptr);

		m_Ptr = ptr;
		m_RefCount = new TRep(ptr);
	}

	template<typename T>
	template<typename T0>
	inline void TRefCountPtrBase<T>::ConstructSharedFromWeak(const TWeakPtr<T0>& ptr)
	{
		static_assert(TIsBaseOfV<TElement, T0>);

		if (ptr.m_RefCount)
		{
			ionassert(ptr.m_Ptr);

			m_Ptr = ptr.m_Ptr;
			m_RefCount = ptr.m_RefCount;

			m_RefCount->IncRef();
		}
	}

	template<typename T>
	template<typename T0>
	inline void TRefCountPtrBase<T>::ConstructWeak(const TSharedPtr<T0>& ptr)
	{
		static_assert(TIsBaseOfV<TElement, T0>);

		if (ptr.m_RefCount)
		{
			ionassert(ptr.m_Ptr);

			m_Ptr = ptr.m_Ptr;
			m_RefCount = ptr.m_RefCount;

			m_RefCount->IncWeak();
		}
	}

	template<typename T>
	template<typename T0>
	inline void TRefCountPtrBase<T>::CopyConstructShared(const TSharedPtr<T0>& other)
	{
		static_assert(TIsBaseOfV<TElement, T0>);

		m_Ptr = (TElement*)other.m_Ptr;
		m_RefCount = (TRep*)other.m_RefCount;

		if (other.m_RefCount)
		{
			ionassert(other.m_Ptr);

			m_RefCount->IncRef();
		}
	}

	template<typename T>
	template<typename T0>
	inline void TRefCountPtrBase<T>::CopyConstructWeak(const TWeakPtr<T0>& other)
	{
		static_assert(TIsBaseOfV<TElement, T0>);

		m_Ptr = (TElement*)other.m_Ptr;
		m_RefCount = (TRep*)other.m_RefCount;

		if (other.m_RefCount)
		{
			ionassert(other.m_Ptr);

			m_RefCount->IncWeak();
		}
	}

	template<typename T>
	template<typename TPtr>
	inline void TRefCountPtrBase<T>::MoveConstruct(TPtr&& other)
	{
		static_assert(TIsRefCountPtrV<TPtr>);
		static_assert(TIsBaseOfV<TElement, typename TPtr::TElement>);

		m_Ptr = (TElement*)other.m_Ptr;
		m_RefCount = (TRep*)other.m_RefCount;

		other.m_Ptr = nullptr;
		other.m_RefCount = nullptr;
	}

	template<typename T>
	template<typename T0>
	inline void TRefCountPtrBase<T>::AliasConstructShared(const TSharedPtr<T0>& other, TElement* ptr)
	{
		static_assert(TOrV<TIsBaseOf<TElement, T0>, TIsBaseOf<T0, TElement>>);

		m_Ptr = ptr;
		m_RefCount = (TRep*)other.m_RefCount;

		if (other.m_RefCount)
		{
			ionassert(other.m_Ptr);

			m_RefCount->IncRef();
		}
	}

	template<typename T>
	inline void TRefCountPtrBase<T>::Swap(TRefCountPtrBase<T>& other)
	{
		std::swap(m_Ptr, other.m_Ptr);
		std::swap(m_RefCount, other.m_RefCount);
	}

	template<typename T>
	inline void TRefCountPtrBase<T>::DeleteShared()
	{
		if (m_RefCount)
		{
			m_RefCount->DecRef();
		}

		m_Ptr = nullptr;
		m_RefCount = nullptr;
	}

	template<typename T>
	inline void TRefCountPtrBase<T>::DeleteWeak()
	{
		if (m_RefCount)
		{
			m_RefCount->DecWeak();
		}

		m_Ptr = nullptr;
		m_RefCount = nullptr;
	}

	#pragma endregion

#pragma endregion

#pragma region TSharedPtr

	template<typename T>
	class TSharedPtr : public TRefCountPtrBase<T>
	{
	public:
		using TBase = TRefCountPtrBase<T>;
		using TRep  = typename TBase::TRep;

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
		explicit TSharedPtr(T* ptr);

		TSharedPtr(const TSharedPtr& other);

		/**
		 * @brief Make a copy of another shared pointer.
		 * 
		 * @tparam T0 Other pointer element type
		 * @param other Other shared pointer
		 */
		template<typename T0>
		TSharedPtr(const TSharedPtr<T0>& other);

		TSharedPtr(TSharedPtr&& other) noexcept;

		/**
		 * @brief Move another shared pointer.
		 * 
		 * @tparam T0 Other pointer element type
		 * @param other Other shared pointer
		 */
		template<typename T0>
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

		TSharedPtr& operator=(const TSharedPtr& other);

		/**
		 * @brief Copy assign other shared pointer to this pointer
		 * 
		 * @tparam T0 Other pointer element type
		 * @param other Other shared pointer
		 * @return This pointer
		 */
		template<typename T0>
		TSharedPtr& operator=(const TSharedPtr<T0>& other);

		TSharedPtr& operator=(TSharedPtr&& other);

		/**
		 * @brief Move assign other shared pointer to this pointer
		 * 
		 * @tparam T0 Other pointer element type
		 * @param other Other shared pointer
		 * @return This pointer
		 */
		template<typename T0>
		TSharedPtr& operator=(TSharedPtr<T0>&& other);

		/**
		 * @brief Make this a null pointer
		 * 
		 * @return This pointer
		 */
		TSharedPtr& operator=(nullptr_t);

		/**
		 * @brief Get the Raw pointer
		 * 
		 * @return Raw pointer
		 */
		T* GetRaw() const;

		/**
		 * @brief Access the pointer
		 */
		T* operator->() const;

		/**
		 * @brief Dereference the pointer
		 */
		T& operator*() const;

		~TSharedPtr();
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
	}

	template<typename T>
	inline TSharedPtr<T>::TSharedPtr(T* ptr)
	{
		ConstructShared(ptr);
		RefCountLogger.Debug("{{{}}} TSharedPtr has been constructed.", (void*)ptr);
	}

	template<typename T>
	inline TSharedPtr<T>::TSharedPtr(const TSharedPtr& other)
	{
		CopyConstructShared(other);
		RefCountLogger.Debug("{{{}}} TSharedPtr has been copy constructed.", (void*)m_Ptr);
	}

	template<typename T>
	template<typename T0>
	inline TSharedPtr<T>::TSharedPtr(const TSharedPtr<T0>& other)
	{
		CopyConstructShared(other);
		RefCountLogger.Debug("{{{}}} TSharedPtr has been copy constructed.", (void*)m_Ptr);
	}

	template<typename T>
	inline TSharedPtr<T>::TSharedPtr(TSharedPtr&& other) noexcept
	{
		MoveConstruct(Move(other));
		RefCountLogger.Debug("{{{}}} TSharedPtr has been move constructed.", (void*)m_Ptr);
	}

	template<typename T>
	template<typename T0>
	inline TSharedPtr<T>::TSharedPtr(TSharedPtr<T0>&& other) noexcept
	{
		MoveConstruct(Move(other));
		RefCountLogger.Debug("{{{}}} TSharedPtr has been move constructed.", (void*)m_Ptr);
	}

	template<typename T>
	template<typename T0>
	inline TSharedPtr<T>::TSharedPtr(const TSharedPtr<T0>& other, TElement* ptr)
	{
		AliasConstructShared(other, ptr);
		RefCountLogger.Debug("{{{}}} TSharedPtr has been alias constructed.", (void*)m_Ptr);
	}

	template<typename T>
	inline TSharedPtr<T>& TSharedPtr<T>::operator=(const TSharedPtr& other)
	{
		TSharedPtr(other).Swap(*this);
		RefCountLogger.Debug("{{{}}} TSharedPtr has been copy assigned.", (void*)m_Ptr);
		return *this;
	}

	template<typename T>
	template<typename T0>
	inline TSharedPtr<T>& TSharedPtr<T>::operator=(const TSharedPtr<T0>& other)
	{
		TSharedPtr(other).Swap(*this);
		RefCountLogger.Debug("{{{}}} TSharedPtr has been copy assigned.", (void*)m_Ptr);
		return *this;
	}

	template<typename T>
	inline TSharedPtr<T>& TSharedPtr<T>::operator=(TSharedPtr&& other)
	{
		TSharedPtr(Move(other)).Swap(*this);
		RefCountLogger.Debug("{{{}}} TSharedPtr has been move assigned.", (void*)m_Ptr);
		return *this;
	}

	template<typename T>
	template<typename T0>
	inline TSharedPtr<T>& TSharedPtr<T>::operator=(TSharedPtr<T0>&& other)
	{
		TSharedPtr(Move(other)).Swap(*this);
		RefCountLogger.Debug("{{{}}} TSharedPtr has been move assigned.", (void*)m_Ptr);
		return *this;
	}

	template<typename T>
	inline TSharedPtr<T>& TSharedPtr<T>::operator=(nullptr_t)
	{
		DeleteShared();
		RefCountLogger.Debug("{{{}}} TSharedPtr has been null assigned.", (void*)m_Ptr);
		return *this;
	}

	template<typename T>
	inline T* TSharedPtr<T>::GetRaw() const
	{
		return m_Ptr;
	}

	template<typename T>
	inline T* TSharedPtr<T>::operator->() const
	{
		return GetRaw();
	}

	template<typename T>
	inline T& TSharedPtr<T>::operator*() const
	{
		ionassert(m_Ptr);
		return *GetRaw();
	}

	template<typename T>
	inline TSharedPtr<T>::~TSharedPtr()
	{
		DeleteShared();
	}

	#pragma endregion

#pragma endregion

#pragma region TWeakPtr

	template<typename T>
	class TWeakPtr : public TRefCountPtrBase<T>
	{
	public:
		using TBase = TRefCountPtrBase<T>;
		using TRep  = typename TBase::TRep;

		/**
		 * @brief Construct a null weak pointer
		 */
		TWeakPtr();

		/**
		 * @brief Construct a null weak pointer
		 */
		TWeakPtr(nullptr_t);

		/**
		 * @brief Construct a Weak pointer based on the other Shared pointer
		 * 
		 * @tparam T0 Other shared pointer element type
		 * @param ptr Other shared pointer
		 */
		template<typename T0>
		TWeakPtr(const TSharedPtr<T0>& ptr);

		TWeakPtr(const TWeakPtr& other);

		/**
		 * @brief Make a copy of another weak pointer.
		 * 
		 * @tparam T0 Other pointer element type
		 * @param other Other weak pointer
		 */
		template<typename T0>
		TWeakPtr(const TWeakPtr<T0>& other);

		TWeakPtr(TWeakPtr&& other);

		/**
		 * @brief Move another weak pointer.
		 * 
		 * @tparam T0 Other pointer element type
		 * @param other Other weak pointer
		 */
		template<typename T0>
		TWeakPtr(TWeakPtr<T0>&& other);

		TWeakPtr& operator=(const TWeakPtr& other);

		/**
		 * @brief Copy assign other weak pointer to this pointer
		 * 
		 * @tparam T0 Other pointer element type
		 * @param other Other weak pointer
		 * @return This pointer
		 */
		template<typename T0>
		TWeakPtr& operator=(const TWeakPtr<T0>& other);

		TWeakPtr& operator=(TWeakPtr&& other);

		/**
		 * @brief Move assign other weak pointer to this pointer
		 * 
		 * @tparam T0 Other pointer element type
		 * @param other Other weak pointer
		 * @return This pointer
		 */
		template<typename T0>
		TWeakPtr& operator=(TWeakPtr<T0>&& other);

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

		~TWeakPtr();
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
	}

	template<typename T>
	template<typename T0>
	inline TWeakPtr<T>::TWeakPtr(const TSharedPtr<T0>& ptr)
	{
		ConstructWeak(ptr);
		RefCountLogger.Debug("{{{}}} TWeakPtr has been constructed from a TSharedPtr.", (void*)m_Ptr);
	}

	template<typename T>
	inline TWeakPtr<T>::TWeakPtr(const TWeakPtr& other)
	{
		CopyConstructWeak(other);
		RefCountLogger.Debug("{{{}}} TWeakPtr has been copy constructed.", (void*)m_Ptr);
	}

	template<typename T>
	template<typename T0>
	inline TWeakPtr<T>::TWeakPtr(const TWeakPtr<T0>& other)
	{
		CopyConstructWeak(other);
		RefCountLogger.Debug("{{{}}} TWeakPtr has been copy constructed.", (void*)m_Ptr);
	}

	template<typename T>
	inline TWeakPtr<T>::TWeakPtr(TWeakPtr&& other)
	{
		MoveConstruct(Move(other));
		RefCountLogger.Debug("{{{}}} TWeakPtr has been move constructed.", (void*)m_Ptr);
	}

	template<typename T>
	template<typename T0>
	inline TWeakPtr<T>::TWeakPtr(TWeakPtr<T0>&& other)
	{
		MoveConstruct(Move(other));
		RefCountLogger.Debug("{{{}}} TWeakPtr has been move constructed.", (void*)m_Ptr);
	}

	template<typename T>
	inline TWeakPtr<T>& TWeakPtr<T>::operator=(const TWeakPtr& other)
	{
		TWeakPtr(other).Swap(*this);
		RefCountLogger.Debug("{{{}}} TWeakPtr has been copy assigned.", (void*)m_Ptr);
		return *this;
	}

	template<typename T>
	template<typename T0>
	inline TWeakPtr<T>& TWeakPtr<T>::operator=(const TWeakPtr<T0>& other)
	{
		TWeakPtr(other).Swap(*this);
		RefCountLogger.Debug("{{{}}} TWeakPtr has been copy assigned.", (void*)m_Ptr);
		return *this;
	}

	template<typename T>
	inline TWeakPtr<T>& TWeakPtr<T>::operator=(TWeakPtr&& other)
	{
		TWeakPtr(Move(other)).Swap(*this);
		RefCountLogger.Debug("{{{}}} TWeakPtr has been move assigned.", (void*)m_Ptr);
		return *this;
	}

	template<typename T>
	template<typename T0>
	inline TWeakPtr<T>& TWeakPtr<T>::operator=(TWeakPtr<T0>&& other)
	{
		TWeakPtr(Move(other)).Swap(*this);
		RefCountLogger.Debug("{{{}}} TWeakPtr has been move assigned.", (void*)m_Ptr);
		return *this;
	}

	template<typename T>
	inline TWeakPtr<T>& TWeakPtr<T>::operator=(nullptr_t)
	{
		DeleteWeak();
		RefCountLogger.Debug("{{{}}} TWeakPtr has been null assigned.", (void*)m_Ptr);
		return *this;
	}

	template<typename T>
	inline TSharedPtr<T> TWeakPtr<T>::Lock() const
	{
		if (IsExpired())
		{
			return TSharedPtr<T>();
		}

		TSharedPtr<T> shared;
		shared.ConstructSharedFromWeak(*this);
		RefCountLogger.Debug("{{{}}} TWeakPtr has been locked.", (void*)m_Ptr);
		return shared;
	}

	template<typename T>
	inline TWeakPtr<T>::~TWeakPtr()
	{
		DeleteWeak();
	}

	#pragma endregion

#pragma endregion

#pragma region TStaticPtrCast

	/**
	 * @brief Cast a shared pointer to another type.
	 * 
	 * @tparam T1 Type to cast to
	 * @tparam T2 Type to cast from
	 * @param other Other shared pointer
	 * @return A new, statically cast shared pointer
	 */
	template<typename T1, typename T2>
	inline TSharedPtr<T1> TStaticPtrCast(const TSharedPtr<T2>& other)
	{
		T2* rawPtr = other.GetRaw();
		TSharedPtr<T1> castPointer(other, static_cast<T1*>(rawPtr));
		RefCountLogger.Debug("{{{}}} TSharedPtr has been copy cast to another type.", (void*)rawPtr);
		return castPointer;
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
	inline TSharedPtr<T1> TStaticPtrCast(TSharedPtr<T2>&& other)
	{
		T2* rawPtr = other.GetRaw();
		TSharedPtr<T1> castPointer(Move(other), static_cast<T1*>(rawPtr));
		RefCountLogger.Debug("{{{}}} TSharedPtr has been move cast to another type.", (void*)rawPtr);
		return castPointer;
	}

#pragma endregion

#pragma endregion
}
