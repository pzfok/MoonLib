#include "CppMath.h"

#include <cmath>
#include <iostream>

#include "CppTime.h"

using namespace std;

CppMath::CppMath(void)
{
}


CppMath::~CppMath(void)
{
}

const uint32_t CppMath::MIN_PRIMER = 2;

template <typename T>
int32_t CppMath::Sign(T value)
{
    if (value > 0)
    {
        return 1;
    }
    else if (value == 0)
    {
        return 0;
    }
    else
    {
        return -1;
    }
}

bool CppMath::IsInt(double doubleValue, double epsilon, int64_t &intValue)
{
    if (epsilon > 0.5 || epsilon < 0)
    {
        return false;
    }

    if (int32_t(doubleValue + epsilon) == int64_t(doubleValue - epsilon))
    {
        return false;
    }

    int64_t value = int64_t(doubleValue);

    intValue = (fabs(doubleValue - value) > 0.5) ? (value + CppMath::Sign(doubleValue)) : (value);
    return true;
}

bool CppMath::IsPrime(uint32_t num)
{
    if (num < MIN_PRIMER)
    {
        return false;
    }

    if (num == MIN_PRIMER)
    {
        return true;
    }

    // 判断是否能被2整除
    if ((num & 1) == 0)
    {
        return false;
    }

    uint32_t sqrtOfNum = (uint32_t)sqrt((double)num); // num的开方

    // 从MIN_PRIMER到sqrt(num),如果任何数都不能被num整除,num是素数,否则不是
    for (uint32_t i = MIN_PRIMER + 1; i <= sqrtOfNum; i += 2)
    {
        if (num % i == 0)
        {
            return false;
        }
    }

    return true;
}

bool CppMath::IsIntegerSquare(uint32_t num)
{
    uint32_t qurtNum = (uint32_t)sqrt((double)num);

    return (qurtNum * qurtNum) == num;
}

set<uint32_t> CppMath::GetDiffPrimerFactorNum(uint32_t num)
{
    map<uint32_t, uint32_t> factorsMap = GetPrimerFactorNum(num);

    set<uint32_t> factors;
    for (auto factorIt = factorsMap.begin(); factorIt != factorsMap.end(); ++factorIt)
    {
        factors.insert(factorIt->first);
    }

    return factors;
}

map<uint32_t, uint32_t> CppMath::GetPrimerFactorNum(uint32_t num)
{
    map<uint32_t, uint32_t> factors;

    // 先把2排除
    while (num % 2 == 0)
    {
        ++factors[2];
        num /= 2;
    }

    // 加1是为了避免6的根号为2，计算不到3这种情况
    uint32_t sqrtOfNumPlus1 = sqrt((double)num) + 1;

    // 从3开始，每次加2
    for (uint32_t i = MIN_PRIMER + 1; i <= sqrtOfNumPlus1; i += 2)
    {
        while (num % i == 0)
        {
            ++factors[i];
            num /= i;
        }
    }

    if (num != 1)
    {
        ++factors[num];
    }

    return factors;
}

bool CppMath::GetDigitMap(uint32_t num, uint16_t &digitMap)
{
    uint32_t digit = 0;
    digitMap = 0;

    while (num != 0)
    {
        digit = num % 10;
        num /= 10;

        // 数字已存在,返回false
        if (digitMap & (1 << digit))
        {
            return false;
        }

        digitMap |= (1 << digit);
    }

    return true;
}

bool CppMath::IsNumsHaveSameDigit(const uint32_t nums[], uint32_t numCount)
{
    uint16_t lastDigitMap = 0;
    uint16_t currDigitMap = 0;

    if (numCount < 2)
    {
        return false;
    }

    if (!CppMath::GetDigitMap(nums[0], lastDigitMap))
    {
        return false;
    }

    for (uint32_t i = 1; i < numCount; ++i)
    {
        if (!CppMath::GetDigitMap(nums[i], currDigitMap))
        {
            return false;
        }

        if (currDigitMap != lastDigitMap)
        {
            return false;
        }
    }

    return true;
}

uint32_t CppMath::Factorial(uint32_t n)
{
    if (n <= 1)
    {
        return 1;
    }

    uint32_t result = 1;
    for (uint32_t i = 2; i <= n; ++i)
    {
        result *= i;
    }

    return result;
}

uint32_t CppMath::FactorialWithCache(uint32_t n, uint32_t cacheSize /*= 10000*/)
{
    static map<uint32_t, uint32_t> factorialCache;
    if (n <= 1)
    {
        return 1;
    }

    auto findIt = factorialCache.find(n);
    if (findIt != factorialCache.end())
    {
        return findIt->second;
    }

    // 找到N前面的一个值
    uint32_t result = 1;
    uint32_t i = 2;

    findIt = factorialCache.lower_bound(n);
    if (findIt != factorialCache.end() && findIt != factorialCache.begin())
    {
        --findIt;
        result = findIt->second;
        i = findIt->first + 1;
    }

    for (; i <= n; ++i)
    {
        result *= i;

        // 缓存下来
        if (factorialCache.size() < cacheSize)
        {
            factorialCache[i] = result;
        }
    }

    // 从末尾删除
    while (factorialCache.size() > cacheSize)
    {
        auto it = factorialCache.end();
        factorialCache.erase(--it);
    }

    return result;
}

uint32_t CppMath::Combination(uint32_t n, uint32_t r)
{
    // C(n,r)=n!/r!/(n-r)!=(r+1)*(r+2)*...*n/1/2/.../(n-r)
    uint32_t result = 1;

    for (uint32_t i = r + 1; i <= n; ++i)
    {
        result *= i;
    }

    uint32_t d = n - r;
    for (uint32_t i = 2; i <= d; ++i)
    {
        result /= i;
    }

    return result;
}

uint32_t CppMath::Random(uint32_t begin /*= 0*/, uint32_t end /*= 0*/)
{
    static bool have_init = false;
    if (!have_init)
    {
        srand(static_cast<unsigned int>(CppTime::GetUTime()));
        have_init = true;
    }

    uint64_t randValue = rand();
    if (end > begin)
    {
        randValue %= (end - begin);
    }

    return randValue + begin;
}

bool CppMathPrimeWithCache::IsPrime(uint32_t num)
{
    if (num < CppMath::MIN_PRIMER)
    {
        return false;
    }

    if (num == CppMath::MIN_PRIMER)
    {
        return true;
    }

    // 判断是否能被2整除
    if ((num & 1) == 0)
    {
        return false;
    }

    // 命中cache，直接返回true
    if (PrimeSet.find(num) != PrimeSet.end())
    {
        return true;
    }

    // 先遍历所有缓存的素数
    for (auto it = PrimeSet.begin(); it != PrimeSet.end(); ++it)
    {
        // 找到整除
        if (num % (*it) == 0)
        {
            return false;
        }

        // 超过大小了，回到旧版本
        if (num < (*it))
        {
            break;
        }
    }

    // 如果缓存数据无效，再使用古老的方式，不过起始值改成*PrimerySet.rbegin() + 1，如果cache有的话
    if (CppMath::IsPrime(num))
    {
        PrimeSet.insert(num);
        return true;
    }

    return false;
}
