#include "json.h"
#include <sstream>
#include <fstream>

#define THROW_TYPE_ERROR(type)	throw std::exception("type is not " #type );
#define THROW_INVALID_JSON		throw std::exception("invalid json")


enum class json::JsonType
{
	None,
	Boolean,
	Integer,
	Float,
	String,
	List,
	Dictionary
};


json::json() { data = nullptr; type = JsonType::None; }
json::json(bool n) : data(nullptr) { SetBool(n); }
json::json(Int_t n) : data(nullptr) { SetInt(n); }
json::json(Float_t n) : data(nullptr) { SetFloat(n); }
json::json(const Str_t& n) : data(nullptr) { SetStr(n); }
json::json(const List_t& n) : data(nullptr) { SetList(n); }
json::json(const Dict_t& n) : data(nullptr) { SetDict(n); }
json::json(const json& j) : data(nullptr) { *this = j; }
json::json(json&& j) noexcept
{
	type = j.type;
	data = j.data;
	j.type = JsonType::None;
	j.data = nullptr;
}
json::~json() { clear(); }

json& json::operator=(const nullptr_t&) { clear(); return *this; }
json& json::operator=(const json& j)
{
	clear();
	switch (j.type)
	{
	case json::JsonType::Boolean: SetBool(*(Int_t*)j.data); break;
	case json::JsonType::Integer: SetInt(*(Int_t*)j.data); break;
	case json::JsonType::Float: SetFloat(*(Float_t*)j.data); break;
	case json::JsonType::String: SetStr(*(Str_t*)j.data); break;
	case json::JsonType::List: SetList(*(List_t*)j.data); break;
	case json::JsonType::Dictionary: SetDict(*(Dict_t*)j.data); break;
	}
	return *this;
}
json& json::operator=(json&& j) noexcept
{
	type = j.type;
	data = j.data;
	j.type = JsonType::None;
	j.data = nullptr;
	return *this;
}

json::operator const bool() const
{
	if (type == JsonType::Boolean || type == JsonType::Integer)
		return (bool)(*(Int_t*)data);
	THROW_TYPE_ERROR(boolean);
}
json::operator const short() const
{
	if (type == JsonType::Integer)
		return (short)(*(Int_t*)data);
	if (type == JsonType::Float)
		return (short)(*(Float_t*)data);
	THROW_TYPE_ERROR(integer);
}
json::operator const int() const
{
	if (type == JsonType::Integer)
		return (int)(*(Int_t*)data);
	if (type == JsonType::Float)
		return (int)(*(Float_t*)data);
	THROW_TYPE_ERROR(integer);
}
json::operator const Int_t() const
{
	if (type == JsonType::Integer)
		return (*(Int_t*)data);
	if (type == JsonType::Float)
		return (Int_t)(*(Float_t*)data);
	THROW_TYPE_ERROR(integer);
}
json::operator const float() const
{
	if (type == JsonType::Float)
		return (float)(*(Float_t*)data);
	if (type == JsonType::Integer)
		return (float)(*(Int_t*)data);
	THROW_TYPE_ERROR(float);
}
json::operator const Float_t() const
{
	if (type == JsonType::Float)
		return (*(Float_t*)data);
	if (type == JsonType::Integer)
		return (Float_t)(*(Int_t*)data);
	THROW_TYPE_ERROR(float);
}
json::operator const char* () const
{
	if (type != JsonType::String)
		THROW_TYPE_ERROR(string);
	return (*(Str_t*)data).c_str();
}
json::operator const Str_t() const
{
	if (type != JsonType::String)
		THROW_TYPE_ERROR(string);
	return (*(Str_t*)data);
}
json::operator const List_t() const
{
	if (type != JsonType::List)
		THROW_TYPE_ERROR(list);
	return (*(List_t*)data);
}
json::operator const Dict_t() const
{
	if (type != JsonType::Dictionary)
		THROW_TYPE_ERROR(dictionary);
	return (*(Dict_t*)data);
}

