#include "JSON_CPP.hpp"

#pragma region value_t

value_t::operator std::string() const
{
    return std::get<std::string>(*this);
}
value_t::operator int() const
{
    return std::get<int>(*this);
}
value_t::operator double() const
{
    return std::get<double>(*this);
}
value_t::operator bool() const
{
    return std::get<double>(*this);
}
value_t::operator std::nullptr_t() const
{
    return std::get<nullptr_t>(*this);
}
value_t::operator object_t() const
{
    return std::get<object_t>(*this);
}
value_t::operator array_t() const
{
    return std::get<array_t>(*this);
}

void value_t::Print(std::ostream& os, bool pretify, const char* tabText, int tabNumber) const
{
    if (std::holds_alternative<std::string>(*this))
    {
        os << std::get<std::string>(*this);
    }
    else if (std::holds_alternative<int>(*this))
    {
        os << std::get<int>(*this);
    }
    else if (std::holds_alternative<double>(*this))
    {
        os << std::get<double>(*this);
    }
    else if (std::holds_alternative<bool>(*this))
    {
        os << (std::get<bool>(*this) ? "true" : "false");
    }
    else if (std::holds_alternative<std::nullptr_t>(*this))
    {
        os << "null";
    }
    else if (std::holds_alternative<array_t>(*this))
    {
        std::string tab;
        for (size_t i = 0; i < tabNumber; ++i)
            tab += tabText;

        array_t temp = std::get<array_t>(*this);
        if (pretify && temp.size() > 0)
            os << "[\n" << tab;
        else
            os << "[";

        for (int i = 0; i < temp.size(); i++)
        {
            bool isString = std::holds_alternative<std::string>(*(temp[i]));
            if (isString)
            {
                os << "\"";
                temp[i]->Print(os, pretify, tabText, tabNumber + 1);
                os << "\"";
            }
            else
            {
                temp[i]->Print(os, pretify, tabText, tabNumber + 1);
            }

            if (i != temp.size() - 1)
                os << ',';

            if (pretify)
            {
                os << '\n';
                if (i != temp.size() - 1)
                    os << tab;
            }

        }

        if (pretify && temp.size() > 0)
        {
            std::string endTab;
            for (size_t i = 0; i < tabNumber - 1; ++i)
                endTab += tabText;
            os << endTab;
        }
        os << "]";
    }
    else
    {
        std::string tab;
        for (size_t i = 0; i < tabNumber; ++i)
            tab += tabText;

        object_t temp = std::get<object_t>(*this);
        if (pretify && temp.size() > 0)
            os << "{\n" << tab;
        else
            os << "{";

        for (int i = 0; i < temp.size(); i++)
        {
            os << '\"' << temp[i].first << "\":";
            if (pretify) os << ' ';

            bool isString = std::holds_alternative<std::string>(*(temp[i].second));
            if (isString)
            {
                os << "\"";
                temp[i].second->Print(os, pretify, tabText, tabNumber + 1);
                os << "\"";
            }
            else
            {
                temp[i].second->Print(os, pretify, tabText, tabNumber + 1);
            }

            if (i != temp.size() - 1)
                os << ',';

            if (pretify)
            {
                os << '\n';
                if (i != temp.size() - 1)
                    os << tab;
            }
        }
        if (pretify && temp.size() > 0)
        {
            std::string endTab;
            for (size_t i = 0; i < tabNumber - 1; ++i)
                endTab += tabText;
            os << endTab;
        }
        os << "}";
    }
}

std::ostream& operator<<(std::ostream& os, const value_t& v)
{
    v.Print(os);

    return os;
}

value_t::Proxy value_t::operator[](const char* key)
{
    jpp_assert(std::holds_alternative<object_t>(*this), "[const char*] only works on object_t");
    return value_t::Proxy(*this,key);
}
value_t& value_t::operator[](int index)
{
    if (std::holds_alternative<array_t>(*this))
        return *(std::get<array_t>(*this).at(index));
    else if (std::holds_alternative<object_t>(*this))
        return *(std::get<object_t>(*this).at(index).second);
}

std::size_t value_t::Size() const
{
    if (std::holds_alternative<array_t>(*this))
    {
        return std::get<array_t>(*this).size();
    }
    else if (std::holds_alternative<object_t>(*this))
    {
        return std::get<object_t>(*this).size();
    }
    else
        return 1;
}

#pragma endregion
#pragma region value_t::Proxy

value_t::Proxy::Proxy(value_t& parent, const char*& key) : parent_(parent)
{
    keyHistory_.insert(keyHistory_.begin(), key);
}
value_t::Proxy::Proxy(Proxy& old, const char*& newKey) : parent_(old.parent_)
{
    keyHistory_ = old.keyHistory_;
    keyHistory_.insert(keyHistory_.begin(), newKey);
}

