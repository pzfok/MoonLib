#include <chrono>
#include <iostream>
#include <typeinfo>
#include <memory>
#include <thread>
#include <map>
#include <initializer_list>

#include <cxxabi.h>
#ifndef __CYGWIN__
#include <execinfo.h>
#endif

#include "gtest/gtest.h"

#include <CppString.h>

#include "global.h"

using namespace std;
using namespace std::chrono;

TEST(Cpp11, chrono)
{
    duration<int32_t> d(50);
    EXPECT_EQ(50, d.count());
}

TEST(Cpp11, std_move)
{
    string a = "a";
    string b = "b";
    a = std::move(b);

    EXPECT_EQ("a", b);
    EXPECT_EQ("b", a);
}

/* 线程局部变量 */
thread_local uint32_t thread_param = 0;
TEST(Cpp11, thread)
{
    EXPECT_EQ(0u, thread_param);
}

class ClassName
{
public:
    string GetClassName()
    {
        const type_info &class_info = typeid(*this);
        int status;

        char *prealname = abi::__cxa_demangle(class_info.name(), 0, 0, &status);

        string name(prealname);
        free(prealname);

        return name;
    }

    static void fun()
    {
        cout << 1 << endl;
    }
};

void foo(void)
{
    printf("foo\n");
}

TEST(Cpp11, typeid)
{
    ClassName class_name;
    EXPECT_EQ("ClassName", class_name.GetClassName());
}

struct BindFuncTest : public binary_function < int, int, bool >
{
    bool operator()(int a, int b) const
    {
        return a < b;
    }
};

/* bind引用测试 */
class RefParam
{
public:
    static uint32_t Count;

    RefParam()
    {
        ++Count;
        DEBUG_LOG("[%s]Count[%u].", __FUNCTION__, Count);
    }

    RefParam(const RefParam &refParam)
    {
        static_cast<void>(refParam);
        ++Count;
        DEBUG_LOG("[%s]Count[%u].", __FUNCTION__, Count);
    }

    RefParam(RefParam &&refParam)
    {
        static_cast<void>(refParam);
        ++Count;
        DEBUG_LOG("[%s]Count[%u].", __FUNCTION__, Count);
    }

    ~RefParam()
    {
        --Count;
        DEBUG_LOG("[%s]Count[%u].", __FUNCTION__, Count);
    }
};

uint32_t RefParam::Count;

void FuncParam(RefParam refParam)
{
    static_cast<void>(refParam);
}

void FuncRefParam(RefParam &refParam)
{
    static_cast<void>(refParam);
}

void FuncRightRefParam(RefParam &&refParam)
{
    static_cast<void>(refParam);
}

TEST(Cpp11, bind)
{
    BindFuncTest test;
    EXPECT_FALSE(bind1st(test, 8)(3));          // 8 < 3
    EXPECT_TRUE(bind2nd(test, 8)(3));           // 3 < 8

    EXPECT_FALSE(bind1st(less<int>(), 8)(3));   // 8 < 3
    EXPECT_TRUE(bind2nd(less<int>(), 8)(3));    // 3 < 8

    EXPECT_EQ(0u, RefParam::Count);
    RefParam refParam;
    EXPECT_EQ(1u, RefParam::Count);
    auto funcParam = bind(FuncParam, refParam);
    EXPECT_EQ(2u, RefParam::Count);
    funcParam();
    auto funcParamWithRef = bind(FuncParam, ref(refParam));
    funcParamWithRef();
    EXPECT_EQ(2u, RefParam::Count);
}

class InitializeInClass
{
public:
    int32_t a = 1;
    int32_t b{3};
    static const int32_t aa = 3;

    InitializeInClass(int32_t inA) :a(inA)
    {

    }
};

TEST(Cpp11, initializeInClass)
{
    InitializeInClass c(4);
    EXPECT_EQ(4, c.a);
    EXPECT_EQ(3, c.b);
}

// 类型别名
using Class1 = ClassName;

class Base
{
public:
    Base()
    {
    }

    virtual void fun() final
    {
    }

    virtual void fun2()
    {
    }

    virtual ~Base()
    {
    }
};

class Derived :public Base
{
public:
    virtual void fun2() override
    {
    }
};

class LiteralConst
{
public:
    int32_t a;
};

