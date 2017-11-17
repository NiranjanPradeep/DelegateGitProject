

#pragma once

#include <functional>

struct Delegate_Types
{
	using InstancePtr_t = void *;
	using Function_t = void(int);
	template <typename C>
		using ClassFunction_t = void(C*, int);
	using FunctionPtr_t = void(*)(int);
	using STDFunction_t = std::function<Function_t>;
	template <typename C>
		using STDClassFunction_t = std::function<ClassFunction_t<C>>;
	using InternalFunction_t = void(void *, int);
	using STDInternalFunction_t = std::function<InternalFunction_t>;
	template<typename C>
		using ClassFunctionPtr_t = void(C::*)(int);
};

struct Delegate_Static			{};
struct Delegate_Dynamic			{};
struct Delegate_Policy_Single	{};
struct Delegate_Policy_Multiple {};

template<typename T>
struct Delegate_Policy_Impl {};

template<>
struct Delegate_Policy_Impl<Delegate_Policy_Single>
{
private:
	Delegate_Types::InstancePtr_t				m_Instance;
	Delegate_Types::STDInternalFunction_t		m_Function;
	Delegate_Types::STDFunction_t				m_Lambda;
public:
	void Assign(Delegate_Types::InstancePtr_t a, Delegate_Types::STDInternalFunction_t b)
	{
		m_Instance = a;
		m_Function = b;
	}
	void Assign(Delegate_Types::STDFunction_t &&a, Delegate_Types::STDInternalFunction_t b)
	{
		m_Lambda = Delegate_Types::STDFunction_t(std::move(a));
		m_Instance = &m_Lambda;
		m_Function = b;
	}
	
	void Invoke(int i)
	{
		m_Function(m_Instance, i);
	}
};

template<>
struct Delegate_Policy_Impl<Delegate_Policy_Multiple>
{
private:
	struct Element
	{
		Delegate_Types::InstancePtr_t			m_Instance;
		Delegate_Types::STDInternalFunction_t	m_Function;
	};
	std::vector<Element> DelegateList;
public:
	void Assign(Delegate_Types::InstancePtr_t a, Delegate_Types::STDInternalFunction_t b)
	{
		DelegateList.push_back({});
		DelegateList.back().m_Instance = a;
		DelegateList.back().m_Function = b;
	}

	void Assign(Delegate_Types::STDFunction_t &&a, Delegate_Types::STDInternalFunction_t b)
	{
		DelegateList.push_back({});
		DelegateList.back().m_Instance = new Delegate_Types::STDFunction_t(std::move(a));
		DelegateList.back().m_Function = b;
	}
	void Invoke(int i)
	{
		for (auto &t : DelegateList)
			t.m_Function(t.m_Instance, i);
	}
};

template <typename Policy = Delegate_Policy_Single, typename Type = Delegate_Static>
class Delegate : private Delegate_Policy_Impl<Policy>, public Delegate_Types
{
public:
// Static Allocations
	template <
		FunctionPtr_t FUNC
	>
	void Bind()				// Function Pointer
	{
		Assign(nullptr, STDInternalFunction_t(FreeFunction<FUNC>));
	}

	template <
		typename C,
		ClassFunctionPtr_t<C> FUNC
	>
	void Bind(C* Instance)	// Class Member Function Pointer
	{
		Assign(Instance, STDInternalFunction_t(ClassFunction<C, FUNC>));
	}

	template <STDFunction_t *func>
	void Bind()				// global std::function, or Functor(?)
	{
		Assign(nullptr, STDInternalFunction_t(STDFunction<func>));
	}


// Dynamic Allocation
	template <typename T>
	void* castToVoidStar(T x) {
		return *reinterpret_cast<void**>(&x);
	}

	template <typename U = Type>
	std::enable_if_t<
		std::is_same_v<U, Delegate_Dynamic>
	>
	Bind(FunctionPtr_t a)	// Function Pointer
	{
		Assign(a, STDInternalFunction_t(FreeFunction<Delegate_Dynamic>));
	}

	template <typename C, typename U = Type>
	std::enable_if_t<
		std::is_same_v<U, Delegate_Dynamic>
	>
	Bind(ClassFunctionPtr_t<C> a, C *Instance)	// Class Member Function Pointer
	{
		std::function<void(int)> tempfunction = [=](int i) {(Instance->*a)(i); };
		Assign(std::move(tempfunction), STDInternalFunction_t(ClassFunction<C, Delegate_Dynamic>));
	}

	template <typename U = Type>
	std::enable_if_t<
		std::is_same_v<U, Delegate_Dynamic>
	>    
	Bind(STDFunction_t &a)	// std::function
	{
		Assign(&a, STDInternalFunction_t(STDFunction<Delegate_Dynamic>));
	}

	template <typename T, typename U = Type>
	std::enable_if_t<
		std::is_same_v<U, Delegate_Dynamic>
	>
	Bind(T &&a)				// Temporary std::function, functor, or lambda
	{
		Assign(std::move(a), STDInternalFunction_t(STDFunction<Delegate_Dynamic>));
	}

	void Invoke(int i)
	{
		this->Delegate_Policy_Impl<Policy>::Invoke(i);
	}

private:
// Static Functions
	template <void(*FUNC)(int)>
	inline static void FreeFunction(InstancePtr_t, int i)
	{
		(*FUNC)(i);
	}
	template <typename C, void (C::*FUNC)(int)>
	inline static void ClassFunction(InstancePtr_t Instance, int i)
	{
		(static_cast<C*>(Instance)->*FUNC)(i);
	}


	template <STDFunction_t *FUNC>
	inline static void STDFunction(InstancePtr_t, int i)
	{
		(*FUNC)(i);
	}

// Dynamic Functions
	template <typename C, typename U = Type>
	std::enable_if_t<
		std::is_same_v<U, Delegate_Dynamic>
	>
	inline static ClassFunction(InstancePtr_t func, int i)
	{
		try 
		{ 
			(* reinterpret_cast< STDFunction_t *>(func))(i);
		}
		catch (const std::bad_function_call &s)
		{
			std::cout << "\n" << s.what();
		}
	}

	template <typename U = Type>
	std::enable_if_t<
		std::is_same_v<U, Delegate_Dynamic>
	>
	inline static FreeFunction(InstancePtr_t Func, int i)
	{
		(*static_cast<FunctionPtr_t>(Func))(i);
	}

	template <typename U = Type>
	std::enable_if_t<
		std::is_same_v<U, Delegate_Dynamic>
	>
	inline static STDFunction(InstancePtr_t func, int i)
	{
		try { STDFunction_t &stdfunc = *static_cast<STDFunction_t *>(func); stdfunc(i); }
		catch (const std::bad_function_call &s)
		{
			std::cout << "\n" << s.what();
		}
	}
};

