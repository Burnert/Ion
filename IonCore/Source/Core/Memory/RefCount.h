#pragma once

#include "MemoryCore.h"
#include "Core/Error/Error.h"

namespace Ion
{
	REGISTER_LOGGER(RefCountLogger, "Core::Memory::RefCount", ELoggerFlags::DisabledByDefault);

	/**
	 * @brief RefCounter Concurrency Mode - whether the ref counter should use
	 * thread-safe atomic operations or not.
	 */
	enum class ERCMode : uint8
	{
		ThreadSafe,
		NonThreadSafe,
	};

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

	template<typename T, ERCMode RC>
	class TPtrBase;

	template<typename T, ERCMode RC>
	class TSharedPtr;

	template<typename T, ERCMode RC>
	class TWeakPtr;

	template<typename T, ERCMode RC>
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

	template<typename T, ERCMode RC>
	struct TIsRefCountPtr<TPtrBase<T, RC>>
	{
		static constexpr inline bool Value = true;
		static constexpr inline bool value = true;
	};

	template<typename T, ERCMode RC>
	struct TIsRefCountPtr<TSharedPtr<T, RC>>
	{
		static constexpr inline bool Value = true;
		static constexpr inline bool value = true;
	};

	template<typename T, ERCMode RC>
	struct TIsRefCountPtr<TWeakPtr<T, RC>>
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

#pragma region TRefCountBase

	/**
	 * @brief Ref counting block base for shared pointers
	 */
	template<ERCMode RC>
	class TRefCountBase
	{
	public:
		using TRefCounter = TIf<RC == ERCMode::ThreadSafe, TAtomic<uint32>, uint32>;

		FORCEINLINE TRefCountBase() :
			m_Count(1),
			m_WeakCount(1)
		{
		}

		/**
		 * @brief Increments the strong reference count
		 */
		FORCEINLINE void IncRef()
		{
			if constexpr (RC == ERCMode::ThreadSafe)
			{
				// Cannot read the incremented value here, because it uses a different
				// instruction (lock xadd) instead of the desired (lock inc).
				++m_Count;

				// That is why m_Count.load(std::memory_order_relaxed) is used for the logging.
				RefCountLogger.Debug("TRefCountBase<ThreadSafe> {{{}}} Atomically (lock inc) incremented the ref count. Current ref count: {}", (void*)this, m_Count.load(std::memory_order_relaxed));
			}
			else
			{
				++m_Count;

				RefCountLogger.Debug("TRefCountBase {{{}}} Incremented the ref count by 1. Current ref count: {}", (void*)this, m_Count);
			}
		}

		FORCEINLINE bool IncRefNonZero()
		{
			// This function is only used in the TWeakPtr.Lock() method.
			if constexpr (RC == ERCMode::ThreadSafe)
			{
				uint32 originalCount = m_Count.load(std::memory_order_relaxed);

				while (1)
				{
					// Don't increment if the count is 0 at any time.
					// The pointer has been destroyed already.
					if (originalCount == 0)
						return false;

					// Increment the count if it hasn't changed, or try again otherwise.
					if (m_Count.compare_exchange_weak(originalCount, originalCount + 1, std::memory_order_relaxed))
					{
						RefCountLogger.Debug("TRefCountBase<ThreadSafe> {{{}}} Atomically (lock cmpxchg) incremented the ref count. Current ref count: {}", (void*)this, originalCount + 1);

						return true;
					}
				}
			}
			else
			{
				if (m_Count == 0)
					return false;

				++m_Count;

				RefCountLogger.Debug("TRefCountBase {{{}}} Incremented the ref count by 1. Current ref count: {}", (void*)this, m_Count);

				return true;
			}
		}

		/**
		 * @brief Decrements the strong reference count
		 *
		 * @details Deletes the owned pointer if the reference count is 0
		 * and decrements the weak reference count.
		 */
		FORCEINLINE void DecRef()
		{
			if constexpr (RC == ERCMode::ThreadSafe)
			{
				uint32 originalCount = m_Count.fetch_sub(1, std::memory_order_release);
				ionassert(originalCount > 0);

				RefCountLogger.Debug("TRefCountBase<ThreadSafe> {{{}}} Atomically (lock xadd) decremented the ref count. Current ref count: {}", (void*)this, originalCount - 1);

				if (originalCount == 1)
				{
					std::atomic_thread_fence(std::memory_order_acquire);

					RefCountLogger.Trace("TRefCountBase<ThreadSafe> {{{}}} Strong ref count reached 0.", (void*)this);

					Destroy();
					DecWeak();
				}
			}
			else
			{
				ionassert(m_Count > 0);
				--m_Count;

				RefCountLogger.Debug("TRefCountBase {{{}}} Decremented the ref count by 1. Current ref count: {}", (void*)this, m_Count);

				if (m_Count == 0)
				{
					RefCountLogger.Trace("TRefCountBase {{{}}} Strong ref count reached 0.", (void*)this);

					Destroy();
					DecWeak();
				}
			}
		}

		/**
		 * @brief Increments the weak reference count
		 */
		FORCEINLINE void IncWeak()
		{
			if constexpr (RC == ERCMode::ThreadSafe)
			{
				++m_WeakCount;

				RefCountLogger.Debug("TRefCountBase<ThreadSafe> {{{}}} Atomically (lock inc) incremented the weak ref count. Current weak ref count: {}", (void*)this, m_Count.load(std::memory_order_relaxed));
			}
			else
			{
				++m_WeakCount;

				RefCountLogger.Debug("TRefCountBase {{{}}} Incremented the weak ref count by 1. Current weak ref count: {}", (void*)this, m_WeakCount);
			}
		}

