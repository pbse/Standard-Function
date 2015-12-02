/*
 * Function.hpp
 *
 *  Created on: Nov 14, 2015
 *      Author: prash_000
 */

#ifndef FUNCTION_HPP_
#define FUNCTION_HPP_
#include <iostream>
#include <type_traits>

#define Forward(type, value) static_cast<type &&>(value)

using namespace std;

namespace cs540 {
class BadFunctionCall : public std::exception {
public:
	virtual const char* What() const throw() {
		return "Error Generated";
	}
};

template <typename ResultType, typename ... ArgumentTypes>
class Function_Storage {
public:
	ResultType (*call)(void*, ArgumentTypes&&...) = nullptr;
	void (*destroy)(void*) = nullptr;
	void (*copy)(const void*, void*) = nullptr;
};

template <typename T>
class Function;

template <typename ResultType, typename ... ArgumentTypes>
class Function<ResultType(ArgumentTypes...)> {

public:

	Function_Storage<ResultType, ArgumentTypes...> v_;
	std::aligned_storage<16> s_;

	template <typename F>
	static ResultType call_impl(void * func, ArgumentTypes&&... args) {
		return (*static_cast<F*>(func))(Forward(ArgumentTypes, args)...);
	}

	template <typename F>
	static void destroy_impl(void * func) {
		static_cast<F*>(func)->~F();
	}

	template<typename F>
	static void copy_impl(const void* f, void* dest)
	{
		new (dest) F(*static_cast<F const*>(f));
	}

	Function() {}

	template <typename FunctionType>
	Function(FunctionType F) {
		FunctionType&& f = Forward(FunctionType, F);
		using functor_type = typename std::decay<FunctionType>::type;
		new (&s_) functor_type(Forward(FunctionType,f));
		v_.call = &call_impl<functor_type>;
		v_.destroy = &destroy_impl<functor_type>;
		v_.copy = &copy_impl<functor_type>;
		//cout<<"call came here";
	}

	Function(const Function &F) {
		if(F.v_.copy==nullptr) cout<<"\nnull\n";
		if (F.v_.copy)
		{
			//cout<<"called2";
			F.v_.copy(&F.s_, &s_);
			v_ = F.v_;
		}
	}

	Function &operator=(const Function &F) {
		auto destroy = v_.destroy;
		if(destroy) {
			v_ = Function_Storage<ResultType, ArgumentTypes...>();
			destroy(&s_);
		}
		if (F.v_.copy){
			F.v_.copy(&F.s_, &s_);
			v_ = F.v_;
		}
		return *this;
	}

	ResultType operator()(ArgumentTypes... arg) {
		if(v_.call==nullptr) throw BadFunctionCall();
		return v_.call(&s_, Forward(ArgumentTypes, arg)...);
	}

	explicit operator bool() const {
		return v_.call!=nullptr;
	}

	~Function() {
		auto destroy = v_.destroy;
		if(destroy) {
			v_ = Function_Storage<ResultType, ArgumentTypes...>();
			destroy(&s_);
		}
	}

	friend bool operator==(const Function<ResultType(ArgumentTypes...)> &f, std::nullptr_t) {
		return !f;
	}

	friend bool operator==(std::nullptr_t, const Function<ResultType(ArgumentTypes...)> & f) {
		return !f;
	}

	friend bool operator!=(const Function<ResultType(ArgumentTypes...)> &f, std::nullptr_t) {
		return bool(f);
	}

	friend bool operator!=(std::nullptr_t, const Function<ResultType(ArgumentTypes...)> &f) {
		return bool(f);
	}

};

}


#endif /* FUNCTION_HPP_ */
