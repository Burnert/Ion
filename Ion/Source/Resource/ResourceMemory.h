#pragma once

#include "Core/Core.h"
#include "ResourceCommon.h"
#include "ResourceFwd.h"

#define ENABLE_LOG_RESOURCE_MEMORY 0

#if ENABLE_LOG_RESOURCE_MEMORY
#define _DEBUG_LOG(...) LOG_DEBUG(__VA_ARGS__);
#else
#define _DEBUG_LOG(...)
#endif

namespace Ion
{
	template<typename T>
	class TResourcePtrBase;

	template<typename T>
	class TResourcePtr;

	template<typename T>
	class TResourceWeakPtr;

	/**
	 * @brief Checks if T is a TResourcePtr type
	 * 
	 * @tparam T Type to check
	 */
	template<typename T>
	struct TIsResourcePtr
	{
		static constexpr inline bool Value = false;
	};

	template<typename T>
	struct TIsResourcePtr<TResourcePtrBase<T>>
	{
		static constexpr inline bool Value = true;
	};

	template<typename T>
	struct TIsResourcePtr<TResourcePtr<T>>
	{
		static constexpr inline bool Value = true;
	};

	template<typename T>
	struct TIsResourcePtr<TResourceWeakPtr<T>>
	{
		static constexpr inline bool Value = true;
	};

	/**
	 * @brief Checks if T is a TResourcePtr type
	 * 
	 * @tparam T Type to check
	 */
	template<typename T>
	static constexpr inline bool TIsResourcePtrV = TIsResourcePtr<T>::Value;

	/**
	 * @brief Used to call ResourceManager::Unregister
	 * without including the header.
	 */
	struct ResourceHelper
	{
		static void UnregisterResource(Resource* ptr);
	};

	/**
	 * @brief Ref counting control block for resource pointers
	 * 
	 * @tparam T Type of the element that the ref counter owns.
	 */
	template<typename T>
	class TResourceRefCount
	{
		static_assert(TIsBaseOfV<Resource, T>);

		/**
		 * @brief Construct a new RefCount object that owns the pointer
		 * 
		 * @param ptr Pointer to take the ownership of.
		 */
		TResourceRefCount(T* ptr) :
			m_Ptr(ptr),
			m_RefCount(1),
			m_WeakCount(1)
		{
		}

		/**
		 * @brief Increments the strong reference count
		 * 
		 * @return uint32 Reference count after incrementing
		 */
		uint32 IncRef()
		{
			_DEBUG_LOG("{0} IncRef: {1}", (void*)m_Ptr, m_RefCount + 1)
			return ++m_RefCount;
		}

		/**
		 * @brief Decrements the strong reference count
		 * 
		 * @details Deletes the owned pointer if the reference count is 0
		 * and decrements the weak reference count.
		 * 
		 * @return uint32 Reference count after decrementing
		 */
		uint32 DecRef()
		{
			uint32 count = --m_RefCount;
			_DEBUG_LOG("{0} DecRef: {1}", (void*)m_Ptr, count)
			if (count == 0)
			{
				Delete();
				DecWeak();
			}
			return count;
		}

		/**
		 * @brief Increments the weak reference count
		 * 
		 * @return uint32 Weak reference count after incrementing
		 */
		uint32 IncWeak()
		{
			_DEBUG_LOG("{0} IncWeak: {1}", (void*)m_Ptr, m_WeakCount + 1)
			return ++m_WeakCount;
		}

		/**
		 * @brief Decrements the weak reference count
		 * 
		 * @details Destroys this control block if the reference count is 0.
		 * 
		 * @return uint32 Weak reference count after decrementing
		 */
		uint32 DecWeak()
		{
			uint32 count = --m_WeakCount;
			_DEBUG_LOG("{0} DecWeak: {1}", (void*)m_Ptr, count)
			if (count == 0)
			{
				DeleteThis();
			}
			return count;
		}

	private:
		/**
		 * @brief Deletes the owned pointer and unregisters the Resource.
		 */
		void Delete()
		{
			_DEBUG_LOG("TResourceRefCount::Delete")

			if (m_Ptr)
			{
				_DEBUG_LOG("TResourceRefCount::Delete - Unregister resource")

				ResourceHelper::UnregisterResource(m_Ptr);
				delete m_Ptr;
			}
		}

