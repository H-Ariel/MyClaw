#pragma once

#include <iostream>
#include <vector>
#include <map>
#include <functional>


class json
{
private:
	enum class JsonType;
	typedef std::string Str_t;
	typedef std::vector<json> List_t;
	typedef std::map<Str_t, json> Dict_t;
	typedef long long Int_t;
	typedef double Float_t;


public:
	json();
	json(bool n);
	json(Int_t n);
	json(Float_t n);
	json(const Str_t& n);
	json(const List_t& n);
	json(const Dict_t& n);
	json(const json& j);
	json(json&& j) noexcept;
	~json();

	json(short n) : json((Int_t)n) {}
	json(unsigned short n) : json((Int_t)n) {}
	json(int n) : json((Int_t)n) {}
	json(unsigned int n) : json((Int_t)n) {}
	json(float n) : json((Float_t)n) {}

	template<template<class ...> class ListContainer, class T>
	json(const ListContainer<T>& n) : data(nullptr) {
		List_t list;
		for (auto& i : n)
			list.push_back(json(i));
		SetList(list);
	}

	json& operator=(const nullptr_t&);
	json& operator=(const json& j);
	json& operator=(json&& j) noexcept;

	operator const bool() const;
	operator const short() const;
	operator const int() const;
	operator const Int_t() const;
	operator const float() const;
	operator const Float_t() const;
	operator const char* () const;
	operator const Str_t() const;
	operator const List_t() const;
	operator const Dict_t() const;

	json& operator[](const Str_t& key);
	json& operator[](const char key[]);
	json& operator[](size_t idx);
	json& operator[](int idx);

	const json& operator[](const Str_t& key) const;
	const json& operator[](const char key[]) const;
	const json& operator[](size_t idx) const;
	const json& operator[](int idx) const;

	void append(const json& j);

	bool contains(const Str_t& key) const;

	void iterate(std::function<void(const json&)> func) const;
	void iterate(std::function<void(const Str_t&, const json&)> func) const;

	void clear();

	std::string ToString() const;

#ifdef COMPILE_JSON_PARSER
	static json FromString(const std::string& string);
	static json FromFile(const std::string& filename);
#endif

private:
	void SetBool(bool n);
	void SetInt(Int_t n);
	void SetFloat(Float_t n);
	void SetStr(const Str_t& n);
	void SetList(const List_t& n);
	void SetDict(const Dict_t& n);

	void* data = nullptr;
	JsonType type;

#ifdef COMPILE_JSON_PARSER
	class Parser
	{
	public:
		Parser(std::string json_str);
		json Parse();

	private:
		json ReadBool();
		json ReadNumber();
		json ReadString();
		json ReadList();
		json ReadDict();
		void skipSpaces();

		const std::string json_str;
		size_t pos;
	};
#endif
};


std::ostream& operator<<(std::ostream& os, const json& j);
#ifdef COMPILE_JSON_PARSER
std::istream& operator>>(std::istream& is, json& j);
#endif
