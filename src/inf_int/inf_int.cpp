#include <math.h>
#include "inf_int.h"
#include "../utils/common_utils.h"
#include "../uint_linked_list/uint_linked_list.h"
#include <ostream>

namespace linked_inf_num {
    inf_int::inf_int() : sign(false) {
    }

    inf_int::inf_int(const inf_int& other) {
        sign = other.sign;
        digits = other.digits;
    }

    inf_int::inf_int(const std::string& s) {
        if (!is_vaild_int(s))
            throw std::runtime_error("Invalid integer: " + s);

        std::string num = s;
        if (num[0] == '-') {
            sign = true;
            num.erase(num.begin());
        }
        else {
            sign = false;
        }

        digits.append(0);

        int cnt = 0;
        std::string binary = inf_int_str2bin_str(num);
        for (int i = binary.size() - 1; i >= 0; --i) {
            digits[0] += (binary[i] - '0') << cnt;
            cnt++;
            if (cnt == INF_INT_DIGIT_SIZE) {
                digits.insert(0, 0);
                cnt = 0;
            }
        }

        normalize();
    }

    inf_int::inf_int(const int& n) {
        int num = n;
        if (num < 0) {
            num = -num;
            sign = true;
        }
        else {
            sign = false;
        }

        digits.append(num);

        normalize();
    }

    inf_int::~inf_int() {
    }

    void inf_int::unify_zero_sign() {
        if (digits.is_zero()) {
            sign = false;
        }
    }

    void inf_int::normalize() {
        digits.remove_leading_zeros();
        unify_zero_sign();
    }

    std::string inf_int::to_string(const bool& comma = false) const {
        std::string binstr = digits.to_bit_string();

        std::string res = "0";
        for (int i = 0; i < binstr.length(); ++i) {
            res = decstr_mut2(res);
            if (binstr[i] == '1') {
                res = decstr_add1_i(res);
            }
        }

        if (comma) {
            res = add_commas(res);
        }

        if (sign) {
            res = "-" + res;
        }

        return res;
    }


    inf_int inf_int::abs_add(const inf_int& l, const inf_int& r) {
        inf_int a = l, b = r;
        inf_int tmp;

        int len_a = a.digits.length();
        int len_b = b.digits.length();

        bool carry = false;
        bool last_carry = false;
        unsigned long long t;
        for (int i = 0; i < std::min(len_a, len_b); ++i) {
            carry = is_add_overflow(a.digits[len_a - i - 1], b.digits[len_b - i - 1], last_carry);
            t = (unsigned long long)a.digits[len_a - i - 1] + b.digits[len_b - i - 1] + last_carry;
            if (carry) {
                t -= UINT_MAX + 1;
            }
            last_carry = carry;
            tmp.digits.insert(0, t);
        }

        inf_int* x, * y;
        int len_x, len_y;

        if (len_a < len_b) {
            x = &b;
            y = &a;
            len_x = len_b;
            len_y = len_a;
        }
        else {
            x = &a;
            y = &b;
            len_x = len_a;
            len_y = len_b;
        }

        for (int i = len_y; i < len_x; ++i) {
            carry = is_add_overflow(x->digits[len_x - i - 1], last_carry);
            t = (unsigned long long)x->digits[len_x - i - 1] + last_carry;
            if (carry) {
                t -= UINT_MAX + 1;
            }

            last_carry = carry;
            tmp.digits.insert(0, t);
        }

        if (carry != 0) {
            tmp.digits.insert(0, carry);
        }

        tmp.normalize();

        return tmp;
    }

    // +a - +b
    inf_int inf_int::abs_sub(const inf_int& l, const inf_int& r) {
        inf_int a = l, b = r;

        if (is_abs_less_than(a, b)) {
            inf_int tmp = abs_sub(b, a);
            tmp.sign = true;
            return tmp;
        }

        int len_a = a.digits.length();
        int len_b = b.digits.length();

        inf_int res;
        unsigned int tmp;

        bool borrow = false;
        bool last_borrow = false;
        for (int i = 0; i < std::min(len_a, len_b); ++i) {
            borrow = is_sub_overflow(a.digits[len_a - i - 1], b.digits[len_b - i - 1], last_borrow);
            if (borrow) {
                tmp = UINT_MAX - b.digits[len_b - i - 1] - last_borrow;
                tmp += a.digits[len_a - i - 1] + 1;
            }
            else {
                tmp = a.digits[len_a - i - 1] - b.digits[len_b - i - 1] - last_borrow;
            }
            last_borrow = borrow;

            res.digits.insert(0, tmp);
        }

        for (int i = len_b; i < len_a; ++i) {
            borrow = is_sub_overflow(a.digits[len_a - i - 1], last_borrow);
            if (borrow) {
                tmp = UINT_MAX;
            }
            else {
                tmp = a.digits[len_a - i - 1] - last_borrow;
            }
            last_borrow = borrow;

            res.digits.insert(0, tmp);
        }

        res.normalize();

        return res;
    }

