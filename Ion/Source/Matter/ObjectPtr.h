#pragma once

#include "Core.h"

namespace Ion
{
#pragma region Matter Object pointers

	class MObject;

	template<typename T>
	using TObjectPtr = TSharedPtr<T>;

	template<typename T>
	using TWeakObjectPtr = TWeakPtr<T>;

	using MObjectPtr = TSharedPtr<MObject>;
	using MWeakObjectPtr = TWeakPtr<MObject>;

#pragma endregion

#pragma region Templates - TRemoveObjectPtr

	template<typename T>
	struct TRemoveObjectPtr { using Type = T; };

	template<typename T>
	struct TRemoveObjectPtr<TObjectPtr<T>> { using Type = T; };

	template<typename T>
	struct TRemoveObjectPtr<TWeakObjectPtr<T>> { using Type = T; };

	template<>
	struct TRemoveObjectPtr<MObjectPtr> { using Type = MObject; };

	template<>
	struct TRemoveObjectPtr<MWeakObjectPtr> { using Type = MObject; };

#pragma endregion

#pragma region Templates - TIsObjectPtr

	template<typename T>
	struct TIsObjectPtr { static constexpr bool Value = false; };

	template<typename T>
	struct TIsObjectPtr<TObjectPtr<T>> { static constexpr bool Value = true; };

	template<typename T>
	struct TIsObjectPtr<TWeakObjectPtr<T>> { static constexpr bool Value = true; };

	template<typename T>
	static constexpr bool TIsObjectPtrV = TIsObjectPtr<T>::Value;

#pragma endregion
}