		/**
		 * @brief Decrements the weak reference count
		 *
		 * @details Destroys this control block if the reference count is 0.
		 */
		FORCEINLINE void DecWeak()
		{
			if constexpr (RC == ERCMode::ThreadSafe)
			{
				uint32 originalCount = m_WeakCount.fetch_sub(1, std::memory_order_release);
				ionassert(originalCount > 0);

				RefCountLogger.Debug("TRefCountBase<ThreadSafe> {{{}}} Decremented the weak ref count by 1. Current weak ref count: {}", (void*)this, originalCount - 1);

				if (originalCount == 1)
				{
					std::atomic_thread_fence(std::memory_order_acquire);

					RefCountLogger.Trace("TRefCountBase<ThreadSafe> {{{}}} Weak ref count reached 0.", (void*)this);

					DestroySelf();
				}
			}
			else
			{
				ionassert(m_WeakCount > 0);
				--m_WeakCount;

				RefCountLogger.Debug("TRefCountBase {{{}}} Decremented the weak ref count by 1. Current weak ref count: {}", (void*)this, m_WeakCount);

				if (m_WeakCount == 0)
				{
					RefCountLogger.Trace("TRefCountBase {{{}}} Weak ref count reached 0.", (void*)this);

					DestroySelf();
				}
			}
		}

		FORCEINLINE uint32 RefCount() const
		{
			if constexpr (RC == ERCMode::ThreadSafe)
			{
				return m_Count.load(std::memory_order_relaxed);
			}
			else
			{
				return m_Count;
			}
		}

		FORCEINLINE uint32 WeakRefCount() const
		{
			if constexpr (RC == ERCMode::ThreadSafe)
			{
				return m_WeakCount.load(std::memory_order_relaxed);
			}
			else
			{
				return m_WeakCount;
			}
		}

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
		TRefCounter m_Count;
		TRefCounter m_WeakCount;

		template<typename T0, ERCMode RC0>
		friend class TPtrBase;
	};

#pragma endregion

#pragma region TPtrRefCountBlock

	template<typename T, ERCMode RC>
	class TPtrRefCountBlock : public TRefCountBase<RC>
	{
	public:
		static_assert(!TIsReferenceV<T>);

		/**
		 * @brief Construct a new RefCount object that owns the pointer
		 * 
		 * @param ptr Pointer to take the ownership of.
		 */
		FORCEINLINE TPtrRefCountBlock(T* ptr) :
			m_Ptr(ptr)
		{
			RefCountLogger.Trace("TPtrRefCountBlock {{{}}} has been constructed with a pointer to {} {{{}}}.", (void*)this, typeid(T).name(), (void*)ptr);
		}

	private:
		virtual void Destroy() noexcept override
		{
			if (m_Ptr)
			{
				RefCountLogger.Trace("TPtrRefCountBlock {{{}}} is deleting the owned object {} {{{}}}...", (void*)this, typeid(T).name(), (void*)m_Ptr);
				delete m_Ptr;
				RefCountLogger.Debug("TPtrRefCountBlock {{{}}} has deleted the owned object {} {{{}}}.", (void*)this, typeid(T).name(), (void*)m_Ptr);
			}
		}

		virtual void DestroySelf() noexcept override
		{
			void* ptr = this;
			RefCountLogger.Trace("TPtrRefCountBlock {{{}}} is deleting itself...", ptr);
			delete this;
			RefCountLogger.Debug("TPtrRefCountBlock {{{}}} has deleted itself.", ptr);
		}

	private:
		/**
		 * @brief Owned pointer
		 */
		T* m_Ptr;
	};

#pragma endregion

#pragma region TPtrDeleterRefCountBlock

	template<typename T, typename FDeleter, ERCMode RC>
	class TPtrDeleterRefCountBlock : public TRefCountBase<RC>
	{
	public:
		static_assert(!TIsReferenceV<T>);

		/**
		 * @brief Construct a ref count block with a custom deleter.
		 * 
		 * @param ptr Pointer to take the ownership of.
		 */
		FORCEINLINE TPtrDeleterRefCountBlock(T* ptr, FDeleter deleter) noexcept :
			m_Ptr(ptr),
			m_Deleter(deleter)
		{
			RefCountLogger.Trace("TPtrDeleterRefCountBlock {{{}}} has been constructed with a pointer to {} {{{}}} and a custom deleter.", (void*)this, typeid(T).name(), (void*)ptr);
		}

	private:
		virtual void Destroy() noexcept override
		{
			if (m_Ptr)
			{
				RefCountLogger.Trace("TPtrDeleterRefCountBlock {{{}}} is deleting the owned object {} {{{}}} using a custom deleter...", (void*)this, typeid(T).name(), (void*)m_Ptr);
				m_Deleter(m_Ptr);
				RefCountLogger.Debug("TPtrDeleterRefCountBlock {{{}}} has deleted the owned object {} {{{}}} using a custom deleter.", (void*)this, typeid(T).name(), (void*)m_Ptr);
			}
		}