		/**
		 * @brief Destroys this control block.
		 */
		void DeleteThis()
		{
			_DEBUG_LOG("TResourceRefCount::DeleteThis")

			delete this;
		}

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
		 * You can say the ref counter holds a weak reference.
		 */
		uint32 m_WeakCount;

		template<typename T0>
		friend class TResourcePtrBase;

		friend class ResourceMemory;
	};

	template<typename T>
	class TResourcePtrBase
	{
	public:
		static_assert(TIsBaseOfV<Resource, T>);
		using TElement  = T;
		using TThis     = TResourcePtrBase<T>;
		using TRefCount = TResourceRefCount<T>;

		/**
		 * @brief Checks if the weak pointer no longer points to a valid resource
		 */
		bool IsExpired() const
		{
			ionassert(m_RefCount);
			return m_RefCount->m_RefCount == 0;
		}

		/**
		 * @brief Checks if the pointer is not null
		 */
		operator bool() const
		{
			return m_Ptr && m_RefCount;
		}

	protected:
		/**
		 * @brief Construct a null resource pointer base
		 */
		TResourcePtrBase() :
			TResourcePtrBase(nullptr)
		{
		}

		/**
		 * @brief Construct a null resource pointer base
		 */
		TResourcePtrBase(nullptr_t) :
			m_Ptr(nullptr),
			m_RefCount(nullptr)
		{
		}

		/**
		 * @brief Makes this pointer own the ptr and constructs a ref count control block.
		 * 
		 * @param ptr Pointer to take the ownership of.
		 */
		void ConstructShared(T* ptr)
		{
			ionassert(ptr);

			m_Ptr = ptr;
			m_RefCount = new TRefCount(ptr);
		}

		/**
		 * @brief Makes this pointer a shared pointer that points
		 * to the same resource as the weak pointer.
		 * 
		 * @tparam T0 Element type
		 * @param ptr Weak pointer
		 */
		template<typename T0>
		void ConstructSharedFromWeak(const TResourceWeakPtr<T0>& ptr)
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

		/**
		 * @brief Makes this pointer a weak pointer that points
		 * to the same resource as the shared pointer.
		 * 
		 * @tparam T0 Element type
		 * @param ptr Shared pointer
		 */
		template<typename T0>
		void ConstructWeak(const TResourcePtr<T0>& ptr)
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

		/**
		 * @brief Copies the other shared pointer to this pointer
		 * 
		 * @tparam T0 Element type
		 * @param other Other shared pointer
		 */
		template<typename T0>
		void CopyConstructShared(const TResourcePtr<T0>& other)
		{
			static_assert(TIsBaseOfV<TElement, T0>);

			m_Ptr = (TElement*)other.m_Ptr;
			m_RefCount = (TRefCount*)other.m_RefCount;

			if (other.m_RefCount)
			{
				ionassert(other.m_Ptr);

				m_RefCount->IncRef();
			}
		}

		/**
		 * @brief Copies the other weak pointer to this pointer
		 * 
		 * @tparam T0 Element type
		 * @param other Other weak pointer
		 */
		template<typename T0>
		void CopyConstructWeak(const TResourceWeakPtr<T0>& other)
		{
			static_assert(TIsBaseOfV<TElement, T0>);

			m_Ptr = (TElement*)other.m_Ptr;
			m_RefCount = (TRefCount*)other.m_RefCount;

			if (other.m_RefCount)
			{
				ionassert(other.m_Ptr);

				m_RefCount->IncWeak();
			}
		}