    void inf_int::lshift32(const unsigned int& a) {
        int len = digits.length();
        for (int i = 0; i < a; ++i) {
            digits.insert(len, 0);
            ++len;
        }
        normalize();
    }

    void inf_int::rshift32(const unsigned int& a) {
        int len = digits.length();
        for (int i = 0; i < a; ++i) {
            digits.remove(len - 1);
            --len;
        }
        normalize();
    }

    void inf_int::negate() {
        if (!digits.is_zero())
            sign = !sign;
    }

    inf_int inf_int::abs_mul(const inf_int& x, const unsigned int& b) {
        inf_int res, a = x;
        unsigned long long tmp = 0;
        unsigned int carry = 0;

        // a * b          + carry
        // ((2^32 - 1)^2) + (((2^32 - 1)^2)/2^32) < 2^64

        for (int i = a.digits.length() - 1; i >= 0; --i) {
            tmp = (unsigned long long)a.digits[i] * (unsigned long long)b + (unsigned long long)carry;
            carry = (unsigned int)(tmp >> INF_INT_DIGIT_SIZE);  // /
            unsigned int t = tmp & (unsigned long long)UINT_MAX;
            res.digits.insert(0, t);               // %
        }

        if (carry != 0) {
            res.digits.insert(0, carry);
        }

        res.normalize();

        return res;
    }

    inf_int inf_int::abs_mul(const inf_int& x, const inf_int& y) {
        inf_int res, a = x, b = y;
        inf_int tmp;

        int len_b = b.digits.length();

        for (int i = len_b - 1; i >= 0; --i) {
            tmp = abs_mul(a, b.digits[i]);
            tmp.lshift32(len_b - i - 1);
            res = abs_add(res, tmp);
        }

        res.normalize();

        return res;
    }

    // a / b
    inf_int inf_int::abs_div(const inf_int& a, const inf_int& b, inf_int& remainder) {
        if (b.digits.is_zero()) {
            throw std::runtime_error("Division by zero");
        }

        if (a.digits.is_zero() || is_abs_less_than(a, b)) {
            remainder = inf_int(0);
            return inf_int(0);
        }

        if (uint_linked_list::is_equal(a.digits, b.digits)) {
            return inf_int(1);
        }

        // a > b, a, b != 0

        remainder = a;
        inf_int res;
        int len_a = a.digits.length();
        int len_b = b.digits.length();

        inf_int tmp = b;
        tmp.lshift32(len_a - len_b + 1);

        for (int i = len_b - 1; i < len_a; ++i) {
            tmp.rshift32(1);
            if (uint_linked_list::is_bitval_less_than(remainder.digits, tmp.digits)) {
                res.digits.append(0);
                continue;
            }

            int d = tmp.digits.get_first_one_pos() - remainder.digits.get_first_one_pos() + INF_INT_DIGIT_SIZE * (remainder.digits.length() - tmp.digits.length());
            unsigned int upper = leftshift_1fixed(1, (d > 31 ? 31 : d)), lower = 1 << (d - 1 < 0 ? 0 : d - 1);
            unsigned int mid;

            while (true) {
                mid = ((int64_t)lower + (int64_t)upper) / 2; // to prevent overflow

                inf_int minuend = abs_mul(tmp, mid);
                inf_int over_minuend = abs_add(tmp, minuend);

                bool min_less_than_remainder = uint_linked_list::is_bitval_less_than(minuend.digits, remainder.digits);
                bool over_min_less_than_remainder = uint_linked_list::is_bitval_less_than(over_minuend.digits, remainder.digits);

                /*
                ---------------------->
                         |(rem)            bool_o bool_m
                |(m) |(om)                  1       1
                    |    |                  0       1
                       |    |               0       1
                         |    |             0       0
                            |    |          0       0
                */

                if (over_min_less_than_remainder && min_less_than_remainder) {
                    lower = (lower == mid ? upper : mid);
                    continue;
                }
                if (!over_min_less_than_remainder && min_less_than_remainder) {
                    if (uint_linked_list::is_equal(over_minuend.digits, remainder.digits)) {
                        lower = mid;
                        continue;
                    }
                    break;
                }
                if (!over_min_less_than_remainder && !min_less_than_remainder) {
                    if (uint_linked_list::is_equal(minuend.digits, remainder.digits)) {
                        break;
                    }
                    upper = mid;
                }
            }

            res.digits.append(mid);
            remainder = abs_sub(remainder, abs_mul(tmp, mid));
        }

        return res;
    }