#ifndef __CYGWIN__
LiteralConst operator "" _C(const char *col, size_t n)
{
    static_cast<void>(n);
    LiteralConst literalConst;
    literalConst.a = CppString::FromString<int32_t>(col);
    return literalConst;
}

// 字面值常量
TEST(Cpp11, LiteralConst)
{
    EXPECT_EQ(123, ("123"_C).a);
}
#endif

template < unsigned N >
struct Fib
{
    enum
    {
        Val = Fib<N - 1>::Val + Fib<N - 2>::Val
    };
};

template<>    //针对和的特化作为结束的条件
struct Fib<0>
{
    enum
    {
        Val = 0
    };
};

template<>
struct Fib<1>
{
    enum
    {
        Val = 1
    };
};

TEST(Cpp11, meta)
{
    EXPECT_EQ(55, Fib<10>::Val);
}

TEST(Cpp11, shared_ptr_and_weak_ptr)
{
    shared_ptr<int> pInt = make_shared<int>(3);
    EXPECT_EQ(3, *pInt);
    EXPECT_EQ(3, *pInt.get());

    // reset接口：替换管理的指针
    pInt.reset(new int(5));
    EXPECT_EQ(5, *pInt);

    // use_count接口：判断有多少个引用
    // unique接口：判断是否是唯一拥有此内存的shared_ptr，比use_count快一点
    EXPECT_EQ(1, pInt.use_count());
    EXPECT_TRUE(pInt.unique());

    // 增加一个shared_ptr
    shared_ptr<int> pInt2(pInt);
    EXPECT_EQ(2, pInt.use_count());
    EXPECT_EQ(2, pInt2.use_count());
    EXPECT_FALSE(pInt.unique());
    EXPECT_FALSE(pInt2.unique());

    // owner_before：用于比较大小，相当于"<"符号，内部相同的shared_ptr进行比较结果为false
    EXPECT_FALSE(pInt.owner_before(pInt2));
    EXPECT_FALSE(pInt2.owner_before(pInt));

    // weak_ptr：弱引用，不计数，可用于观察
    weak_ptr<int> pWeakInt(pInt);

    // 弱引用不影响计数
    EXPECT_EQ(2, pInt.use_count());
    EXPECT_FALSE(pInt.unique());

    // use_count：weak_ptr也有
    EXPECT_EQ(2, pWeakInt.use_count());

    // expired：检查是否数据已经不存在了
    EXPECT_FALSE(pWeakInt.expired());

    // lock：创建一个shared_ptr出来，如果失败，则内部为NULL
    shared_ptr<int> pInt3(pWeakInt.lock());
    EXPECT_TRUE(pInt3 != NULL);             // lock后判空不可少，否则可能会产生空指针访问
    EXPECT_EQ(3, pWeakInt.use_count());

    // 全部释放
    pInt.reset();
    pInt2.reset();
    pInt3.reset();

    EXPECT_EQ(0, pWeakInt.use_count());
    EXPECT_TRUE(pWeakInt.expired());

    shared_ptr<int> pInt4 = pWeakInt.lock();
    EXPECT_FALSE(pInt3 != NULL);
}

// 测试完美转发
template<typename T>
struct TypeName
{
    static const char *get()
    {
        return "Type";
    }
};

template<typename T>
struct TypeName<const T>
{
    static const char *get()
    {
        return "const Type";
    }
};

template<typename T>
struct TypeName<T&>
{
    static const char *get()
    {
        return "Type&";
    }
};

template<typename T>
struct TypeName<const T&>
{
    static const char *get()
    {
        return "const Type&";
    }
};

template<typename T>
struct TypeName<T&&>
{
    static const char *get()
    {
        return "Type&&";
    }
};

template<typename T>
struct TypeName<const T&&>
{
    static const char *get()
    {
        return "const Type&&";
    }
};

template < typename T>
const char * printValType(T &&val)
{
    static_cast<void>(val);
    return TypeName<T&&>::get();
}

class A
{
};

// 左值
A &lRefA()
{
    static A a;
    return a;
}

// 常左值
const A& clRefA()
{
    static A a;
    return a;
}

// 右值
A rRefA()
{
    return A();
}

// 常右值
const A crRefA()
{
    return A();
}

template<typename T>
const char * funcWithRightValue(T &&a)
{
    return printValType(a);
}

template<typename T>
const char * funcWithRightValueWithForward(T &&a)
{
    return printValType(forward<T>(a));
}

