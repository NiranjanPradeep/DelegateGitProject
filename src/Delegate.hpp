

#pragma once

#include <functional>
#include "Delegate_Traits.hpp"
#include "Delegate_Policies.hpp"

struct Delegate_Static			{};
struct Delegate_Dynamic			{};

template <typename Policy = Delegate_Policy_Single, typename Type = Delegate_Static>
class Delegate : Delegate_Policy_Impl<Policy>
{
	using DT = Delegate_Traits<>;
public:
// Static Allocations
	template <
		DT::FunctionPtr_t FUNC
	>
	void Bind()				// Function Pointer
	{
		std::cout << "\nGlobal Free Function Pointer";
		Assign(nullptr, (FreeFunction<FUNC>));
	}

	template <
		typename C,
		DT::ClassFunctionPtr_t<C> FUNC
	>
	void Bind(C* Instance)	// Class Member Function Pointer
	{
		//std::cout << "\nGlobal Class Member Function Pointer";
		Assign(Instance, (ClassFunction<C, FUNC>));
	}

	template <typename T, T *a>
	std::enable_if_t<std::is_convertible_v<decltype(*a), DT::STDFunction_t>>
	Bind()
	{
		//std::cout << "\nGlobal std::function";
		Assign(nullptr, (STDFunction<T, a>));
	}

	template <DT::STDFunction_t *FUNC>
	void Bind()
	{
		Bind<DT::STDFunction_t, FUNC>();
	}

// Dynamic Allocation
	template <typename U = Type>
	std::enable_if_t<
		std::is_same_v<U, Delegate_Dynamic>
	>
	Bind(DT::FunctionPtr_t a)	// Function Pointer
	{
		//std::cout << "\nDynamic Free Function Pointer";
		Assign(a, (FreeFunction<Delegate_Dynamic>));
	}

	template <typename C, typename U = Type>
	std::enable_if_t<
		std::is_same_v<U, Delegate_Dynamic>
	>
	Bind(DT::ClassFunctionPtr_t<C> a, C *Instance)	// Class Member Function Pointer
	{
		//std::cout << "\nDynamic Class Member Function Pointer";
		Assign(std::move([=](int i) {(Instance->*a)(i); }), (ClassFunction<C, Delegate_Dynamic>));
	}

	template <typename T, typename U = Type>
	std::enable_if_t<
		std::is_same_v<U, Delegate_Dynamic> && std::is_lvalue_reference_v<T>
	>    
	Bind(T &&a)				// lvalue std::function
	{
		//std::cout << "\nDynamic Local std::function";
		Assign(std::ref(a), (STDFunction<Delegate_Dynamic>));
	}

	template <typename T, typename U = Type>
	std::enable_if_t<
		std::is_same_v<U, Delegate_Dynamic> && std::is_rvalue_reference_v<T&&>
	>
	Bind(T &&a)				// rvalue std::function, functor, or lambda
	{
		//std::cout << "\nDynamic Temporary std::function";
		Assign(std::move(a), (STDFunction<Delegate_Dynamic>));
	}

	void Invoke(int i)
	{
		this->Delegate_Policy_Impl<Policy>::Invoke(i);
	}

private:
// Static Functions
	template <void(*FUNC)(int)>
	inline static void FreeFunction(DT::InstancePtr_t, int i)
	{
		(*FUNC)(i);
	}
	template <typename C, void (C::*FUNC)(int)>
	inline static void ClassFunction(DT::InstancePtr_t Instance, int i)
	{
		(static_cast<C*>(Instance)->*FUNC)(i);
	}

	template <typename T, T *FUNC>
	inline static void STDFunction(DT::InstancePtr_t, int i)
	{
		(*FUNC)(i);
	}

// Dynamic Functions
	template <typename C, typename U = Type>
	std::enable_if_t<
		std::is_same_v<U, Delegate_Dynamic>
	>
	inline static ClassFunction(DT::InstancePtr_t func, int i)
	{
		return (* static_cast<DT::STDFunction_t *>(func))(i);
	}

	template <typename U = Type>
	std::enable_if_t<
		std::is_same_v<U, Delegate_Dynamic>
	>
	inline static FreeFunction(DT::InstancePtr_t Func, int i)
	{
		return (*static_cast<DT::FunctionPtr_t>(Func))(i);
	}

	template <typename U = Type>
	std::enable_if_t<
		std::is_same_v<U, Delegate_Dynamic>
	>
	inline static STDFunction(DT::InstancePtr_t Func, int i)
	{
		return (*static_cast<DT::STDFunction_t *>(Func))(i);
	}
};
