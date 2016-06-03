#pragma once

#include <functional>
#include <string>
#include <memory>
#include <vector>

template <typename T, typename L = typename std::decay<T>::type>
std::shared_ptr<L> make_shared_lambda(T&& t)
{
	return std::make_shared<L>(std::forward<T>(t));
}

template<typename T>
class Dispatcher;

template<typename Ret, typename ...Args>
class Dispatcher<Ret(Args...)>
{
public:
	template<Ret(*funcPtr)(Args...)>
	bool add(const std::string& tag)
	{
		return addImpl(tag, funcPtr);
	}

	template<typename T, Ret(T::*funcPtr)(Args...)>
	bool add(const std::string& tag, std::shared_ptr<T> obj)
	{
		return add(tag, makeFunction(obj, funcPtr));
	}

	template<typename T>
	bool add(const std::string& tag, std::shared_ptr<T> t)
	{
		return addImpl(tag, *t.get());
	}

	bool remove(const std::string& tag)
	{
		auto it = find(tags.begin(), tags.end(), tag);
		if (it == tags.end())
		{
			return false;
		}

		auto index{ distance(tags.begin(), it) };
		tags.erase(it);

		delegates.erase(delegates.begin() + index);

		return true;
	}

	void operator()(Args... args)
	{
		for (auto& delegate : delegates)
		{
			delegate(args...);
		};
	}

private:
	template <class T, class Ret, class ... Args>
	std::function<Ret(Args...)> makeFunction(std::shared_ptr<T> t, Ret(T::*funcPtr)(Args...))
	{
		return [t, funcPtr](Args ... args) mutable -> R { return ((t).*(funcPtr))(args...); };
	}

	bool addImpl(const std::string& tag, std::function<Ret(Args...)> delegate)
	{
		if (find(tags.begin(), tags.end(), tag) != tags.end())
		{
			return false;
		}

		delegates.push_back(delegate);
		tags.push_back(tag);

		return true;
	}

private:
	std::vector<std::function<Ret(Args...)>> delegates;
	std::vector<std::string> tags;
};