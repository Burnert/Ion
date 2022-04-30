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

	template<typename T>
	static constexpr inline bool TIsResourcePtrV = TIsResourcePtr<T>::Value;

	struct ResourceHelper
	{
		static void UnregisterResource(Resource* ptr);
	};

	template<typename T>
	class TResourceRefCount
	{
		static_assert(TIsBaseOfV<Resource, T>);

		TResourceRefCount(T* ptr) :
			m_Ptr(ptr),
			m_RefCount(1),
			m_WeakCount(1)
		{
		}

		uint32 IncRef()
		{
			_DEBUG_LOG("{0} IncRef: {1}", (void*)m_Ptr, m_RefCount + 1)
			return ++m_RefCount;
		}

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

		uint32 IncWeak()
		{
			_DEBUG_LOG("{0} IncWeak: {1}", (void*)m_Ptr, m_WeakCount + 1)
			return ++m_WeakCount;
		}

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

		void DeleteThis()
		{
			_DEBUG_LOG("TResourceRefCount::DeleteThis")

			delete this;
		}

	private:
		T* m_Ptr;

		uint32 m_RefCount;
		uint32 m_WeakCount;

		template<typename T0>
		friend class TResourcePtrBase;
	};

	template<typename T>
	class TResourcePtrBase
	{
	public:
		static_assert(TIsBaseOfV<Resource, T>);
		using TElement  = T;
		using TThis     = TResourcePtrBase<T>;
		using TRefCount = TResourceRefCount<T>;

		bool IsExpired() const
		{
			ionassert(m_RefCount);
			return m_RefCount->m_RefCount == 0;
		}

		operator bool() const
		{
			return m_Ptr && m_RefCount;
		}

	protected:
		TResourcePtrBase() :
			TResourcePtrBase(nullptr)
		{
		}

		TResourcePtrBase(nullptr_t) :
			m_Ptr(nullptr),
			m_RefCount(nullptr)
		{
		}

		void ConstructShared(T* ptr)
		{
			ionassert(ptr);

			m_Ptr = ptr;
			m_RefCount = new TRefCount(ptr);
		}

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

		template<typename TPtr>
		void MoveConstruct(TPtr&& other)
		{
			static_assert(TIsBaseOfV<TElement, typename TPtr::TElement>);

			m_Ptr = (TElement*)other.m_Ptr;
			m_RefCount = (TRefCount*)other.m_RefCount;

			other.m_Ptr = nullptr;
			other.m_RefCount = nullptr;
		}

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

		void DeleteShared()
		{
			if (m_RefCount)
			{
				m_RefCount->DecRef();
			}

			m_Ptr = nullptr;
			m_RefCount = nullptr;
		}

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

		template<typename T1, typename T2>
		friend TResourcePtr<T1> TStaticResourcePtrCast(const TResourcePtr<T2>& other);
	};

	template<typename T>
	class TResourcePtr : public TResourcePtrBase<T>
	{
	public:
		using TBase     = TResourcePtrBase<T>;
		using TRefCount = typename TBase::TRefCount;

		TResourcePtr() :
			TResourcePtr(nullptr)
		{
		}

		TResourcePtr(nullptr_t) :
			TBase(nullptr)
		{
		}

		explicit TResourcePtr(T* ptr)
		{
			_DEBUG_LOG("TResourcePtr {0} Ptr constructor", (void*)ptr)
			ConstructShared(ptr);
		}

		template<typename T0>
		TResourcePtr(const TResourcePtr<T0>& other)
		{
			_DEBUG_LOG("TResourcePtr {0} Copy constructor", (void*)other.m_Ptr)
			CopyConstructShared(other);
		}

		template<typename T0>
		TResourcePtr(TResourcePtr<T0>&& other) noexcept
		{
			_DEBUG_LOG("TResourcePtr {0} Move constructor", (void*)other.m_Ptr)
			MoveConstruct(Move(other));
		}

		template<typename T0>
		TResourcePtr(const TResourcePtr<T0>& other, TElement* ptr)
		{
			_DEBUG_LOG("TResourcePtr {0} Alias constructor", (void*)other.m_Ptr)
			AliasConstructShared(other, ptr);
		}

		template<typename T0>
		TResourcePtr& operator=(const TResourcePtr<T0>& other)
		{
			_DEBUG_LOG("TResourcePtr {0} Copy assignment operator", (void*)other.m_Ptr)
			DeleteShared();
			CopyConstructShared(other);
			return *this;
		}

		template<typename T0>
		TResourcePtr& operator=(TResourcePtr<T0>&& other)
		{
			_DEBUG_LOG("TResourcePtr {0} Move assignment operator", (void*)other.m_Ptr)
			DeleteShared();
			MoveConstruct(Move(other));
			return *this;
		}

		TResourcePtr& operator=(nullptr_t)
		{
			_DEBUG_LOG("TResourcePtr Null assignment operator")
			DeleteShared();
			return *this;
		}

		T* GetRaw() const
		{
			return m_Ptr;
		}

		T* operator->() const
		{
			return GetRaw();
		}

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

		template<typename T0>
		TResourceWeakPtr(const TResourcePtr<T0>& ptr)
		{
			_DEBUG_LOG("TResourceWeakPtr {0} TResourcePtr constructor", (void*)ptr.GetRaw())
			ConstructWeak(ptr);
		}

		TResourceWeakPtr(const TResourceWeakPtr& other)
		{
			_DEBUG_LOG("TResourceWeakPtr {0} Copy constructor", (void*)other.m_Ptr)
			CopyConstructWeak(other);
		}

		template<typename T0>
		TResourceWeakPtr(const TResourceWeakPtr<T0>& other)
		{
			_DEBUG_LOG("TResourceWeakPtr {0} Copy constructor", (void*)other.m_Ptr)
			CopyConstructWeak(other);
		}

		template<typename T0>
		TResourceWeakPtr& operator=(const TResourceWeakPtr<T0>& other)
		{
			_DEBUG_LOG("TResourceWeakPtr {0} Copy assignment operator", (void*)other.m_Ptr)
			DeleteWeak();
			CopyConstructWeak(other);
			return *this;
		}

		template<typename T0>
		TResourceWeakPtr& operator=(TResourceWeakPtr<T0>&& other)
		{
			_DEBUG_LOG("TResourceWeakPtr {0} Move assignment operator", (void*)other.m_Ptr)
			DeleteWeak();
			MoveConstruct(Move(other));
			return *this;
		}

		TResourceWeakPtr& operator=(nullptr_t)
		{
			_DEBUG_LOG("TResourceWeakPtr Null assignment operator")
			DeleteWeak();
			return *this;
		}

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

	template<typename T1, typename T2>
	inline TResourcePtr<T1> TStaticResourcePtrCast(const TResourcePtr<T2>& other)
	{
		T2* rawPtr = other.GetRaw();
		TResourcePtr<T1> castPointer(other, static_cast<T1*>(rawPtr));
		_DEBUG_LOG("{0} TStaticResourcePtrCast (copy)", (void*)rawPtr)
		return castPointer;
	}

	template<typename T1, typename T2>
	inline TResourcePtr<T1> TStaticResourcePtrCast(TResourcePtr<T2>&& other)
	{
		T2* rawPtr = other.GetRaw();
		TResourcePtr<T1> castPointer(Move(other), static_cast<T1*>(rawPtr));
		_DEBUG_LOG("{0} TStaticResourcePtrCast (move)", (void*)rawPtr)
		return castPointer;
	}

	using ResourcePtr     = TResourcePtr<Resource>;
	using ResourceWeakPtr = TResourceWeakPtr<Resource>;
}