TEST(Cpp11, forward)
{
    // 正常情况
    EXPECT_EQ("Type&", printValType(lRefA()));
    EXPECT_EQ("const Type&", printValType(clRefA()));
    EXPECT_EQ("Type&&", printValType(rRefA()));
    EXPECT_EQ("const Type&&", printValType(crRefA()));

    // 传入函数，右值会退化为左值
    EXPECT_EQ("Type&", funcWithRightValue(lRefA()));
    EXPECT_EQ("const Type&", funcWithRightValue(clRefA()));
    EXPECT_EQ("Type&", funcWithRightValue(rRefA()));            // 应该是Type&&
    EXPECT_EQ("const Type&", funcWithRightValue(crRefA()));     // 应该是const Type&&

    // 带forward的作用，完美转发
    EXPECT_EQ("Type&", funcWithRightValueWithForward(lRefA()));
    EXPECT_EQ("const Type&", funcWithRightValueWithForward(clRefA()));
    EXPECT_EQ("Type&&", funcWithRightValueWithForward(rRefA()));
    EXPECT_EQ("const Type&&", funcWithRightValueWithForward(crRefA()));
}

// 初始化表达式
TEST(Cpp11, InitExpr)
{
    double doubleValue = 3.1415926;

    // 编译器会告警，精度丢失
    // warning: narrowing conversion of ‘doubleValue’ from ‘double’ to ‘int32_t {aka int}’ inside { } [-Wnarrowing]
    int32_t int32Value = {doubleValue};
    int32_t int32Value2{doubleValue};
    int32_t int32Value3 = doubleValue;
    int32_t int32Value4(doubleValue);

    EXPECT_EQ(3, int32Value);
    EXPECT_EQ(3, int32Value2);
    EXPECT_EQ(3, int32Value3);
    EXPECT_EQ(3, int32Value4);
}

// 常量表达式
constexpr int GetConstExpr()
{
    return 888;
}

TEST(Cpp11, constexpr)
{
    constexpr int constValue = GetConstExpr();
    EXPECT_EQ(GetConstExpr(), constValue);
}

// 类型别名声明
using MyDouble = double;
TEST(Cpp11, TypeDeclaration)
{
    MyDouble doubleValue = 1.3;
    EXPECT_DOUBLE_EQ(1.3, doubleValue);
}

// 获得表达式的类型：decltype
// 用于不计算表达式的值，在编译期获得表达式的类型
double AbortFunc()
{
    throw "";
    return 0.1;
}

TEST(Cpp11, decltype)
{
    // 此处不会调用函数AbortFunc()，所以也不会抛出异常
    decltype(AbortFunc() + 123 * 123.0) constValue = 123.0;
    EXPECT_DOUBLE_EQ(123.0, constValue);

    const int32_t aConstValue = 1;

    // 使用auto定义aAutoValue，由于是一个非常量，可以更改它的值
    {
        auto aAutoValue = aConstValue;
        aAutoValue = 2;
        EXPECT_EQ(2, aAutoValue);
    }

    // 使用decltype定义的，会保存const属性，所以不能修改值
    {
        decltype(aConstValue) aDecltypeValue = aConstValue;
        // error: assignment of read-only variable ‘aDecltypeValue’
        // aDecltypeValue = 2;
        EXPECT_EQ(1, aDecltypeValue);
    }

    // decltype会保留引用，所以decltype内部是引用的话，必须要在初始化的时候赋值
    {
        const int32_t &aRefValue = aConstValue;
        // error: ‘aRefValue2’ declared as reference but not initialized
        // decltype(aRefValue) aRefValue2;
        EXPECT_EQ(1, aRefValue);
    }

    // 一个重要的特性是，使用decltype，如果需要定义一个类型的引用，可以用括号把表达式包起来：
    {
        int32_t aValue = 1;
        decltype((aValue)) aRefValue = aValue;

        // 这是常规方式定义表达式类型的引用
        // 那么问题来了，既然可以这样定义，那么上面的语法的作用是什么呢？
        // 解答：括号括起来的，编译器将其当做表达式，表达式可以是左值，因此认为它是引用
        decltype(aValue) &aRefValue2 = aValue;

        // 修改引用会修改引用的对象aValue
        aRefValue = 2;
        EXPECT_EQ(2, aValue);
        EXPECT_EQ(2, aRefValue2);
    }
}