json& json::operator[](const Str_t& key)
{
	if (type == JsonType::None)
		SetDict({});

	if (type == JsonType::Dictionary)
	{
		type = JsonType::Dictionary;
		return (*(Dict_t*)data)[key];
	}
	THROW_TYPE_ERROR(dictionary);
}
json& json::operator[](const char key[])
{
	return (*this)[Str_t(key)];
}
json& json::operator[](size_t idx)
{
	if (type != JsonType::List) THROW_TYPE_ERROR(list);

	if (idx < (*(List_t*)data).size())
		return (*(List_t*)data)[idx];

	throw std::exception(("json does not contains index: " + std::to_string(idx)).c_str());
}
json& json::operator[](int idx)
{
	return this->operator[]((size_t)idx);
}

const json& json::operator[](const Str_t& key) const
{
	if (type == JsonType::Dictionary)
		return (*(Dict_t*)data).at(key);
	THROW_TYPE_ERROR(dictionary);
}
const json& json::operator[](const char key[]) const
{
	return (*this)[Str_t(key)];
}
const json& json::operator[](size_t idx) const
{
	if (type != JsonType::List) THROW_TYPE_ERROR(list);

	if (idx < (*(List_t*)data).size())
		return (*(List_t*)data)[idx];

	throw std::exception(("json does not contains index: " + std::to_string(idx)).c_str());
}
const json& json::operator[](int idx) const
{
	return this->operator[]((size_t)idx);
}

void json::append(const json& j)
{
	if (type == JsonType::None)
		SetList({});
	if (type == JsonType::List)
		(*(List_t*)data).push_back(j);
	else
		THROW_TYPE_ERROR(list);
}

bool json::contains(const Str_t& key) const
{
	if (type != JsonType::Dictionary)
		return false;
	return (*(Dict_t*)data).count(key);
}

void json::iterate(std::function<void(const json&)> func) const
{
	if (type != JsonType::List)
		THROW_TYPE_ERROR(list);

	for (const json& a : (*(List_t*)data))
		func(a);
}
void json::iterate(std::function<void(const std::string&, const json&)> func) const
{
	if (type != JsonType::Dictionary)
		THROW_TYPE_ERROR(dictionary);

	for (const std::pair<const std::string, const json>& a : (*(Dict_t*)data))
		func(a.first, a.second);
}

void json::clear()
{
	if (data) {
		switch (type) {
		case JsonType::Boolean:
		case JsonType::Integer:
			delete static_cast<Int_t*>(data);
			break;
		case JsonType::Float:
			delete static_cast<Float_t*>(data);
			break;
		case JsonType::String:
			delete static_cast<Str_t*>(data);
			break;
		case JsonType::List:
			delete static_cast<List_t*>(data);
			break;
		case JsonType::Dictionary:
			delete static_cast<Dict_t*>(data);
			break;
		default:
			break;
		}
	}

	data = nullptr;
	type = JsonType::None;
}

std::string json::ToString() const
{
	std::stringstream sstr;

	switch (type)
	{
	case JsonType::None:
		sstr << "null";
		break;

	case JsonType::Boolean:
		sstr << std::boolalpha << (bool)(*(Int_t*)data);
		break;

	case JsonType::Integer:
		sstr << (*(Int_t*)data);
		break;

	case JsonType::Float:
		sstr << (*(Float_t*)data);
		break;

	case JsonType::String:
		sstr << '"' << (*(Str_t*)data) << '"';
		break;

	case JsonType::List:
	{
		List_t l = *(List_t*)data;

		sstr << "[ ";
		const size_t size = l.size();
		if (size)
		{
			size_t i = 0;
			sstr << l[i].ToString();
			for (i = i + 1; i < size; i++)
			{
				sstr << ", " << l[i].ToString();
			}
		}
		sstr << " ]";
	}
	break;

	case JsonType::Dictionary:
	{
		Dict_t d = *(Dict_t*)data;

		sstr << "{ ";
		const size_t size = d.size();
		if (size)
		{
			size_t i = 0;
			for (auto& a : d)
			{
				sstr << '"' << a.first << "\": " << a.second.ToString();
				if (i < size - 1)
					sstr << ", ";
				i++;
			}
		}
		sstr << " }";
	}
	break;

	default:
		break;
	}

	return sstr.str();
}