		virtual void DestroySelf() noexcept override
		{
			void* ptr = this;
			RefCountLogger.Trace("TPtrDeleterRefCountBlock {{{}}} is deleting itself...", ptr);
			delete this;
			RefCountLogger.Debug("TPtrDeleterRefCountBlock {{{}}} has deleted itself.", ptr);
		}

	private:
		/**
		 * @brief Owned pointer
		 */
		T* m_Ptr;
		FDeleter m_Deleter;
	};

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

	template<typename T, ERCMode RC>
	class TObjectRefCountBlock : public TRefCountBase<RC>
	{
	public:
		static_assert(!TIsReferenceV<T>);

		/**
		 * @brief Create a ref counter that has the element constructed in place.
		 */
		template<typename... Args>
		FORCEINLINE TObjectRefCountBlock(Args&&... args)
		{
			new((void*)&m_Object.Object) T(Forward<Args>(args)...);
			RefCountLogger.Trace("TObjectRefCountBlock {{{}}} has been constructed in place with an object of type {}.", (void*)this, typeid(T).name());
		}

		FORCEINLINE ~TObjectRefCountBlock()
		{
		}

	private:
		virtual void Destroy() noexcept override
		{
			RefCountLogger.Trace("TObjectRefCountBlock {{{}}} is calling the object destructor...", (void*)this);
			m_Object.Object.~T();
			RefCountLogger.Debug("TObjectRefCountBlock {{{}}} has destroyed the object.", (void*)this);
		}

		virtual void DestroySelf() noexcept override
		{
			void* ptr = this;
			RefCountLogger.Trace("TObjectRefCountBlock {{{}}} is deleting itself...", ptr);
			delete this;
			RefCountLogger.Debug("TObjectRefCountBlock {{{}}} has deleted itself.", ptr);
		}

	private:
		union
		{
			/** Owned object */
			_Memory_Detail::TWrapper<T> m_Object;
		};

		template<typename T0, ERCMode RC0>
		friend class TPtrBase;
	};

#pragma endregion

#pragma region TObjectDestroyRefCountBlock

	template<typename T, typename FOnDestroy, ERCMode RC>
	class TObjectDestroyRefCountBlock : public TRefCountBase<RC>
	{
	public:
		static_assert(!TIsReferenceV<T>);

		/**
		 * @brief Create a ref counter that has the element constructed in place
		 * and has a destroy callback function.
		 */
		template<typename... Args>
		FORCEINLINE TObjectDestroyRefCountBlock(FOnDestroy onDestroy, Args&&... args) :
			m_OnDestroy(onDestroy)
		{
			new((void*)&m_Object.Object) T(Forward<Args>(args)...);
			RefCountLogger.Trace("TObjectDestroyRefCountBlock {{{}}} has been constructed in place with an object of type {} and a destroy callback.", (void*)this, typeid(T).name());
		}

		FORCEINLINE ~TObjectDestroyRefCountBlock()
		{
		}

	private:
		virtual void Destroy() noexcept override
		{
			static_assert(std::is_invocable_v<FOnDestroy, TRemoveRef<T>&>);

			RefCountLogger.Trace("TObjectDestroyRefCountBlock {{{}}} is calling the object destructor...", (void*)this);
			m_OnDestroy(m_Object.Object);
			m_Object.Object.~T();
			RefCountLogger.Debug("TObjectDestroyRefCountBlock {{{}}} has destroyed the object.", (void*)this);
		}

		virtual void DestroySelf() noexcept override
		{
			void* ptr = this;
			RefCountLogger.Trace("TObjectDestroyRefCountBlock {{{}}} is deleting itself...", ptr);
			delete this;
			RefCountLogger.Debug("TObjectDestroyRefCountBlock {{{}}} has deleted itself.", ptr);
		}

	private:
		union
		{
			/** Owned object */
			_Memory_Detail::TWrapper<T> m_Object;
		};
		FOnDestroy m_OnDestroy;

		template<typename T0, ERCMode RC0>
		friend class TPtrBase;
	};

#pragma endregion

#pragma region TPtrBase

	template<typename T, ERCMode RC>
	class TPtrBase
	{
	public:
		using TElement  = T;
		using TThis     = TPtrBase<T, RC>;
		static constexpr ERCMode Concurrency = RC;

		FORCEINLINE uint32 RefCount() const
		{
			if (!m_Rep)
				return 0;

			return m_Rep->RefCount();
		}

		FORCEINLINE uint32 WeakRefCount() const
		{
			if (!m_Rep)
				return 0;

			return m_Rep->WeakRefCount();
		}

	protected:
		/**
		 * @brief Construct a null pointer base
		 */
		FORCEINLINE TPtrBase() :
			TPtrBase(nullptr)
		{
		}

		/**
		 * @brief Construct a null pointer base
		 */
		FORCEINLINE TPtrBase(nullptr_t) :
			m_Ptr(nullptr),
			m_Rep(nullptr)
		{
		}

		/**
		 * @brief Constructs a ref count control block that owns the pointer.
		 * 
		 * @tparam T0 Element type
		 * @param ptr Pointer to take the ownership of.
		 */
		template<typename T0>
		FORCEINLINE void ConstructShared(T0* ptr)
		{
			ionassert(ptr);

			SetPtrRepEnableSFT(ptr, new TPtrRefCountBlock<T0, RC>(ptr));
		}

