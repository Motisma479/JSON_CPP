#include <iostream>
#include <variant>
#include <cassert>
#include <fstream>
#include <filesystem>

#define jpp_assert(expr, msg) assert(( (void)(msg), (expr) ))

struct value_t;
using value_ptr = std::shared_ptr<value_t>;
using object_t = std::vector<std::pair<std::string, value_ptr>>;
using array_t = std::vector<value_ptr>;

struct value_t : std::variant<
    std::string,
    int,
    double,
    bool,
    std::nullptr_t,
    object_t,
    array_t
> {
    using variant::variant; // inherit constructors

    //auto cast
    operator std::string() const;
    operator int() const;
    operator double() const;
    operator bool() const;
    operator std::nullptr_t() const;
    operator object_t() const;
    operator array_t() const;

    //cout stream
    void Print(std::ostream& os, bool pretify = false, const char* tabText = "    ", int tabNumber = 1) const;
    
    friend std::ostream& operator<<(std::ostream& os, const value_t& v);

    class Proxy //A proxy class to manage the access by key
    {
    public:
        Proxy(value_t& parent, const char*& key);
        Proxy(Proxy& old, const char*& newKey);

        value_t& operator = (const value_t& value);

        operator value_t() const;

        Proxy operator[](const char* key);
        value_t& operator[](int index);

    private:
        value_t& parent_;
        std::vector<const char*> keyHistory_ = {}; //this should be at 1 unless we are in an unexisting object;
    
        std::pair<std::string, value_ptr> NestedObjectCreator(std::vector<const char*> keyHistory);

        value_t& GetLastObject(value_t& obj);
    };

    value_t::Proxy operator[](const char* key);
    value_t& operator[](int index);

    std::size_t Size() const;
};

class JSON
{
private:
    value_t value_;

    bool IsObjectLike(std::initializer_list<JSON> init);

#pragma region ParseFunction
    std::istream& SkipWhitespace(std::istream& in);

    std::string ParseString(std::istream& in);

    value_t ParseArray(std::istream& in);

    value_t ParseObject(std::istream& in);

    value_t ParseLiteral(std::istream& in, const std::string& literal, value_t value);

    value_t ParseNumber(std::istream& in);

    value_t ParseValue(std::istream& in);
#pragma endregion

public:

#pragma region constructors
    JSON();
    JSON(bool _v);
    JSON(int _v);
    JSON(double _v);
    JSON(const char* _v);
    JSON(array_t _v);
    JSON(object_t _v);
    JSON(nullptr_t _v);
    JSON(std::initializer_list<JSON> init);
#pragma endregion

    value_t::Proxy operator [] (const char* key);
    value_t& operator [] (int index);
    
    value_t Get();

#pragma region Utility
    template<typename... Args>
    inline static array_t array(const Args&... args)
    {
        jpp_assert((std::is_constructible_v<value_t, Args> && ...), "All arguments must be value_t");

        array_t arr;

        (arr.push_back(std::make_shared<value_t>((args))), ...);

        return arr;
    }
    static array_t array();

    template<typename... Args>
    inline static object_t object(const Args&... args)
    {
        jpp_assert((std::is_constructible_v<value_t, Args> && ...), "All arguments must be value_t");

        object_t obj;

        std::vector<value_t> vec;
        ((vec.push_back((args))), ...);

        for (int i = 0; i < vec.size(); i += 2)
        {
            std::string key = std::get<std::string>(vec[i]);

            for (std::pair<std::string, value_ptr> data : obj)
                jpp_assert(data.first != key, "a key is already in used in an object.");

            obj.push_back(std::make_pair(key, std::make_shared<value_t>(vec[i + 1])));
        }
        return obj;
    }
    static object_t object();

    void Write(const char* path, bool pretify = true);
    void Read(const char* path);
#pragma endregion
};