		/**
		 * @brief Moves the other pointer to this pointer
		 * 
		 * @tparam TPtr Pointer type
		 * @param other Other pointer
		 */
		template<typename TPtr>
		void MoveConstruct(TPtr&& other)
		{
			static_assert(TIsResourcePtrV<TPtr>);
			static_assert(TIsBaseOfV<TElement, typename TPtr::TElement>);

			m_Ptr = (TElement*)other.m_Ptr;
			m_RefCount = (TRefCount*)other.m_RefCount;

			other.m_Ptr = nullptr;
			other.m_RefCount = nullptr;
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
		void AliasConstructShared(const TResourcePtr<T0>& other, TElement* ptr)
		{
			static_assert(TOrV<TIsBaseOf<TElement, T0>, TIsBaseOf<T0, TElement>>);

			m_Ptr = ptr;
			m_RefCount = (TRefCount*)other.m_RefCount;

			if (other.m_RefCount)
			{
				ionassert(other.m_Ptr);

				m_RefCount->IncRef();
			}
		}

		void Swap(TResourcePtrBase& other)
		{
			std::swap(m_Ptr, other.m_Ptr);
			std::swap(m_RefCount, other.m_RefCount);
		}

		/**
		 * @brief Delete the shared pointer
		 * 
		 * @details Decrements the strong reference count.
		 */
		void DeleteShared()
		{
			if (m_RefCount)
			{
				m_RefCount->DecRef();
			}

			m_Ptr = nullptr;
			m_RefCount = nullptr;
		}

		/**
		 * @brief Delete the weak pointer
		 * 
		 * @details Decrements the weak reference count.
		 */
		void DeleteWeak()
		{
			if (m_RefCount)
			{
				m_RefCount->DecWeak();
			}

			m_Ptr = nullptr;
			m_RefCount = nullptr;
		}

	private:
		T* m_Ptr;
		TRefCount* m_RefCount;

		template<typename T0>
		friend class TResourcePtr;

		template<typename T0>
		friend class TResourceWeakPtr;

		template<typename T0>
		friend class TResourcePtrBase;

		friend class ResourceMemory;
	};

	template<typename T>
	class TResourcePtr : public TResourcePtrBase<T>
	{
	public:
		using TBase     = TResourcePtrBase<T>;
		using TRefCount = typename TBase::TRefCount;

		/**
		 * @brief Construct a null shared resource pointer
		 */
		TResourcePtr() :
			TResourcePtr(nullptr)
		{
		}

		/**
		 * @brief Construct a null shared resource pointer
		 */
		TResourcePtr(nullptr_t) :
			TBase(nullptr)
		{
		}

		/**
		 * @brief Construct a shared resource pointer that owns the ptr
		 * 
		 * @param ptr Pointer to take the ownership of.
		 */
		explicit TResourcePtr(T* ptr)
		{
			_DEBUG_LOG("TResourcePtr {0} Ptr constructor", (void*)ptr)
			ConstructShared(ptr);
		}

		TResourcePtr(const TResourcePtr& other)
		{
			CopyConstructShared(other);
		}

		/**
		 * @brief Make a copy of another shared resource pointer.
		 * 
		 * @tparam T0 Other pointer element type
		 * @param other Other shared pointer
		 */
		template<typename T0>
		TResourcePtr(const TResourcePtr<T0>& other)
		{
			_DEBUG_LOG("TResourcePtr {0} Copy constructor", (void*)other.m_Ptr)
			CopyConstructShared(other);
		}

		TResourcePtr(TResourcePtr&& other) noexcept
		{
			MoveConstruct(Move(other));
		}

		/**
		 * @brief Move another shared resource pointer.
		 * 
		 * @tparam T0 Other pointer element type
		 * @param other Other shared pointer
		 */
		template<typename T0>
		TResourcePtr(TResourcePtr<T0>&& other) noexcept
		{
			_DEBUG_LOG("TResourcePtr {0} Move constructor", (void*)other.m_Ptr)
			MoveConstruct(Move(other));
		}

		/**
		 * @brief Create an alias pointer of another shared resource pointer.
		 * 
		 * @details Points to the ptr, while using the same control block
		 * as the other shared pointer. Mainly used for pointer type casting.
		 * 
		 * @tparam T0 Other pointer element type
		 * @param other Other shared pointer
		 * @param ptr Raw pointer to point to
		 */
		template<typename T0>
		TResourcePtr(const TResourcePtr<T0>& other, TElement* ptr)
		{
			_DEBUG_LOG("TResourcePtr {0} Alias constructor", (void*)other.m_Ptr)
			AliasConstructShared(other, ptr);
		}

		TResourcePtr& operator=(const TResourcePtr& other)
		{
			TResourcePtr(other).Swap(*this);
			return *this;
		}