		/**
		 * @brief Constructs a ref count control block with an element of type T constructed in place.
		 */
		template<typename... Args>
		FORCEINLINE void ConstructSharedInPlace(Args&&... args)
		{
			TObjectRefCountBlock<T, RC>* block = new TObjectRefCountBlock<T, RC>(Forward<Args>(args)...);

			SetPtrRepEnableSFT(&block->m_Object.Object, block);
		}

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
		FORCEINLINE void ConstructSharedWithDeleter(T0* ptr, FDeleter deleter)
		{
			static_assert(std::is_nothrow_invocable_v<FDeleter, T0*>);
			ionassert(ptr);

			SetPtrRepEnableSFT(ptr, new TPtrDeleterRefCountBlock<T0, FDeleter, RC>(ptr, deleter));
		}

		/**
		 * @brief Constructs a ref count control block with an element of type T constructed in place
		 * and a custom destroy callback, that will be called right before the object is destroyed.
		 * 
		 * @tparam FOnDestroy Destroy callback type - void(T&)
		 * 
		 * @param onDestroy Destroy callback function, called before the object is destroyed.
		 */
		template<typename FOnDestroy, typename... Args>
		FORCEINLINE void ConstructSharedInPlaceWithDestroyCallback(FOnDestroy onDestroy, Args&&... args)
		{
			auto block = new TObjectDestroyRefCountBlock<T, FOnDestroy, RC>(onDestroy, Forward<Args>(args)...);

			SetPtrRepEnableSFT(&block->m_Object.Object, block);
		}

		/**
		 * @brief Makes this pointer a shared pointer that points
		 * to the same object as the weak pointer.
		 * 
		 * @tparam T0 Element type
		 * @param ptr Weak pointer
		 */
		template<typename T0>
		FORCEINLINE void ConstructSharedFromWeak(const TWeakPtr<T0, RC>& ptr)
		{
			static_assert(TIsBaseOfV<TElement, T0>);

			if (ptr.m_Rep && ptr.m_Rep->IncRefNonZero())
			{
				m_Ptr = ptr.m_Ptr;
				m_Rep = ptr.m_Rep;
			}
		}

		/**
		 * @brief Makes this pointer a weak pointer that points
		 * to the same object as the shared pointer.
		 * 
		 * @tparam T0 Element type
		 * @param ptr Shared pointer
		 */
		template<typename T0>
		FORCEINLINE void ConstructWeak(const TSharedPtr<T0, RC>& ptr)
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

		/**
		 * @brief Copies the other shared pointer to this pointer
		 * 
		 * @tparam T0 Element type
		 * @param other Other shared pointer
		 */
		template<typename T0>
		FORCEINLINE void CopyConstructShared(const TSharedPtr<T0, RC>& other)
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

		/**
		 * @brief Copies the other weak pointer to this pointer
		 * 
		 * @tparam T0 Element type
		 * @param other Other weak pointer
		 */
		template<typename T0>
		FORCEINLINE void CopyConstructWeak(const TWeakPtr<T0, RC>& other)
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

		/**
		 * @brief Moves the other pointer to this pointer
		 * 
		 * @tparam TPtr Pointer type
		 * @param other Other pointer
		 */
		template<typename TPtr>
		FORCEINLINE void MoveConstruct(TPtr&& other)
		{
			static_assert(TIsRefCountPtrV<TPtr>);
			static_assert(TIsBaseOfV<TElement, typename TPtr::TElement>);
			static_assert(RC == TPtr::Concurrency);

			m_Ptr = other.m_Ptr;
			m_Rep = other.m_Rep;

			other.m_Ptr = nullptr;
			other.m_Rep = nullptr;
		}

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
		FORCEINLINE void AliasConstructShared(const TSharedPtr<T0, RC>& other, TElement* ptr)
		{
			m_Ptr = ptr;
			m_Rep = other.m_Rep;

			if (m_Rep)
			{
				ionassert(m_Ptr);

				m_Rep->IncRef();
			}
		}

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
		FORCEINLINE void AliasConstructShared(TSharedPtr<T0, RC>&& other, TElement* ptr)
		{
			m_Ptr = ptr;
			m_Rep = other.m_Rep;

			other.m_Ptr = nullptr;
			other.m_Rep = nullptr;
		}

		template<typename T0>
		FORCEINLINE void SetPtrRepEnableSFT(T0* ptr, TRefCountBase<RC>* rep)
		{
			m_Ptr = ptr;
			m_Rep = rep;
			if constexpr (TCanEnableSFTV<T0>)
			{
				m_Ptr->m_WeakSFT = TSharedPtr<T0, RC>(*static_cast<TSharedPtr<T, RC>*>(this), ptr);
			}
		}

		FORCEINLINE void Swap(TPtrBase& other)
		{
			std::swap(m_Ptr, other.m_Ptr);
			std::swap(m_Rep, other.m_Rep);
		}

		/**
		 * @brief Delete the shared pointer
		 * 
		 * @details Decrements the strong reference count.
		 */
		FORCEINLINE void DeleteShared()
		{
			if (m_Rep)
			{
				m_Rep->DecRef();
			}

			m_Ptr = nullptr;
			m_Rep = nullptr;
		}

		/**
		 * @brief Delete the weak pointer
		 * 
		 * @details Decrements the weak reference count.
		 */
		FORCEINLINE void DeleteWeak()
		{
			if (m_Rep)
			{
				m_Rep->DecWeak();
			}

			m_Ptr = nullptr;
			m_Rep = nullptr;
		}

