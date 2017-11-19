

#pragma once

template <typename T = void>
struct Delegate_Traits
{
	using InstancePtr_t = void *;
	using Function_t = void(int);
	// template <typename C> using ClassFunction_t = void(C*, int);
	using FunctionPtr_t = void(*)(int);
	using STDFunction_t = std::function<Function_t>;
	using InternalFunction_t = void(void *, int);
	template<typename C>
	using ClassFunctionPtr_t = void(C::*)(int);
};