		/**
		 * @brief Copy assign other shared pointer to this pointer
		 * 
		 * @tparam T0 Other pointer element type
		 * @param other Other shared pointer
		 * @return This pointer
		 */
		template<typename T0>
		TResourcePtr& operator=(const TResourcePtr<T0>& other)
		{
			_DEBUG_LOG("TResourcePtr {0} Copy assignment operator", (void*)other.m_Ptr)
			TResourcePtr(other).Swap(*this);
			return *this;
		}

		TResourcePtr& operator=(TResourcePtr&& other)
		{
			TResourcePtr(Move(other)).Swap(*this);
			return *this;
		}

		/**
		 * @brief Move assign other shared pointer to this pointer
		 * 
		 * @tparam T0 Other pointer element type
		 * @param other Other shared pointer
		 * @return This pointer
		 */
		template<typename T0>
		TResourcePtr& operator=(TResourcePtr<T0>&& other)
		{
			_DEBUG_LOG("TResourcePtr {0} Move assignment operator", (void*)other.m_Ptr)
			TResourcePtr(Move(other)).Swap(*this);
			return *this;
		}

		/**
		 * @brief Make this a null resource pointer
		 * 
		 * @return This pointer
		 */
		TResourcePtr& operator=(nullptr_t)
		{
			_DEBUG_LOG("TResourcePtr Null assignment operator")
			DeleteShared();
			return *this;
		}

		/**
		 * @brief Get the Raw resource pointer
		 * 
		 * @return Raw Resource pointer
		 */
		T* GetRaw() const
		{
			return m_Ptr;
		}

		/**
		 * @brief Access the pointer
		 */
		T* operator->() const
		{
			return GetRaw();
		}

		/**
		 * @brief Dereference the pointer
		 */
		T& operator*() const
		{
			ionassert(m_Ptr);
			return *GetRaw();
		}

		~TResourcePtr()
		{
			DeleteShared();
		}
	};

	template<typename T>
	class TResourceWeakPtr : public TResourcePtrBase<T>
	{
	public:
		using TBase = TResourcePtrBase<T>;
		using TRefCount = typename TBase::TRefCount;

		/**
		 * @brief Construct a null weak resource pointer
		 */
		TResourceWeakPtr() :
			TResourceWeakPtr(nullptr)
		{
		}

		/**
		 * @brief Construct a null weak resource pointer
		 */
		TResourceWeakPtr(nullptr_t) :
			TBase(nullptr)
		{
		}

		/**
		 * @brief Construct a Weak pointer based on the other Shared pointer
		 * 
		 * @tparam T0 Other shared pointer element type
		 * @param ptr Other shared pointer
		 */
		template<typename T0>
		TResourceWeakPtr(const TResourcePtr<T0>& ptr)
		{
			_DEBUG_LOG("TResourceWeakPtr {0} TResourcePtr constructor", (void*)ptr.GetRaw())
			ConstructWeak(ptr);
		}

		TResourceWeakPtr(const TResourceWeakPtr& other)
		{
			CopyConstructWeak(other);
		}

		/**
		 * @brief Make a copy of another weak resource pointer.
		 * 
		 * @tparam T0 Other pointer element type
		 * @param other Other weak pointer
		 */
		template<typename T0>
		TResourceWeakPtr(const TResourceWeakPtr<T0>& other)
		{
			_DEBUG_LOG("TResourceWeakPtr {0} Copy constructor", (void*)other.m_Ptr)
			CopyConstructWeak(other);
		}

		TResourceWeakPtr(TResourceWeakPtr&& other)
		{
			MoveConstruct(Move(other));
		}

		/**
		 * @brief Move another weak resource pointer.
		 * 
		 * @tparam T0 Other pointer element type
		 * @param other Other weak pointer
		 */
		template<typename T0>
		TResourceWeakPtr(TResourceWeakPtr<T0>&& other)
		{
			_DEBUG_LOG("TResourceWeakPtr {0} Move constructor", (void*)other.m_Ptr)
			MoveConstruct(Move(other));
		}

		TResourceWeakPtr& operator=(const TResourceWeakPtr& other)
		{
			TResourceWeakPtr(other).Swap(*this);
			return *this;
		}

		/**
		 * @brief Copy assign other weak pointer to this pointer
		 * 
		 * @tparam T0 Other pointer element type
		 * @param other Other weak pointer
		 * @return This pointer
		 */
		template<typename T0>
		TResourceWeakPtr& operator=(const TResourceWeakPtr<T0>& other)
		{
			_DEBUG_LOG("TResourceWeakPtr {0} Copy assignment operator", (void*)other.m_Ptr)
			TResourceWeakPtr(other).Swap(*this);
			return *this;
		}