	private:
		T* m_Ptr;
		TRefCountBase<RC>* m_Rep;

		template<typename T0, ERCMode RC0>
		friend class TSharedPtr;

		template<typename T0, ERCMode RC0>
		friend class TWeakPtr;

		template<typename T0, ERCMode RC0>
		friend class TPtrBase;

		template<typename T0, ERCMode RC0, typename... Args>
		friend TSharedPtr<T0, RC0> MakeShared(Args&&... args);

		template<typename T, ERCMode RC0, typename FOnDestroy, typename... Args>
		friend TEnableIfT<std::is_invocable_v<FOnDestroy, TRemoveRef<T>&>, TSharedPtr<T, RC0>> MakeSharedDC(FOnDestroy onDestroy, Args&&... args);
	};

#pragma endregion

#pragma region TSharedPtr

	template<typename T, ERCMode RC = ERCMode::ThreadSafe>
	class TSharedPtr : public TPtrBase<T, RC>
	{
	public:
		using TBase = TPtrBase<T, RC>;

		/**
		 * @brief Construct a null shared pointer
		 */
		FORCEINLINE TSharedPtr() :
			TSharedPtr(nullptr)
		{
		}

		/**
		 * @brief Construct a null shared pointer
		 */
		FORCEINLINE TSharedPtr(nullptr_t) :
			TBase(nullptr)
		{
			RefCountLogger.Debug("TSharedPtr {{{}}} has been null constructed.", (void*)m_Rep);
		}

		/**
		 * @brief Construct a shared pointer that owns the ptr
		 * 
		 * @param ptr Pointer to take the ownership of.
		 */
		template<typename T0, TEnableIfT<TIsPtrCompatibleV<T0, T>>* = 0>
		FORCEINLINE explicit TSharedPtr(T0* ptr)
		{
			ConstructShared(ptr);
			RefCountLogger.Debug("TSharedPtr {{{}}} has been constructed with a pointer to {} {{{}}}.", (void*)m_Rep, typeid(T0).name(), (void*)ptr);
		}

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
		FORCEINLINE TSharedPtr(T0* ptr, FDeleter deleter)
		{
			ConstructSharedWithDeleter(ptr, deleter);
			RefCountLogger.Debug("TSharedPtr {{{}}} has been constructed with a pointer to {} {{{}}} and a custom deleter.", (void*)m_Rep, typeid(T0).name(), (void*)ptr);
		}

		/**
		 * @brief Make a copy of another shared pointer.
		 * 
		 * @param other Other shared pointer
		 */
		FORCEINLINE TSharedPtr(const TSharedPtr& other)
		{
			RefCountLogger.Debug("TSharedPtr {{{}}} has been copy constructed.", (void*)other.m_Rep);
			CopyConstructShared(other);
		}

		/**
		 * @brief Make a copy of another shared pointer.
		 * 
		 * @tparam T0 Other pointer element type
		 * @param other Other shared pointer
		 */
		template<typename T0, TEnableIfT<TIsPtrCompatibleV<T0, T>>* = 0>
		FORCEINLINE TSharedPtr(const TSharedPtr<T0>& other)
		{
			RefCountLogger.Debug("TSharedPtr {{{}}} has been copy constructed.", (void*)other.m_Rep);
			CopyConstructShared(other);
		}

		/**
		 * @brief Move another shared pointer.
		 * 
		 * @param other Other shared pointer
		 */
		FORCEINLINE TSharedPtr(TSharedPtr&& other) noexcept
		{
			RefCountLogger.Debug("TSharedPtr {{{}}} has been move constructed.", (void*)other.m_Rep);
			MoveConstruct(Move(other));
		}

		/**
		 * @brief Move another shared pointer.
		 * 
		 * @tparam T0 Other pointer element type
		 * @param other Other shared pointer
		 */
		template<typename T0, TEnableIfT<TIsPtrCompatibleV<T0, T>>* = 0>
		FORCEINLINE TSharedPtr(TSharedPtr<T0>&& other) noexcept
		{
			RefCountLogger.Debug("TSharedPtr {{{}}} has been move constructed.", (void*)other.m_Rep);
			MoveConstruct(Move(other));
		}

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
		FORCEINLINE TSharedPtr(const TSharedPtr<T0>& other, TElement* ptr)
		{
			RefCountLogger.Debug("TSharedPtr {{{}}} has been alias constructed with a pointer to {} {{{}}}.", (void*)other.m_Rep, typeid(TElement).name(), (void*)ptr);
			AliasConstructShared(other, ptr);
		}

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
		FORCEINLINE TSharedPtr(TSharedPtr<T0>&& other, TElement* ptr)
		{
			RefCountLogger.Debug("TSharedPtr {{{}}} has been alias move constructed with a pointer to {} {{{}}}.", (void*)other.m_Rep, typeid(TElement).name(), (void*)ptr);
			AliasConstructShared(Move(other), ptr);
		}

		FORCEINLINE ~TSharedPtr()
		{
			RefCountLogger.Debug("TSharedPtr {{{}}} has been destroyed.", (void*)m_Rep);
			DeleteShared();
		}

