// FixedPoint.h
#ifndef FIXEDPOINT_H
#define FIXEDPOINT_H

#include <cstdint>
#include <type_traits>
#include <algorithm>

/*
 * 32 位定点静态常数表
 * PI:3.1415926535897931 : 13493037704
 *
 * CORDIC 常数表:
 * {0,1.00000000000000000,0.78539816339744828,0.70710678118654746,},
 * {1,0.50000000000000000,0.46364760900080609,0.63245553203367577,},
 * {2,0.25000000000000000,0.24497866312686414,0.61357199107789628,},
 * {3,0.12500000000000000,0.12435499454676144,0.60883391251775243,},
 * {4,0.06250000000000000,0.06241880999595735,0.60764825625616825,},
 * {5,0.03125000000000000,0.03123983343026828,0.60735177014129604,},
 * {6,0.01562500000000000,0.01562372862047683,0.60727764409352614,},
 * {7,0.00781250000000000,0.00781234106010111,0.60725911229889284,},
 * {8,0.00390625000000000,0.00390623013196697,0.60725447933256249,},
 * {9,0.00195312500000000,0.00195312251647882,0.60725332108987529,},
 * {10,0.00097656250000000,0.00097656218955932,0.60725303152913446,},
 * {11,0.00048828125000000,0.00048828121119490,0.60725295913894495,},
 * {12,0.00024414062500000,0.00024414062014936,0.60725294104139727,},
 * {13,0.00012207031250000,0.00012207031189367,0.60725293651701029,},
 * {14,0.00006103515625000,0.00006103515617421,0.60725293538591352,},
 * {15,0.00003051757812500,0.00003051757811553,0.60725293510313938,},
 *
 * 转换成 32 位定点:
 * {0,4294967296,3373259426,3037000499,},
 * {1,2147483648,1991351317,2716375826,},
 * {2,1073741824,1052175346,2635271635,},
 * {3,536870912,534100634,2614921742,},
 * {4,268435456,268086747,2609829388,},
 * {5,134217728,134174062,2608555989,},
 * {6,67108864,67103403,2608237620,},
 * {7,33554432,33553749,2608158027,},
 * {8,16777216,16777130,2608138129,},
 * {9,8388608,8388597,2608133154,},
 * {10,4194304,4194302,2608131910,},
 * {11,2097152,2097151,2608131599,},
 * {12,1048576,1048575,2608131522,},
 * {13,524288,524287,2608131502,},
 * {14,262144,262143,2608131497,},
 * {15,131072,131071,2608131496,},
 *
 */

/*
 * 用于优化乘法溢出的简易方法
 *  FixedPoint<FB, T> operator*(const FixedPoint<FB, T>& n) const {
 *      auto l_i = this->content >> FB;
 *      auto r_i = n.content >> FB;
 *      auto l_f = this->content & ((1L << FB) - 1);
 *      auto r_f = n.content & ((1L << FB) - 1);
 *      return Raw(((l_i * r_i) << FB) + (l_i * r_f) + (l_f * r_i) + ((l_f * r_f) >> FB));
 *  }
 *
 */


template <const size_t FB, typename T = int64_t>
struct FixedPoint {
private:

    // 编译时做位长检验
    template< bool EN_B, class EN_T = void >
    using _enable_if_t_ = typename std::enable_if<EN_B, EN_T>::type;

    // 定点数小数位小于 32 位防止溢出
    using _check_fixed_bit_ = _enable_if_t_<FB <= 32>;

    // DEBUG 的时候方便调试
    using Number = FixedPoint<FB, T>;

    T _content;

public:

    constexpr static size_t FixedBits = FB;

    FixedPoint() : _content(0) { }
    FixedPoint(float num){ *this = Float(num); }
    FixedPoint(double num) { *this = Double(num); }
    FixedPoint(T num, size_t fb) { _content = num >> (fb - FB); }

    template<typename N>
    FixedPoint(N num){ *this = Int(num); }

    const T& getContent() const { return _content; }

    static Number Int(int64_t number) { return Raw(number << FB); }
    static Number Float(float number) { return Raw(static_cast<T>((number) * (1 << FB))); }
    static Number Double(double number) { return Raw(static_cast<T>((number) * (1 << FB))); }
    static Number Raw(T content) { return Number(content, FB); }
    static Number Raw(T content, size_t fb) { return Number(content, fb); }

    T toInt() const { return _content >> FB; }
    float toFloat() const { return (static_cast<float>(_content) / (1 << FB)); }
    double toDouble() const { return (static_cast<double>(_content) / (1 << FB)); }

    // 基本运算
    Number operator+(const Number& n) const { return Raw(_content + n._content); }
    Number operator-(const Number& n) const { return Raw(_content - n._content); }
    Number operator*(const Number& n) const { return Raw((_content * n._content) >> FB); }
    Number operator/(const Number& n) const { return Raw((_content << FB) / n._content); }