value_t& value_t::Proxy::operator = (const value_t& value)
{
    object_t& temp = std::get<object_t>(parent_);

    if (keyHistory_.size() != 1)
    {
        temp.push_back(NestedObjectCreator(keyHistory_));

        value_t& lastObject = GetLastObject(parent_);

        *(std::get<object_t>(lastObject).back().second) = value;
        return lastObject;
    }
    else
    {
        object_t::iterator it;

        it = std::find_if(temp.begin(), temp.end(),
            [this](const auto& pair) { return pair.first == keyHistory_.front(); });

        if (it == temp.end())
        {
            temp.push_back(std::make_pair(keyHistory_.front(), std::make_shared<value_t>(nullptr)));
            it = temp.end() - 1;
        }
        *(it->second) = value;
        return parent_;
    }
}

value_t::Proxy::operator value_t() const
{
    object_t& temp = std::get<object_t>(parent_);
    auto it = std::find_if(temp.begin(), temp.end(),
        [this](const auto& pair) { return pair.first == keyHistory_.front(); });

    if (it == temp.end())
        throw std::runtime_error(std::string("Key not found: ") + keyHistory_.front());

    return *(it->second);
}

value_t::Proxy value_t::Proxy::operator[](const char* key)
{
    object_t& temp = std::get<object_t>(parent_);
    auto it = std::find_if(temp.begin(), temp.end(),
        [this](const auto& pair) { return pair.first == keyHistory_.front(); });

    if (it == temp.end())
        return Proxy(*this, key);

    return Proxy(*(it->second), key);
}
value_t& value_t::Proxy::operator[](int index)
{
    object_t& temp = std::get<object_t>(parent_);
    object_t::iterator it = std::find_if(temp.begin(), temp.end(),
        [this](const auto& pair) { return pair.first == keyHistory_.front(); });

    if (it == temp.end())
        throw std::runtime_error(std::string("Key not found: ") + keyHistory_.front());

    if (std::holds_alternative<array_t>(*(it->second)))
        return *(std::get<array_t>(*(it->second)).at(index));
    else if (std::holds_alternative<object_t>(*(it->second)))
        return *(std::get<object_t>(*(it->second)).at(index).second);
}

std::pair<std::string, value_ptr> value_t::Proxy::NestedObjectCreator(std::vector<const char*> keyHistory)
{
    std::pair<std::string, value_ptr> result = std::make_pair(keyHistory.back(), std::make_shared<value_t>(nullptr));
    keyHistory.pop_back();
    if (keyHistory.size() != 0)
    {
        object_t obj;
        obj.push_back(NestedObjectCreator(keyHistory));
        *(result.second) = obj;
    }

    return result;
}

value_t& value_t::Proxy::GetLastObject(value_t& obj)
{
    object_t& temp = std::get<object_t>(obj);
    if (std::holds_alternative<object_t>(*(temp.back().second)))
        return GetLastObject(*(temp.back().second));
    else
        return obj;
}

#pragma endregion
#pragma region JSON
bool JSON::IsObjectLike(std::initializer_list<JSON> init)
{
    if (init.size() < 2 && (init.size() & 1) != 0) return false;
    for (int i = 0; i < init.size(); i += 2)
    {
        const value_t& suposedKey = (*(init.begin() + i)).value_;
        if (!std::holds_alternative<std::string>(suposedKey)) return false;
    }
    return true;
}

std::istream& JSON::SkipWhitespace(std::istream& in)
{
    while (std::isspace(in.peek())) in.get();
    return in;
}

std::string JSON::ParseString(std::istream& in)
{
    std::string result;
    char ch;
    if (in.get() != '"') throw std::runtime_error("Expected '\"'");
    while (in.get(ch)) {
        if (ch == '"') break;
        if (ch == '\\') {
            char esc;
            in.get(esc);
            switch (esc) {
            case '"': result += '"'; break;
            case '\\': result += '\\'; break;
            case 'n': result += '\n'; break;
            default: throw std::runtime_error("Unknown escape");
            }
        }
        else {
            result += ch;
        }
    }
    return result;
}

value_t JSON::ParseArray(std::istream& in)
{
    if (in.get() != '[') throw std::runtime_error("Expected '['");

    array_t arr;
    SkipWhitespace(in);

    if (in.peek() == ']')
    {
        in.get();
        return arr;
    }

    while (true) {
        SkipWhitespace(in);
        arr.push_back(std::make_shared<value_t>(ParseValue(in)));
        SkipWhitespace(in);
        char ch = in.get();
        if (ch == ']') break;
        if (ch != ',') throw std::runtime_error("Expected ',' or ']'");
    }

    return arr;
}