		/**
		 * @brief Copy assign other shared pointer to this pointer
		 * 
		 * @param other Other shared pointer
		 */
		FORCEINLINE TSharedPtr& operator=(const TSharedPtr& other)
		{
			RefCountLogger.Debug("TSharedPtr {{{}}} has been copy assigned to TSharedPtr {{{}}}.", (void*)other.m_Rep, (void*)m_Rep);
			TSharedPtr(other).Swap(*this);
			return *this;
		}

		/**
		 * @brief Copy assign other shared pointer to this pointer
		 * 
		 * @tparam T0 Other pointer element type
		 * @param other Other shared pointer
		 */
		template<typename T0, TEnableIfT<TIsPtrCompatibleV<T0, T>>* = 0>
		FORCEINLINE TSharedPtr& operator=(const TSharedPtr<T0>& other)
		{
			RefCountLogger.Debug("TSharedPtr {{{}}} has been copy assigned to TSharedPtr {{{}}}.", (void*)other.m_Rep, (void*)m_Rep);
			TSharedPtr(other).Swap(*this);
			return *this;
		}

		/**
		 * @brief Move assign other shared pointer to this pointer
		 * 
		 * @param other Other shared pointer
		 */
		FORCEINLINE TSharedPtr& operator=(TSharedPtr&& other)
		{
			RefCountLogger.Debug("TSharedPtr {{{}}} has been move assigned to TSharedPtr {{{}}}.", (void*)other.m_Rep, (void*)m_Rep);
			TSharedPtr(Move(other)).Swap(*this);
			return *this;
		}

		/**
		 * @brief Move assign other shared pointer to this pointer
		 * 
		 * @tparam T0 Other pointer element type
		 * @param other Other shared pointer
		 */
		template<typename T0, TEnableIfT<TIsPtrCompatibleV<T0, T>>* = 0>
		FORCEINLINE TSharedPtr& operator=(TSharedPtr<T0>&& other)
		{
			RefCountLogger.Debug("TSharedPtr {{{}}} has been move assigned to TSharedPtr {{{}}}.", (void*)other.m_Rep, (void*)m_Rep);
			TSharedPtr(Move(other)).Swap(*this);
			return *this;
		}

		/**
		 * @brief Make this a null pointer
		 */
		FORCEINLINE TSharedPtr& operator=(nullptr_t)
		{
			RefCountLogger.Debug("Null has been assigned to TSharedPtr {{{}}}.", (void*)m_Rep);
			DeleteShared();
			return *this;
		}

		/**
		 * @brief Check if two pointers point to the same object
		 *
		 * @param other Other shared pointer
		 */
		template<typename T0>
		FORCEINLINE bool operator==(const TSharedPtr<T0>& other)
		{
			return m_Ptr == other.m_Ptr;
		}

		/**
		 * @brief Check if two pointers don't point to the same object
		 *
		 * @param other Other shared pointer
		 */
		template<typename T0>
		FORCEINLINE bool operator!=(const TSharedPtr<T0>& other)
		{
			return m_Ptr != other.m_Ptr;
		}

		/**
		 * @brief Get the Raw pointer
		 * 
		 * @return T* Raw pointer
		 */
		FORCEINLINE T* Raw() const
		{
			return m_Ptr;
		}

		/**
		 * @brief Access the pointer
		 */
		FORCEINLINE T* operator->() const
		{
			ionassert(m_Ptr);
			return m_Ptr;
		}

		/**
		 * @brief Dereference the pointer
		 */
		FORCEINLINE T& operator*() const
		{
			ionassert(m_Ptr);
			return *m_Ptr;
		}

		/**
		 * @brief Check if this pointer is not null.
		 */
		FORCEINLINE bool IsValid() const
		{
			return m_Rep && m_Ptr;
		}

		/**
		 * @see IsValid()
		 */
		FORCEINLINE operator bool() const
		{
			return IsValid();
		}
	};

#pragma endregion

#pragma region TWeakPtr

	template<typename T, ERCMode RC = ERCMode::ThreadSafe>
	class TWeakPtr : public TPtrBase<T, RC>
	{
	public:
		using TBase = TPtrBase<T, RC>;

		/**
		 * @brief Construct a null weak pointer
		 */
		FORCEINLINE TWeakPtr() :
			TWeakPtr(nullptr)
		{
		}

		/**
		 * @brief Construct a null weak pointer
		 */
		FORCEINLINE TWeakPtr(nullptr_t) :
			TBase(nullptr)
		{
			RefCountLogger.Debug("TWeakPtr {{{}}} has been null constructed.", (void*)m_Ptr);
		}

		/**
		 * @brief Construct a Weak pointer based on a Shared pointer
		 * 
		 * @tparam T0 Shared pointer element type
		 * @param ptr Shared pointer
		 */
		template<typename T0, ERCMode RC0, TEnableIfT<TIsPtrCompatibleV<T0, T>>* = 0>
		FORCEINLINE TWeakPtr(const TSharedPtr<T0, RC0>& shared)
		{
			ConstructWeak(shared);
			RefCountLogger.Debug("TWeakPtr {{{}}} has been constructed from TSharedPtr {{{}}}.", (void*)m_Rep, (void*)shared.m_Rep);
		}

		/**
		 * @brief Make a copy of another weak pointer.
		 * 
		 * @param other Other weak pointer
		 */
		FORCEINLINE TWeakPtr(const TWeakPtr& other)
		{
			RefCountLogger.Debug("TWeakPtr {{{}}} has been copy constructed.", (void*)other.m_Rep);
			CopyConstructWeak(other);
		}

