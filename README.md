# JSON_CPP
JSON_CPP is a very small library to read and write JSON in C++.

## Information:
JSON_CPP supports all the basic requirements of the JSON file format. 

`The support of surrogate pair is still in developement.`

## Requirement:

- You need [![C++](https://img.shields.io/badge/v20-blue?style=flat&logo=C%2B%2B&label=C%2B%2B)](https://isocpp.org/) to use the library.

- (optional) if you use utf char in your JSON file you will have to set the terminal format.

## How to use:
The library is easy to use; first include the library in your project:

`#include "JSON_CPP.hpp"`

Then you can use the `JSON` class to start using JSON files in cpp.
### Create a JSON in c++
To create a JSON you have to make a chain of keys and values:
```cpp
#include "JSON_CPP.hpp"
int main()
{
    JSON file = {
        "valueA", 1,
        "valueB", "two",
        "valueC", true,
        "valueD", nullptr,
        "valueE", {
            "valueA", "foo",
            "valueB", "bar"
        }
    };
    return 0;
}
```
It corresponds to that JSON file:
```json
{
    "valueA": 1,
    "valueB": "two",
    "valueC": true,
    "valueD": null,
    "valueE": {
        "valueA": "foo",
        "valueB": "bar"
    }
}
```
### Objects
The use of objects may be confusing. There are 2 ways to make objects; the first is within the `JSON` like that:
```cpp
#include "JSON_CPP.hpp"
int main()
{
    JSON file = {
        "foo", {
            "bar", {}
        }
    };
    return 0;
}
```
The other is to use `JSON::object()`:
```cpp
#include "JSON_CPP.hpp"
int main()
{
    JSON file = JSON::object(
        "foo", JSON::object(
            "bar", JSON::object()
        )
    );
    return 0;
}
```
`JSON::object()` is faster and is used in arrays.
#### Note:
Once you use `JSON::object()` you can't use `{}` inside.

### Arrays
Arrays in JSON are a bit special as they can contain any type of compatible value.
To create an Array you need to use `JSON::array()` like that:
```cpp
#include "JSON_CPP.hpp"
int main()
{
    JSON file = JSON::array(
        "a string",
        8,
        true,
        3.14,
        JSON::object(
            "bar", nullptr
        ),
        JSON::array()
    );
    return 0;
}
```
It corresponds to that JSON file:
```json
[
    "a string",
    8,
    true,
    3.14,
    {
        "bar": null
    },
    []
]
```
### Read
```cpp
#include "JSON_CPP.hpp"
int main()
{
    JSON file;
    file.Read("path/to/the/file.json")
    return 0;
}
```
### Write
```cpp
#include "JSON_CPP.hpp"
int main()
{
    JSON file = {
        "foo", "bar"
        "number", 8
    };
    file.Write("path/to/save/file.json", true)
    return 0;
}
```
Output:
```json
{
    "foo": "bar",
    "number": 8
}
```

### Print
To print a JSON in the console, you can do:
```cpp
std::cout << file.Get() << std::endl;
```
It will print the JSON as it is and won't be prettify.
You can also do this:
```cpp
file.Get().Print(std::cout, true);
```
Doing that lets you print in an `std::ostream` and will let you choose between prettify or not.