    inf_int inf_int::add(const inf_int& a, const inf_int& b) {
        // + +
        if (!a.sign && !b.sign) {
            return abs_add(a, b);
        }

        // + -
        if (!a.sign && b.sign) {
            return abs_sub(a, b);
        }

        // - +
        if (a.sign && !b.sign) {
            return abs_sub(b, a);
        }

        // - -
        inf_int tmp;
        tmp = abs_add(a, b);

        tmp.sign = true;
        tmp.unify_zero_sign();

        return tmp;
    }

    inf_int inf_int::sub(const inf_int& a, const inf_int& b) {
        // + +
        if (!a.sign && !b.sign) {
            return abs_sub(a, b);
        }

        // + -
        if (!a.sign && b.sign) {
            return abs_add(a, b);
        }

        inf_int tmp;
        // - +
        if (a.sign && !b.sign) {
            tmp = abs_add(a, b);
            tmp.sign = true;
            tmp.unify_zero_sign();
            return tmp;
        }

        // - -
        tmp = abs_sub(a, b);
        tmp.sign = !tmp.sign;
        tmp.unify_zero_sign();
        return tmp;
    }

    inf_int inf_int::mul(const inf_int& a, const inf_int& b) {
        // + + or - -
        if (a.sign == b.sign) {
            return abs_mul(a, b);
        }

        // + - or - +
        inf_int tmp;
        tmp = abs_mul(a, b);
        tmp.sign = true;
        tmp.unify_zero_sign();
        return tmp;
    }

    inf_int inf_int::div(const inf_int& a, const inf_int& b) {
        inf_int remainder;

        // + + or - -
        if (a.sign == b.sign) {
            return abs_div(a, b, remainder);
        }

        // + - or - +
        inf_int tmp;
        tmp = abs_div(a, b, remainder);
        tmp.sign = true;
        tmp.unify_zero_sign();
        return tmp;
    }


    bool inf_int::is_equal(const inf_int& a, const inf_int& b) {
        if (a.digits.is_zero() && b.digits.is_zero())
            return true;

        if (a.sign != b.sign)
            return false;

        return uint_linked_list::is_equal(a.digits, b.digits);
    }

    bool inf_int::is_abs_less_than(const inf_int& a, const inf_int& b) {
        return uint_linked_list::is_bitval_less_than(a.digits, b.digits);
    }

    bool inf_int::is_less_than(const inf_int& a, const inf_int& b) {
        // + -
        if (!a.sign && b.sign)
            return false;

        // - +
        if (a.sign && !b.sign)
            return true;

        // - -
        if (a.sign && b.sign)
            return !is_abs_less_than(a, b);

        // + +
        return is_abs_less_than(a, b);
    }


    inf_int operator+(const inf_int& a, const inf_int& b) {
        return inf_int::add(a, b);
    }

    inf_int operator-(const inf_int& a, const inf_int& b) {
        return inf_int::sub(a, b);
    }

    inf_int operator*(const inf_int& a, const inf_int& b) {
        return inf_int::mul(a, b);
    }

    inf_int operator/(const inf_int& a, const inf_int& b) {
        return inf_int::div(a, b);
    }

    bool operator==(const inf_int& a, const inf_int& b) {
        return inf_int::is_equal(a, b);
    }

    bool operator<(const inf_int& a, const inf_int& b) {
        return inf_int::is_less_than(a, b);
    }

    bool operator<=(const inf_int& a, const inf_int& b) {
        return a < b || a == b;
    }

    bool operator>(const inf_int& a, const inf_int& b) {
        return !(a <= b);
    }

    bool operator>=(const inf_int& a, const inf_int& b) {
        return !(a < b);
    }

    std::ostream& operator<<(std::ostream& output, const inf_int& num) {
        output << num.to_string(false);
        return output;
    }
}