		/**
		 * @brief Make a copy of another weak pointer.
		 * 
		 * @tparam T0 Other pointer element type
		 * @param other Other weak pointer
		 */
		template<typename T0, ERCMode RC0, TEnableIfT<TIsPtrCompatibleV<T0, T>>* = 0>
		FORCEINLINE TWeakPtr(const TWeakPtr<T0, RC0>& other)
		{
			RefCountLogger.Debug("TWeakPtr {{{}}} has been copy constructed.", (void*)other.m_Rep);
			CopyConstructWeak(other);
		}

		/**
		 * @brief Move another weak pointer.
		 * 
		 * @param other Other weak pointer
		 */
		FORCEINLINE TWeakPtr(TWeakPtr&& other)
		{
			RefCountLogger.Debug("TWeakPtr {{{}}} has been move constructed.", (void*)other.m_Rep);
			MoveConstruct(Move(other));
		}

		/**
		 * @brief Move another weak pointer.
		 * 
		 * @tparam T0 Other pointer element type
		 * @param other Other weak pointer
		 */
		template<typename T0, ERCMode RC0, TEnableIfT<TIsPtrCompatibleV<T0, T>>* = 0>
		FORCEINLINE TWeakPtr(TWeakPtr<T0, RC0>&& other)
		{
			RefCountLogger.Debug("TWeakPtr {{{}}} has been move constructed.", (void*)other.m_Rep);
			MoveConstruct(Move(other));
		}

		FORCEINLINE ~TWeakPtr()
		{
			RefCountLogger.Debug("TWeakPtr {{{}}} has been destroyed.", (void*)m_Rep);
			DeleteWeak();
		}

		/**
		 * @brief Copy assign other weak pointer to this pointer
		 * 
		 * @param other Other weak pointer
		 */
		FORCEINLINE TWeakPtr& operator=(const TWeakPtr& other)
		{
			RefCountLogger.Debug("TWeakPtr {{{}}} has been copy assigned to TWeakPtr {{{}}}.", (void*)other.m_Rep, (void*)m_Rep);
			TWeakPtr(other).Swap(*this);
			return *this;
		}

		/**
		 * @brief Copy assign other weak pointer to this pointer
		 * 
		 * @tparam T0 Other pointer element type
		 * @param other Other weak pointer
		 */
		template<typename T0, ERCMode RC0, TEnableIfT<TIsPtrCompatibleV<T0, T>>* = 0>
		FORCEINLINE TWeakPtr& operator=(const TWeakPtr<T0, RC0>& other)
		{
			RefCountLogger.Debug("TWeakPtr {{{}}} has been copy assigned to TWeakPtr {{{}}}.", (void*)other.m_Rep, (void*)m_Rep);
			TWeakPtr(other).Swap(*this);
			return *this;
		}

		/**
		 * @brief Move assign other weak pointer to this pointer
		 * 
		 * @param other Other weak pointer
		 */
		FORCEINLINE TWeakPtr& operator=(TWeakPtr&& other) noexcept
		{
			RefCountLogger.Debug("TWeakPtr {{{}}} has been move assigned to TWeakPtr {{{}}}.", (void*)other.m_Rep, (void*)m_Rep);
			TWeakPtr(Move(other)).Swap(*this);
			return *this;
		}

		/**
		 * @brief Move assign other weak pointer to this pointer
		 * 
		 * @tparam T0 Other pointer element type
		 * @param other Other weak pointer
		 */
		template<typename T0, ERCMode RC0, TEnableIfT<TIsPtrCompatibleV<T0, T>>* = 0>
		FORCEINLINE TWeakPtr& operator=(TWeakPtr<T0, RC0>&& other) noexcept
		{
			RefCountLogger.Debug("TWeakPtr {{{}}} has been move assigned to TWeakPtr {{{}}}.", (void*)other.m_Rep, (void*)m_Rep);
			TWeakPtr(Move(other)).Swap(*this);
			return *this;
		}

		/**
		 * @brief Assign a shared pointer to this weak pointer.
		 *
		 * @tparam T0 Shared pointer element type
		 * @param shared Shared pointer
		 */
		template<typename T0, ERCMode RC0, TEnableIfT<TIsPtrCompatibleV<T0, T>>* = 0>
		FORCEINLINE TWeakPtr& operator=(const TSharedPtr<T0, RC0>& shared)
		{
			RefCountLogger.Debug("TSharedPtr {{{}}} has been assigned to TWeakPtr {{{}}}.", (void*)shared.m_Rep, (void*)m_Rep);
			TWeakPtr(shared).Swap(*this);
			return *this;
		}

		/**
		 * @brief Make this a null weak pointer
		 * 
		 * @return This pointer
		 */
		FORCEINLINE TWeakPtr& operator=(nullptr_t)
		{
			RefCountLogger.Debug("Null has been assigned to TWeakPtr {{{}}}.", (void*)m_Rep);
			DeleteWeak();
			return *this;
		}

