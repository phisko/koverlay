# [Serializable](Serializable.hpp)

Mix-in class letting any derived class serialize its attributes with great simplicity.

By default, serializes objects as JSON. Can be easily customized with policies.

### Members

##### Constructor

```cpp
Serializable()
```

A default constructor is available if the CRTP class is [Reflecitble](Reflectible.md).

```cpp
Serializable(Fields &&...attrs)
```

Takes as arguments the various attributes which should be serialized, paired with their names (see `Example`).

##### serialize

```cpp
void serialize(std::ostream &s) const;
std::ostream &operator<<(std::ostream &s, const putils::Serializable<Derived, OutputPolicy> &obj)
```

##### unserialize

```cpp
void unserialize(std::istream &s) noexcept override
std::istream &operator>>(std::istream &s, putils::Serializable<Derived, OutputPolicy> &obj)
```

### Example

```cpp
int main()
{
    struct Tmp : public Serializable<Tmp>
    {
        Tmp(int x, int y)
        :
            Serializable(
                std::make_pair("x", &Tmp::_x),
                std::make_pair("y", &Tmp::_y)
            ), _x(x), _y(y)
        {}

        int _x, _y;
    };

    Tmp test(24, 42);
    std::stringstream s;

    std::cout << test << std::endl;
    s << test << std::endl;

    test._x = 5; test._y = 5;

    std::cout << test << std::endl;
    s >> test;
    std::cout << test << std::endl;
    
    class ReflectibleTest : public putils::Reflectible<ReflectibleTest>, public putils::Serializable<ReflectibleTest>
    {
    public:
        std::string hiString = "hi";
        int _fourtyTwo = 42;
        
        static const auto get_class_name() { return pmeta_nameof(ReflectibleTest); }
        static const auto &get_attributes()
        {
            static const auto table = pmeta::make_table(
                pmeta_reflectible_attribute(&ReflectibleTest::hiString),
                pmeta_reflectible_attribute_private(&ReflectibleTest::_fourtyTwo)
            );
            return table;
        }
        static void get_methods() {}
        static void get_parents() {}
    };
    
    std::cout << ReflectibleTest{} << std::endl;
}
```

