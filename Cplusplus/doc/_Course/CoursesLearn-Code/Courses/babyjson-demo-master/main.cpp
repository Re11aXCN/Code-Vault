#include <variant>
#include <vector>
#include <unordered_map>
#include <string>
#include <string_view>
#include <optional>
#include <regex>
#include <charconv>
#include "print.h"

struct JSONObject;

using JSONDict = std::unordered_map<std::string, JSONObject>;
using JSONList = std::vector<JSONObject>;

struct JSONObject {
	std::variant
		< std::nullptr_t  // null
		, bool            // true
		, int             // 42
		, double          // 3.14
		, std::string     // "hello"
		, JSONList        // [42, "hello"]
		, JSONDict        // {"hello": 985, "world": 211}
		> inner;

	void do_print() const {
		printnl(inner);
	}

	template <class T>
	bool is() const {
		return std::holds_alternative<T>(inner);
	}

	template <class T>
	T const& get() const {
		return std::get<T>(inner);
	}

	template <class T>
	T& get() {
		return std::get<T>(inner);
	}
};

template <class T>
std::optional<T> try_parse_num(std::string_view str) {
	T value;
	auto res = std::from_chars(str.data(), str.data() + str.size(), value);
	if (res.ec == std::errc() && res.ptr == str.data() + str.size()) {
		return value;
	}
	return std::nullopt;
}

char unescaped_char(char c) {
	switch (c) {
	case 'n': return '\n';
	case 'r': return '\r';
	case '0': return '\0';
	case 't': return '\t';
	case 'v': return '\v';
	case 'f': return '\f';
	case 'b': return '\b';
	case 'a': return '\a';
	default: return c;
	}
}

std::pair<JSONObject, size_t> parse(std::string_view json) {
	if (json.empty()) {
		return { JSONObject{std::nullptr_t{}}, 0 };
	}
	else if (size_t off = json.find_first_not_of(" \n\r\t\v\f\0"); off != 0 && off != json.npos) {
		auto [obj, eaten] = parse(json.substr(off));
		return { std::move(obj), eaten + off };
	}
	else if ('0' <= json[0] && json[0] <= '9' || json[0] == '+' || json[0] == '-') {
		std::regex num_re{ "[+-]?[0-9]+(\\.[0-9]*)?([eE][+-]?[0-9]+)?" };
		std::cmatch match;
		if (std::regex_search(json.data(), json.data() + json.size(), match, num_re)) {
			std::string str = match.str();
			if (auto num = try_parse_num<int>(str)) {
				return { JSONObject{*num}, str.size() };
			}
			//if (auto num = try_parse_num<double>(str); num.has_value() ) {
			//    return { JSONObject{num.value()}, str.size()};
			//}
			if (auto num = try_parse_num<double>(str)) {
				return { JSONObject{*num}, str.size() };
			}
		}
	}
	else if (json[0] == '"') {
		std::string str;
		enum {
			Raw,
			Escaped,
		} phase = Raw;
		size_t i;
		for (i = 1; i < json.size(); i++) {
			char ch = json[i];
			if (phase == Raw) {
				if (ch == '\\') {
					phase = Escaped;
				}
				else if (ch == '"') {
					i += 1;
					break;
				}
				else {
					str += ch;
				}
			}
			else if (phase == Escaped) {
				str += unescaped_char(ch);
				phase = Raw;
			}
		}
		return { JSONObject{std::move(str)}, i };
	}
	else if (json[0] == '[') {
		std::vector<JSONObject> res;
		size_t i;
		for (i = 1; i < json.size();) {
			if (json[i] == ']') {
				i += 1;
				break;
			}
			auto [obj, eaten] = parse(json.substr(i));
			if (eaten == 0) {
				i = 0;
				break;
			}
			res.push_back(std::move(obj));
			i += eaten;
			if (json[i] == ',') {
				i += 1;
			}
		}
		return { JSONObject{std::move(res)}, i };
	}
	else if (json[0] == '{') {
		std::unordered_map<std::string, JSONObject> res;
		size_t i;
		for (i = 1; i < json.size();) {
			if (json[i] == '}') {
				i += 1;
				break;
			}
			auto [keyobj, keyeaten] = parse(json.substr(i));
			if (keyeaten == 0) {
				i = 0;
				break;
			}
			i += keyeaten;
			if (!std::holds_alternative<std::string>(keyobj.inner)) {
				i = 0;
				break;
			}
			if (json[i] == ':') {
				i += 1;
			}
			std::string key = std::move(std::get<std::string>(keyobj.inner));
			auto [valobj, valeaten] = parse(json.substr(i));
			if (valeaten == 0) {
				i = 0;
				break;
			}
			i += valeaten;
			res.try_emplace(std::move(key), std::move(valobj));
			if (json[i] == ',') {
				i += 1;
			}
		}
		return { JSONObject{std::move(res)}, i };
	}
	return { JSONObject{std::nullptr_t{}}, 0 };
}