// 自定义class，实现begin和end接口，就可以使用返回for循环了
class MyForClass
{
public:
    int *begin()
    {
        return arr;
    }

    int *end()
    {
        return arr + sizeof(arr) / sizeof(arr[0]);
    }
private:
    int arr[10];
};

// for循环遍历
TEST(Cpp11, forLoop)
{
    string str = "12345";

    // 这样是值的拷贝，修改c不会修改原始数据
    for (auto c : str)
    {
        c = '1';

        // 这里需要用一下c，否则会告警
        static_cast<void>(c);
    }
    EXPECT_EQ("12345", str);

    // 使用引用，则修改c会改变原始值
    for (auto &c : str)
    {
        c = '1';
    }

    EXPECT_EQ("11111", str);

    // 内置数组也可以使用for循环遍历
    char strArr[] = "12345";
    for (auto c : strArr)
    {
        // 数组的长度是6，这样遍历会执行6次，最后的结束符也会在这里出现
        if (c != '\0')
        {
            c = '1';
        }
    }
    EXPECT_TRUE(strcmp("12345", strArr) == 0);

    for (auto &c : strArr)
    {
        // 数组的长度是6，这样遍历会执行6次，最后的结束符也会在这里出现
        if (c != '\0')
        {
            c = '1';
        }
    }

    EXPECT_TRUE(strcmp("11111", strArr) == 0);

    // 多维数组的遍历
    char a[10][10];
    for (auto &row : a)
    {
        for (auto &col : row)
        {
            col = '1';
        }
    }

    // 如果需要用到下一层的for循环，必须要使用引用，否则会将row转换为指针，而不是数组类型
    // 于是无法调用begin和end函数
    //     for (auto row : a)
    //     {
    //         // error: ‘begin’ was not declared in this scope
    //         // error: ‘end’ was not declared in this scope
    //         for (auto col : row)
    //         {
    //             col = '1';
    //         }
    //     }
}

// const迭代器
// 为了搭配auto，简化使用，增加了cbegin和cend接口。
TEST(Cpp11, ConstIterator)
{
    vector<uint32_t> vecNum{1,2,3,4,5,6};
    for (auto cIt = vecNum.cbegin(); cIt != vecNum.cend(); ++cIt)
    {
        static_cast<void>(cIt);
    }
}

// 内置数组指针迭代器
TEST(Cpp11, ArrayIterator)
{
    // begin和end函数可以用于内置数组
    char strArr[] = "12345";
    for (auto it = begin(strArr); it != end(strArr); ++it)
    {
        if (*it != '\0')
        {
            *it = '1';
        }
    }

    EXPECT_TRUE(strcmp("11111", strArr) == 0);

    // 数组下标可以是负数
    char *pStr = &strArr[2];
    pStr[-2] = '0';
    EXPECT_TRUE(strcmp("01111", strArr) == 0);

    MyForClass myForClass;
    int32_t count = 0;
    for (auto it : myForClass)
    {
        static_cast<void>(it);
        ++count;
    }
    EXPECT_EQ(10, count);
}

TEST(Cpp11, sizeof)
{
    // sizeof不会执行表达式，所以可以对无效的指针解引用获得其对象的大小
    // 再深入一点，这玩意是编译器获得大小的，当然不会执行
    double *pDouble = nullptr;
    EXPECT_EQ(sizeof(double), sizeof(*pDouble));
}

int32_t InitializerListFunc(initializer_list<int32_t> nums)
{
    int32_t sum = 0;
    for (const auto &num : nums)
    {
        sum += num;
    }

    return sum;
}

TEST(Cpp11, InitializerList)
{
    auto sum = InitializerListFunc({1,2,3,4,5,6});
    EXPECT_EQ(21, sum);
}

TEST(Cpp11, lambda)
{
    int32_t a = 1;

    // mutable可以让捕获的变量可以修改，但是值捕获并不会影响到原始变量
    auto f1 = [a]()mutable
    {
        a++;
    };

    f1();
    EXPECT_EQ(1, a);

    // 对于引用捕获，使用mutable和不使用没什么区别
    auto f2 = [&a]()
    {
        a++;
    };

    f2();
    EXPECT_EQ(2, a);

    auto f3 = [&a]()mutable
    {
        a++;
    };

    f3();
    EXPECT_EQ(3, a);
}