    Number operator<<(const uint64_t& n) const { return Raw(_content << n); }
    Number operator>>(const uint64_t& n) const { return Raw(_content >> n); }

    Number operator-() const { return Raw(-_content); }

    Number& operator+=(const Number& n) { return *this = *this + n; }
    Number& operator-=(const Number& n) { return *this = *this - n; }
    Number& operator*=(const Number& n) { return *this = *this * n; }
    Number& operator/=(const Number& n) { return *this = *this / n; }

    bool operator<(const Number& n) const { return _content < n._content; }
    bool operator>(const Number& n) const { return _content > n._content; }
    bool operator==(const Number& n) const { return _content == n._content;  }
    bool operator!=(const Number& n) const { return _content != n._content;  }
    bool operator>=(const Number& n) const { return _content >= n._content;  }
    bool operator<=(const Number& n) const { return _content <= n._content;  }

    // 定义一些基本常量
    const static FixedPoint<FB, T> PI;

    const static FixedPoint<FB, T> MIN;
    const static FixedPoint<FB, T> MAX;

    const static FixedPoint<FB, T> cordic_table[16][2];

    static Number min(
            const Number& a,
            const Number& b) {
        return a < b ? a : b;
    }

    static Number max(
            const Number& a,
            const Number& b) {
        return a > b ? a : b;
    }

    static Number abs(const Number& n) {
        return n._content < 0 ? -n : n;
    }

    static Number sign(const Number& n) {
        return n._content < 0 ? -1 : 1;
    }

    static Number floor(const Number& n) {
        auto fract = n._content & ((1 << FB) - 1);
        return   fract ? Raw(n._content - fract) : n ;
    }

    static Number ceil(const Number& n) {
        return  n._content & ((1 << FB) - 1) ? n + 1 : n ;
    }

    static Number round(const Number& n) {
        auto fract = n._content & ((1 << FB) - 1);
        if (fract > ( 1 << (FB - 1))) {
            return n + 1;
        } else {
            return Raw(n._content - fract);
        }
    }

    static Number sqrt(const Number& n) {
        if (n == 0) return 0;
        auto _sqrt = [&n](Number _r){ return  (_r + ( n / _r)) / 2; };
        auto lr = n;
        auto r = _sqrt(n);
        while (lr - r > 0) { lr = r; r = _sqrt(r); }
        return r;
    }

    // 三角函数和向量运算

public:
    // 定义基本数据结构

    class SinCosPair;
    class PVector;
    class Point;
    class Vector;

    class SinCosPair {
    private:
        Number _sin; Number _cos;

    public:
        const Number& sin() const { return _sin; }
        const Number& cos() const { return _cos; }
        SinCosPair(const Number& sin, const Number& cos)
            : _sin(sin), _cos(cos) {  }
        SinCosPair() : SinCosPair(0, 0) { }
    };

    class PVector {
    private:
        Number _theta; Number _mold;

    public:
        const Number& theta() const { return _theta; }
        const Number& mold() const { return _mold; }
        PVector(const Number& theta, const Number& mold)
            : _theta(theta), _mold(mold) { }
        PVector() : PVector(0, 0) { }
    };

    // 向量运算
    class Vector {
    private:
        Number _x; Number _y;

    public:
        const Number& x() const { return _x; }
        const Number& y() const { return _y; }
        Vector(const Number& x, const Number& y)
            : _x(x), _y(y) { }
        Vector() : Vector(0, 0) { }

        Vector operator*(const Number& n) const { return { _x * n, _y * n }; }
        Vector operator/(const Number& n) const { return { _x / n, _y / n }; }
        Vector operator+(const Vector& v) const { return { _x + v.x(), _y + v.y() }; }
        Vector operator-(const Vector& v) const { return { _x - v.x(), _y - v.y() }; }

        bool operator==(const Vector& v) const { return _x == v.x() && _y == v.y(); }
        bool operator!=(const Vector& v) const { return _x != v.x() || _y != v.y(); }

        Vector operator*=(const Number& n) { return *this = *this * n; }
        Vector operator/=(const Number& n) { return *this = *this / n; }
        Vector operator+=(const Vector& v) { return *this = *this + v; }
        Vector operator-=(const Vector& v) { return *this = *this - v; }

        Vector operator-() const { return { - _x, - _y }; }

        Number dot(const Vector& v) const {  return _x * v.x() + _y * v.y(); }
        Number operator*(const Vector& v) const {  return dot(v); }

        Number mold() const { return Number::mold(_y, _x); }
        Number theta() const { return Number::atan2(_y, _x); }

        Vector unit() const { auto l = mold(); return l == 0  ? Vector(0, 0)  : Vector(_x / l, _y / l); }

