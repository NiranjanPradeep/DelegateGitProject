

#pragma once

struct Delegate_Policy_Single {};
struct Delegate_Policy_Multiple {};

template<typename T>
struct Delegate_Policy_Impl {};

template<>
struct Delegate_Policy_Impl<Delegate_Policy_Single>
{
	using DT = Delegate_Traits<int>;
private:
	DT::InstancePtr_t				m_Instance;
	DT::InternalFunction_t *		m_Function;
	DT::STDFunction_t				m_Lambda;
public:
	void Assign(DT::InstancePtr_t a, DT::InternalFunction_t *b)
	{
		m_Instance = a;
		m_Function = b;
	}
	void Assign(DT::STDFunction_t &&a, DT::InternalFunction_t *b)
	{
		m_Lambda = std::move(a);
		m_Instance = &m_Lambda;
		m_Function = b;
	}
	void Assign(DT::STDFunction_t &a, DT::InternalFunction_t *b)
	{
		m_Lambda = a;
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
	using DT = Delegate_Traits<int>;
private:
	struct Element
	{
		DT::InstancePtr_t			m_Instance;
		DT::InternalFunction_t*	m_Function;
	};
	std::vector<Element> DelegateList;
public:
	void Assign(DT::InstancePtr_t a, DT::InternalFunction_t *b)
	{
		DelegateList.push_back({});
		DelegateList.back().m_Instance = a;
		DelegateList.back().m_Function = b;
	}

	void Assign(DT::STDFunction_t &&a, DT::InternalFunction_t *b)
	{
		DelegateList.push_back({});
		DelegateList.back().m_Instance = new DT::STDFunction_t(std::move(a));
		DelegateList.back().m_Function = b;
	}
	void Assign(DT::STDFunction_t &a, DT::InternalFunction_t *b)
	{
		DelegateList.push_back({});
		DelegateList.back().m_Instance = &a;
		DelegateList.back().m_Function = b;
	}
	void Invoke(int i)
	{
		for (auto &t : DelegateList)
			t.m_Function(t.m_Instance, i);
	}
};