value_t JSON::ParseObject(std::istream& in) {
    if (in.get() != '{') throw std::runtime_error("Expected '{'");

    object_t obj;
    SkipWhitespace(in);

    if (in.peek() == '}') { in.get(); return obj; }

    while (true) {
        SkipWhitespace(in);
        std::string key = ParseString(in);
        SkipWhitespace(in);
        if (in.get() != ':') throw std::runtime_error("Expected ':'");
        SkipWhitespace(in);
        obj.push_back(std::make_pair(key, std::make_shared<value_t>(ParseValue(in))));
        SkipWhitespace(in);
        char ch = in.get();
        if (ch == '}') break;
        if (ch != ',') throw std::runtime_error("Expected ',' or '}'");
    }

    return obj;
}

value_t JSON::ParseLiteral(std::istream& in, const std::string& literal, value_t value) {
    for (char c : literal) {
        if (in.get() != c) throw std::runtime_error("Invalid literal");
    }
    return value;
}

value_t JSON::ParseNumber(std::istream& in) {
    std::string numStr;
    bool isFloating = false;
    while (std::isdigit(in.peek()) || in.peek() == '.' || in.peek() == '-' || in.peek() == '+' || in.peek() == 'e' || in.peek() == 'E') {
        if (in.peek() == '.' || in.peek() == 'e' || in.peek() == 'E') isFloating = true;
        numStr += in.get();
    }
    if (isFloating)
        return std::stod(numStr);
    return std::stoi(numStr);
}

value_t JSON::ParseValue(std::istream& in) {
    SkipWhitespace(in);
    char ch = in.peek();
    if (ch == '"') return ParseString(in);
    if (ch == '{') return ParseObject(in);
    if (ch == '[') return ParseArray(in);
    if (std::isdigit(ch) || ch == '-' || ch == '+') return ParseNumber(in);
    if (in.peek() == 't') return ParseLiteral(in, "true", true);
    if (in.peek() == 'f') return ParseLiteral(in, "false", false);
    if (in.peek() == 'n') return ParseLiteral(in, "null", nullptr);
    throw std::runtime_error("Unknown JSON value");
}

JSON::JSON() : value_(object()) {}

JSON::JSON(bool _v) : value_(_v) {}

JSON::JSON(int _v) : value_(_v) {}

JSON::JSON(double _v) : value_(_v) {}

JSON::JSON(const char* _v) : value_(_v) {}

JSON::JSON(array_t _v) : value_(_v) {}

JSON::JSON(object_t _v) : value_(_v) {}

JSON::JSON(nullptr_t _v) : value_(_v) {}

JSON::JSON(std::initializer_list<JSON> init)
{
    if (IsObjectLike(init))
    {
        object_t object;
        for (int i = 0; i < init.size(); i += 2)
        {
            std::string key = std::get<std::string>((*(init.begin() + i)).value_);

            for (std::pair<std::string, value_ptr> data : object)
                jpp_assert(data.first != key, "a key is already in used in an object.");

            object.push_back(std::make_pair(key,
                std::make_shared<value_t>((*(init.begin() + i + 1)).value_)));
        }
        value_ = object;
    }
    else
    {
        //I may want here to put a deprecated or a waring signe
        array_t arr;
        for (JSON v : init)
        {
            arr.push_back(std::make_shared<value_t>(v.value_));
        }
        value_ = arr;
    }
}

value_t::Proxy JSON::operator [] (const char* key)
{
    jpp_assert(std::holds_alternative<object_t>(value_), "[const char*] only works on object_t");

    return value_t::Proxy(value_, key);
}

value_t& JSON::operator [] (int index)
{
    if (std::holds_alternative<array_t>(value_))
        return *(std::get<array_t>(value_).at(index));
    else if (std::holds_alternative<object_t>(value_))
        return *(std::get<object_t>(value_).at(index).second);
}

value_t JSON::Get()
{
    return value_;
}


array_t JSON::array()
{
    array_t arr;
    return arr;
}


object_t JSON::object()
{
    object_t obj;
    return obj;
}

void JSON::Write(const char* path, bool pretify)
{
    std::filesystem::path filePath(path);
    std::filesystem::create_directories(filePath.parent_path());

    std::ofstream jsonFile;
    jsonFile.open(path);

    value_.Print(jsonFile, pretify);

    jsonFile.close();
}
void JSON::Read(const char* path)
{
    std::ifstream jsonFile(path);
    if (!jsonFile.is_open()) {
        std::cerr << "Failed to open file: " << path << '\n';
        return;
    }

    value_ = ParseValue(jsonFile);

    jsonFile.close();
}
#pragma endregion