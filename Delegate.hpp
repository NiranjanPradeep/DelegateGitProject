

#pragma once

#include <type_traits>

struct DelegateTypeSingle;
struct DelegateTypeMultiple;

template <typename Type, typename R, typename... T>
class Delegate;

template <typename Type, typename R, typename... T>
class Delegate<R(T...), Type>
{
private:
	using InstancePtr			= void *;
	using FunctionPtr			= R(*)(T...);
	template <typename C>
	using ClassFunctionPtr		= R(C::*)(T...);
	using InternalFunctionPtr	= R (*)(InstancePtr, T &&...);
	using DelegateInfo			= std::pair<InternalFunctionPtr, InstancePtr>;
	std::vector<DelegateInfo>	m_DelegateInfoList;

private:
	template <FunctionPtr Function>
	static R		InternalFreeFunction(InstancePtr, T && ... ARGS)
	{
		return Function(std::forward<T>(ARGS)...);
	}

	template <typename C, ClassFunctionPtr<C> Function>
	static R		InternalClassFunction(InstancePtr pInstance, T && ... ARGS)
	{
		return (static_cast<C*>(pInstance)->*Function)(std::forward<T>(ARGS)...);
	}

public:
	Delegate()
	{
		Initialize<Type>();
	}

	template <typename V = Type>
	std::enable_if_t< std::is_same_v<V, DelegateTypeSingle>> 
	Initialize()
	{
		m_DelegateInfoList.emplace_back();
	}

	template <typename V = Type>
	std::enable_if_t< std::is_same_v<V, DelegateTypeMultiple>>
	Initialize()
	{
		
	}

	template
		<
		FunctionPtr Function,
		typename V = Type
		>
	std::enable_if_t < std::is_same_v<V, DelegateTypeSingle> >
	Bind(void)
	{
		m_DelegateInfoList.back() = { &InternalFreeFunction<Function>, nullptr };
	}

	template 
		<
		FunctionPtr Function,
		typename V = Type
		>
	std::enable_if_t < std::is_same_v<V, DelegateTypeMultiple> >
	Bind(void)
	{
		m_DelegateInfoList.emplace_back(&InternalFreeFunction<Function>, nullptr);
	}

	template
		<
		typename C,
		ClassFunctionPtr<C> Function,
		typename V = Type
		>
	std::enable_if_t < std::is_same_v<V, DelegateTypeSingle> >
	Bind(InstancePtr pInstance)
	{
		m_DelegateInfoList.back() = { &InternalClassFunction<C, Function>, pInstance };
	}

	template
		<
		typename C,
		ClassFunctionPtr<C> Function,
		typename V = Type
		>
	std::enable_if_t < std::is_same_v<V, DelegateTypeMultiple> >
	Bind(InstancePtr pInstance)
	{
		m_DelegateInfoList.emplace_back(&InternalClassFunction<C, Function>, pInstance);
	}

	template <typename... U>
	R			Invoke(U && ... ARGS) const
	{
		for (auto &t : m_DelegateInfoList)
			t.first(t.second, std::forward<U>(ARGS)...);
	}
};
