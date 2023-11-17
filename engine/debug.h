#pragma once
#include <string>
#include <functional>
#include <iostream>

namespace Engine
{
	struct Error
	{
		std::string message;

		Error();
		Error(const Error& rhs);
		Error(const std::string& _message);
		Error& operator=(const Error& rhs);

		void Print() const;
	};

	inline Error::Error()
	{}

	inline Error::Error(const Error& rhs) :
		message(rhs.message)
	{}

	inline Error::Error(const std::string& _message) :
		message(_message)
	{}

	inline Error& Error::operator=(const Error& rhs)
	{
		message = rhs.message;
		return *this;
	}

	inline void Error::Print() const
	{
		std::cout << "[ERROR] " << message << std::endl;
	}


	inline std::string ToString(const char* val)
	{
		return val;
	}

	inline std::string ToString(const std::string& val)
	{
		return val;
	}

	inline std::string ToString(int val)
	{
		return std::to_string(val);
	}

	inline std::string ToString(float val)
	{
		return std::to_string(val);
	}

	template<typename... ARGS>
	void Affirm(bool test, ARGS... args)
	{
		if (test)
			return;

		Error error;
		error.message = (ToString(args) + ...);

		while (error.message.back() == '\n')
			error.message.pop_back();

		throw(error);
	}

	template<typename... ARGS>
	void Info(ARGS... args)
	{
		std::string message = (ToString(args) + ...);

		while (message.back() == '\n')
			message.pop_back();

		std::cout << "[INFO] " << message << std::endl;
	}
}

#define TRY(instructions) [&](){try{instructions;}catch(const Engine::Error& error){error.Print();return false;}return true;}()