        Vector rotate(const Number& theta) const {
            SinCosPair pair = sincos(theta);
            return { pair.cos() * _x - pair.sin() * _y,
                        pair.sin() * _x + pair.cos() * _y };
        }

        Vector normal() const { return { -_y , _x }; }
        Number project(const Vector& v) const { return (*this * v) / v.mold(); }
    };

private:

    static Vector rotate(const Vector& v, size_t i, bool s) {
        if (s) {
            return { v.x() + (v.y() >> i), v.y() - (v.x() >> i) };
        } else {
            return { v.x() - (v.y() >> i), v.y() + (v.x() >> i) };
        }
    }


public:

    static Number standrad(const Number& _rad) {
        auto rad = _rad;
        while(rad > PI) rad -= PI << 1;
        while (rad < -PI) rad += PI << 1;
        return rad;
    }

    static PVector polar( const Number& x,
                          const Number& y ) {
        Vector v = {x, y};
        Number shift;
        if (x < 0) v = {-x, y};

        FixedPoint result = 0;
        Number K = 1;

        for (auto i = 0; i < std::min(16, static_cast<int>(FB)) ; i++) {
            auto theta = cordic_table[i][0];
            if ( v.y() > 0) {
                v = rotate(v, i, 1);
                result += theta;
                K = cordic_table[i][1];
            } else if (v.y() < 0) {
                v = rotate(v, i, 0);
                result -= theta;
                K = cordic_table[i][1];
            } else {
                break;
            }
        }

        if (x < 0) {
            if (y >= 0) {
                result = PI - result;
            } else {
                result = - PI - result ;
            }
        }

        return {result, v.x() * K};
    }

    static SinCosPair sincos(const Number& _rad) {
        auto rad = standrad(_rad);

        auto sign_x = Int(1);
        auto sign_y = Int(1);

        if (rad > PI >> 1) {
            rad -= PI;
            sign_x = -1;
            sign_y = -1;
        } else if ( rad < -(PI >> 1)) {
            rad += PI;
            sign_x = -1;
            sign_y = -1;
        }

        Vector v = {1, 0};
        Number result = 0;
        Number K = 1;

        for (auto i = 0; i < std::min(16, static_cast<int>(FB)) ; i++) {
            auto theta = cordic_table[i][0];
            if (rad - result > 0) {
                v = rotate(v, i, 0);
                result += theta;
                K = cordic_table[i][1];
            } else if (rad - result < 0) {
                v = rotate(v, i, 1);
                result -= theta;
                K = cordic_table[i][1];
            } else {
                break;
            }
        }

        return { v.y() * K * sign_y,  v.x() * K * sign_x };
    }

    static Number sin(const Number& rad) {
        return sincos(rad).sin();
    }

    static Number cos(const Number& rad) {
        return sincos(rad).cos();
    }

    static Number atan2(
            const Number& y,
            const Number& x ) {
        return polar(x, y).theta();
    }

    static Number mold(
            const Number& y,
            const Number& x ) {
        return polar(x, y).mold();
    }

};


// 初始化常量
template <const size_t FB, typename T>
const FixedPoint<FB, T> FixedPoint<FB, T>::PI = Raw(13493037704, 32);

template <const size_t FB, typename T>
const FixedPoint<FB, T> FixedPoint<FB, T>::MIN = Raw(1);

template <const size_t  FB, typename T>
const FixedPoint<FB, T> FixedPoint<FB, T>::MAX = Raw(9223372036854775807L);

template <const size_t FB, typename T>
const FixedPoint<FB, T> FixedPoint<FB, T>::cordic_table[16][2]  = {
    {Raw(3373259426, 32),Raw(3037000499, 32)},
    {Raw(1991351317, 32),Raw(2716375826, 32)},
    {Raw(1052175346, 32),Raw(2635271635, 32)},
    {Raw(534100634, 32),Raw(2614921742, 32)},
    {Raw(268086747, 32),Raw(2609829388, 32)},
    {Raw(134174062, 32),Raw(2608555989, 32)},
    {Raw(67103403, 32),Raw(2608237620, 32)},
    {Raw(33553749, 32),Raw(2608158027, 32)},
    {Raw(16777130, 32),Raw(2608138129, 32)},
    {Raw(8388597, 32),Raw(2608133154, 32)},
    {Raw(4194302, 32),Raw(2608131910, 32)},
    {Raw(2097151, 32),Raw(2608131599, 32)},
    {Raw(1048575, 32),Raw(2608131522, 32)},
    {Raw(524287, 32),Raw(2608131502, 32)},
    {Raw(262143, 32),Raw(2608131497, 32)},
    {Raw(131071, 32),Raw(2608131496, 32)},
};

#endif // FIXEDPOINT_H