template <class ...Fs>
struct overloaded : Fs... {
	using Fs::operator()...;
};

template <class ...Fs>
overloaded(Fs...) -> overloaded<Fs...>;
// 回家作业实现 std::nullptr_t  // null  , bool // true
// 要进行详细测试

int main() {
	// R"JSON({"work":996,"school":[985,211],"school":"trashgench"})JSON";
	// R"JSON("HELLO\n\WORLD",0,1)JSON";
	std::string_view str = R"JSON(985.211)JSON";
	std::string_view str = R"JSON({
	"work": 996,
	"shcool": [985, [211, 996]],
	})JSON";
	auto [obj, eaten] = parse(str);
	print(obj);
	auto const& dict = obj.get<JSONDict>();
	print("The capitalist make me work in", dict.at("work").get<int>());
	auto const& school = dict.at("school");
	auto dovisit = [&](auto& dovisit, JSONObject const& school)->void {
		std::visit([&](auto const& school) {
			if constexpr (std::is_same_v<std::decay_t<decltype(school)>, JSONList>)
				for (auto const& subschool : school) {
					dovisit(dovisit, subschool);
				}
			else {
				print("I purchased my diploma from", school);
			}
			}, school.inner);
		};
	dovisit(dovisit, school);
	std::visit(
		overloaded{
			[&](int val) {
				print("int is:", val);
			},
			[&](double val) {
				print("double is:", val);
			},
			[&](std::string val) {
				print("string is:", val);
			},
			[&](auto val) {
				print("unknown object is:", val);
			},
		},
		obj.inner);
	return 0;
}
// c++规定编译器编译阶段不能够出现未定义行为，所以我们可以给函数和返回给的变量加上constexpr关键字
// 左移右移都不允许是负数 << (移动范围时0~31)

/*
constexpr int func(int i)
{
	if(i >= 32 || i<0) return 0
	return 1 << i
}
constexpr int _2 = func(31)
printf("%d\n", _2); // -2147483648
*/

// x86是小端，return *char(*)a不行指针不能够乱转为了跨平台
// 无符号的溢出会回环
// 迭代器必须用的时候在获取，不能够一致获取


// class struct成员变量最好初始化
// 构造函数可以不写我们可以通过这样

/*

struct C
{
	size_t age = 45;
	std::string name = "hello"s; // 在字符串加上s表示str，要不然会发生char*转string，使用是需要using namespace std::literals;
}
main()
{
	auto c = C{.name = "name"}; //c++20可以单独初始化某个成员变量
	c = C{"1","world"};
}
*/

/*
函数参数用结构体写

struct lerp
{
	float from = 0;
	float to = 1;
	float fac = 0.5f;
	struct result{
		float value;
		float secondval;
	}
	result operator()() const // 仿函数
	{
		return { .value = fac * (to - from) + from;
				.secondval = fac * (from - to) + to}
	}
}

auto [x, y] = lerp{.from = 0, .to = 1, .fac = 0.25}(); // []表示解包
*/