		/**
		 * @brief Make a shared pointer out of this weak pointer
		 * 
		 * @return If the weak pointer has not expired, a shared pointer
		 * that points to the same object, else a null shared pointer.
		 */
		FORCEINLINE TSharedPtr<T, RC> Lock() const
		{
			if (IsExpired())
			{
				return TSharedPtr<T>();
			}

			RefCountLogger.Debug("TWeakPtr {{{}}} has been locked.", (void*)m_Rep);
			TSharedPtr<T, RC> shared;
			shared.ConstructSharedFromWeak(*this);
			RefCountLogger.Debug("TSharedPtr {{{}}} has been constructed from TWeakPtr {{{}}}.", (void*)shared.m_Rep, (void*)m_Rep);
			return shared;
		}

		/**
		 * @brief Get the Raw pointer. Make sure to call IsExpired() 
		 * to check if the pointer is valid before dereferencing it.
		 *
		 * @return T* Raw pointer
		 */
		FORCEINLINE T* Raw() const noexcept
		{
			return m_Ptr;
		}

		/**
		 * @brief Checks if the weak pointer no longer points to a valid object
		 * It will return false if the pointer is null.
		 */
		FORCEINLINE bool IsExpired() const
		{
			return m_Rep && RefCount() == 0;
		}

		/**
		 * @brief Check if this pointer is not null and not expired.
		 */
		FORCEINLINE bool IsValid() const
		{
			return m_Rep && m_Ptr && (RefCount() > 0);
		}

		/**
		 * @see IsValid()
		 */
		FORCEINLINE operator bool() const
		{
			return IsValid();
		}
	};

#pragma endregion

#pragma region Enable Shared From This

	template<typename T, ERCMode RC = ERCMode::ThreadSafe>
	class TEnableSFT
	{
	public:
		using TEnableSFTType = TEnableSFT;

		FORCEINLINE TSharedPtr<T, RC> SharedFromThis()
		{
			return m_WeakSFT.Lock();
		}

		FORCEINLINE TSharedPtr<const T, RC> SharedFromThis() const
		{
			return m_WeakSFT.Lock();
		}

		FORCEINLINE TWeakPtr<T, RC> WeakFromThis() noexcept
		{
			return m_WeakSFT;
		}

		FORCEINLINE TWeakPtr<const T, RC> WeakFromThis() const noexcept
		{
			return m_WeakSFT;
		}

	protected:
		FORCEINLINE TEnableSFT() noexcept :
			m_WeakSFT()
		{
		}

		FORCEINLINE TEnableSFT(const TEnableSFT&) noexcept :
			m_WeakSFT()
		{
		}

		FORCEINLINE ~TEnableSFT() = default;

		FORCEINLINE TEnableSFT& operator=(const TEnableSFT&) noexcept
		{
			return *this;
		}

	private:
		mutable TWeakPtr<T, RC> m_WeakSFT;

		template<typename T0, ERCMode RC0>
		friend class TPtrBase;
	};

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
	template<typename T1, typename T2, ERCMode RC>
	FORCEINLINE TSharedPtr<T1, RC> PtrCast(const TSharedPtr<T2, RC>& other)
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
	template<typename T1, typename T2, ERCMode RC>
	FORCEINLINE TSharedPtr<T1, RC> PtrCast(TSharedPtr<T2, RC>&& other)
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
	template<typename T1, typename T2, ERCMode RC>
	FORCEINLINE TSharedPtr<T1, RC> DynamicPtrCast(const TSharedPtr<T2, RC>& other)
	{
		if (!dynamic_cast<T1*>(other.Raw()))
			return TSharedPtr<T1, RC>();

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
	template<typename T1, typename T2, ERCMode RC>
	FORCEINLINE TSharedPtr<T1, RC> DynamicPtrCast(TSharedPtr<T2, RC>&& other)
	{
		if (!dynamic_cast<T1*>(other.Raw()))
			return TSharedPtr<T1, RC>();

		return PtrCast<T1>(Move(other));
	}

#pragma endregion

#pragma region MakeShared / MakeSharedDC

	#define FRIEND_MAKE_SHARED \
	template<typename _T, ERCMode _RC> \
	friend class TObjectRefCountBlock; \
	template<typename _T, typename _F, ERCMode _RC> \
	friend class TObjectDestroyRefCountBlock

	/**
	 * @brief Make a shared pointer with an element constructed in-place.
	 */
	template<typename T, ERCMode RC = ERCMode::ThreadSafe, typename... Args>
	FORCEINLINE TSharedPtr<T, RC> MakeShared(Args&&... args)
	{
		TSharedPtr<T, RC> ptr;
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
	template<typename T, ERCMode RC = ERCMode::ThreadSafe, typename FOnDestroy, typename... Args>
	FORCEINLINE TEnableIfT<std::is_invocable_v<FOnDestroy, TRemoveRef<T>&>, TSharedPtr<T, RC>> MakeSharedDC(FOnDestroy onDestroy, Args&&... args)
	{
		TSharedPtr<T, RC> ptr;
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
	template<typename T, ERCMode RC = ERCMode::ThreadSafe>
	FORCEINLINE TSharedPtr<T, RC> MakeSharedFrom(T* ptr)
	{
		return TSharedPtr<T, RC>(ptr);
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
	template<typename T, ERCMode RC = ERCMode::ThreadSafe, typename FDeleter, TEnableIfT<std::is_nothrow_invocable_v<FDeleter, T*>>* = 0>
	FORCEINLINE TSharedPtr<T, RC> MakeSharedFrom(T* ptr, FDeleter deleter)
	{
		return TSharedPtr<T, RC>(ptr, deleter);
	}

#pragma endregion

	int RefCountPtrTest();

#pragma endregion
}