		TResourceWeakPtr& operator=(TResourceWeakPtr&& other)
		{
			TResourceWeakPtr(Move(other)).Swap(*this);
			return *this;
		}

		/**
		 * @brief Move assign other weak pointer to this pointer
		 * 
		 * @tparam T0 Other pointer element type
		 * @param other Other weak pointer
		 * @return This pointer
		 */
		template<typename T0>
		TResourceWeakPtr& operator=(TResourceWeakPtr<T0>&& other)
		{
			_DEBUG_LOG("TResourceWeakPtr {0} Move assignment operator", (void*)other.m_Ptr)
			TResourceWeakPtr(Move(other)).Swap(*this);
			return *this;
		}

		/**
		 * @brief Make this a null weak pointer
		 * 
		 * @return This pointer
		 */
		TResourceWeakPtr& operator=(nullptr_t)
		{
			_DEBUG_LOG("TResourceWeakPtr Null assignment operator")
			DeleteWeak();
			return *this;
		}

		/**
		 * @brief Make a shared pointer out of this weak pointer
		 * 
		 * @return If the weak pointer has not expired, a shared pointer
		 * that points to the same resource, else, a null shared pointer.
		 */
		TResourcePtr<T> Lock() const
		{
			if (IsExpired())
			{
				return TResourcePtr<T>();
			}

			TResourcePtr<T> shared;
			shared.ConstructSharedFromWeak(*this);
			return shared;
		}

		~TResourceWeakPtr()
		{
			DeleteWeak();
		}
	};

	/**
	 * @brief Cast a shared pointer to another type.
	 * 
	 * @tparam T1 Type to cast to
	 * @tparam T2 Type to cast from
	 * @param other Other shared pointer
	 * @return A new, statically cast shared pointer
	 */
	template<typename T1, typename T2>
	inline TResourcePtr<T1> TStaticResourcePtrCast(const TResourcePtr<T2>& other)
	{
		T2* rawPtr = other.GetRaw();
		TResourcePtr<T1> castPointer(other, static_cast<T1*>(rawPtr));
		_DEBUG_LOG("{0} TStaticResourcePtrCast (copy)", (void*)rawPtr)
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
	inline TResourcePtr<T1> TStaticResourcePtrCast(TResourcePtr<T2>&& other)
	{
		T2* rawPtr = other.GetRaw();
		TResourcePtr<T1> castPointer(Move(other), static_cast<T1*>(rawPtr));
		_DEBUG_LOG("{0} TStaticResourcePtrCast (move)", (void*)rawPtr)
		return castPointer;
	}

	/**
	 * @brief Generic shared resource pointer
	 */
	using ResourcePtr     = TResourcePtr<Resource>;
	
	/**
	 * @brief Generic weak resource pointer
	 */
	using ResourceWeakPtr = TResourceWeakPtr<Resource>;

	/**
	 * @brief Helper class for Resource pointers
	 */
	class ResourceMemory
	{
	public:
		/**
		 * @brief Manually increment the pointer reference count
		 * 
		 * @param ptr Pointer to increment the ref count of.
		 * @return Reference count after the operation.
		 */
		static uint32 IncRef(const TResourcePtrBase<Resource>& ptr);

		/**
		 * @brief Manually increment the reference count for a registered resource.
		 * 
		 * @param resource Resource reference to find a pointer to
		 * @return Reference count after the operation ((uint32)-1 if the resource does not exist)
		 */
		static uint32 IncRef(const Resource& resource);

		/**
		 * @brief Manually decrement the pointer reference count
		 * 
		 * @param ptr Pointer to decrement the ref count of.
		 * @return Reference count after the operation.
		 */
		static uint32 DecRef(const TResourcePtrBase<Resource>& ptr);

		/**
		 * @brief Manually decrement the reference count for a registered resource.
		 * 
		 * @param resource Resource reference to find a pointer to
		 * @return Reference count after the operation ((uint32)-1 if the resource does not exist)
		 */
		static uint32 DecRef(const Resource& resource);
	};
}