void json::SetBool(bool n)
{
	clear();
	type = JsonType::Boolean;
	data = new Int_t;
	*(Int_t*)data = n;
}
void json::SetInt(Int_t n)
{
	clear();
	type = JsonType::Integer;
	data = new Int_t;
	*(Int_t*)data = n;
}
void json::SetFloat(Float_t n)
{
	clear();
	type = JsonType::Float;
	data = new Float_t;
	*(Float_t*)data = n;
}
void json::SetStr(const Str_t& n)
{
	clear();
	type = JsonType::String;
	data = new Str_t;
	*(Str_t*)data = n;
}
void json::SetList(const List_t& n)
{
	clear();
	type = JsonType::List;
	data = new List_t;
	*(List_t*)data = n;
}
void json::SetDict(const Dict_t& n)
{
	clear();
	type = JsonType::Dictionary;
	data = new Dict_t;
	*(Dict_t*)data = n;
}


std::ostream& operator<<(std::ostream& os, const json& j)
{
	return os << j.ToString();
}

#ifdef COMPILE_JSON_PARSER
json json::FromString(const std::string& string)
{
	Parser parser(string);
	return parser.Parse();
}
json json::FromFile(const std::string& filename)
{
	json json;
	std::ifstream file(filename);
	if (file.is_open())
		file >> json;
	return json;
}

std::istream& operator>>(std::istream& is, json& j)
{
	j = json::FromString(std::string(std::istreambuf_iterator<char>(is), {}));
	return is;
}



json::Parser::Parser(std::string json_str)
	: json_str(json_str), pos(0)
{
}
json json::Parser::Parse()
{
	skipSpaces();

	if (isdigit(json_str[pos]) || json_str[pos] == '-')
		return ReadNumber();
	else if (json_str[pos] == '"')
		return ReadString();
	else if (json_str[pos] == '[')
		return ReadList();
	else if (json_str[pos] == '{')
		return ReadDict();
	else if (json_str[pos] == 't' || json_str[pos] == 'f')
		return ReadBool();

	THROW_INVALID_JSON;
}
json json::Parser::ReadBool()
{
	std::string tmp;

	if (json_str[pos] == 't')
	{
		tmp = json_str.substr(pos, 4);
		pos += 4;
	}
	else if (json_str[pos] == 'f')
	{
		tmp = json_str.substr(pos, 5);
		pos += 5;
	}

	if (tmp == "true")
		return true;
	if (tmp == "false")
		return false;

	THROW_INVALID_JSON;
}
json json::Parser::ReadNumber()
{
	if (!isdigit(json_str[pos]) && json_str[pos] != '-')
		THROW_INVALID_JSON;

	std::string tmp;
	if (json_str[pos] == '-')
	{
		tmp += '-';
		pos += 1;
	}
	for (; isdigit(json_str[pos]) || json_str[pos] == '.'; tmp += json_str[pos++]);

	if (tmp.find('.') == std::string::npos)
		return stoll(tmp);
	return stod(tmp);
}
json json::Parser::ReadString()
{
	if (json_str[pos] != '"')
		THROW_INVALID_JSON;

	std::string tmp;
	for (pos += 1; json_str[pos] != '"'; tmp += json_str[pos++]);

	if (json_str[pos] != '"')
		THROW_INVALID_JSON;
	pos += 1;

	return tmp;
}
json json::Parser::ReadList()
{
	std::vector<json> list;

	if (json_str[pos] != '[')
		THROW_INVALID_JSON;
	pos += 1;

	skipSpaces();

	if (json_str[pos] != ']')
	{
		while (true)
		{
			skipSpaces();
			list.push_back(Parse());
			skipSpaces();

			if (json_str[pos] != ',')
				break;

			pos += 1;
		}
		skipSpaces();
	}

	if (json_str[pos] != ']')
		THROW_INVALID_JSON;
	pos += 1;

	return list;
}
json json::Parser::ReadDict()
{
	std::map<std::string, json> map;

	if (json_str[pos] != '{')
		THROW_INVALID_JSON;
	pos += 1;

	skipSpaces();

	if (json_str[pos] != ']')
	{
		while (true)
		{
			skipSpaces();

			std::string key = ReadString();

			skipSpaces();
			if (json_str[pos] != ':')
				THROW_INVALID_JSON;
			pos += 1;
			skipSpaces();
			map[key] = Parse();
			skipSpaces();

			if (json_str[pos] != ',')
				break;

			pos += 1;
		}
		skipSpaces();
	}

	if (json_str[pos] != '}')
		THROW_INVALID_JSON;
	pos += 1;

	return map;
}
inline void json::Parser::skipSpaces()
{
	while (isspace(json_str[pos])) pos++;
}
#endif
