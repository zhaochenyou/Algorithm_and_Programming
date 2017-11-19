// MIT License
// Copyright 2017 zhaochenyou16@gmail.com

#pragma once

#ifndef MAX
#define MAX 1024
#endif
#include <cstring>
#include <string>
#include <algorithm>
#include <cassert>
using namespace std;


//
// interface
//

struct Number {
  //整数部分 integer_[0]存储个位, integer_[1]存储十位...
  //输出时从后向前反向输出
  int integer[MAX];
  //有效数字的个数
  int integer_len;

  //小数部分 decimal_[0]存储十分之一位, decimal_[1]存储百分之一位...
  //输出时从前向后正向输出
  int decimal[MAX];
  //有效数字的个数
  int decimal_len;

  //正负号
  bool negative;
};


//初始化
Number Init();
Number Init(int a);
Number Init(double a);

//负号
Number Negative(const Number &a);
//加法
Number Add(const Number &a, const Number &b);
//减法
Number Sub(const Number &a, const Number &b);
//乘法
Number Multiple(const Number &a, const Number &b);
//除法
Number Devide(const Number &a, const Number &b);

//负的
bool IsNegative(const Number &a);
//正的
bool IsPositive(const Number &a);
//大于
bool Greater(const Number &a, const Number &b);
//大于等于
bool GreaterEqual(const Number &a, const Number &b);
//大于
bool Less(const Number &a, const Number &b);
//小于等于
bool LessEqual(const Number &a, const Number &b);
//等于
bool Equal(const Number &a, const Number &b);
//不等于
bool NotEqual(const Number &a, const Number &b);

//输出字符串
std::string IntString(const Number &a);
std::string FloatString(const Number &a);



//
// implement
//

namespace detail {

  auto AssertValid(const Number &a) -> void;

  auto FloatUp(const Number &a) -> Number;

  auto FloatDown(const Number &a, int decimal_len) -> Number;

}

//初始化零
Number Init() {
  Number c;
  c.negative = false;
  memset(c.decimal, 0, sizeof(c.decimal));
  c.decimal_len = 0;
  memset(c.integer, 0, sizeof(c.integer));
  c.integer_len = 0;

  detail::AssertValid(c);
  return c;
}

//初始化整数
Number Init(int a) {
  Number c = Init();

  c.negative = (a < 0);
  a = (a < 0)? (-a) : (a);
  for (int i = 0; i < MAX && a; i++) {
    c.integer_len++;
    c.integer[i] = a % 10;
    a /= 10;
  }

  detail::AssertValid(c);
  return c;
}

//初始化小数
Number Init(double a) {
  Number c = Init((int)a);

  c.negative = (a < 0.0);
  a = (a < 0.0)? (-a) : (a);
  a = a - (double)((int)a);
  for (int i = 0; i < MAX && (a != 0.0); i++) {
    a *= 10.0;
    int tmp = (int)(a);
    c.decimal_len++;
    c.decimal[i] = tmp % 10;
  }

  detail::AssertValid(c);
  return c;
}

//负号
Number Negative(const Number &a) {
  Number c = a;
  c.negative = !a.negative;

  detail::AssertValid(c);
  return c;
}

//加
Number Add(const Number &a, const Number &b) {
  //符号不同
  //12 + (-7) -> 12 - 7
  if (a.negative != b.negative) {
    Number c = Negative(b);
    return Sub(a, c);
  }

  //符号相同
  //12 + 7
  //-12 + (-7)
  Number c = Init();
  int n;

  //decimal
  n = max(a.decimal_len, b.decimal_len);
  for (int i = 0; i < n; i++) {
    c.decimal_len++;
    int j = n-i-1;
    c.decimal[j] += (a.decimal[j] + b.decimal[j]) % 10;
    if (j-1 >= 0)
      c.decimal[j-1] += (a.decimal[j] + b.decimal[j]) / 10;
    else
      c.integer[1-j] += (a.decimal[j] + b.decimal[j]) / 10;
  }

  //integer
  n = max(a.integer_len, b.integer_len);
  for (int i = 0; i < n; i++) {
    c.integer_len++;
    c.decimal[i] += (a.decimal[i] + b.decimal[i]) % 10;
    c.decimal[i+1] += (a.decimal[i] + b.decimal[i]) / 10;
  }

  c.negative = a.negative;
  detail::AssertValid(c);
  return c;
}

//减
Number Sub(const Number &a, const Number &b) {
  //符号不同
  //12 - (-7) -> 12 + 7
  if (a.negative != b.negative) {
    Number c = Negative(b);
    return Add(a, c);
  }

  //符号相同
  //12 - 7
  //-7 - (-12)

  //a < b
  //7 - 32
  //-12 - (-3)
  if (Less(a, b)) {
    return Negative(Sub(b, a));
  }

  //a >= b且都是负数
  //-5 - (-12) -> 12 - 5
  if (IsNegative(a) && Greater(a, b)) {
    return Sub(Negative(b), Negative(a));
  }

  //a >= b且都是正数
  //12 - 5

  Number c = Init();
  int n;
  int borrow = 0;

  //decimal
  n = max(a.decimal_len, b.decimal_len);
  for (int i = 0; i < n; i++) {
    c.decimal_len++;
    int j = n-i-1;
    int tmp_a = a.decimal[j] - borrow;
    if (tmp_a < b.decimal[j]) {
      borrow = (b.decimal[j] - tmp_a) / 10 + 1;
      tmp_a += borrow * 10;
    }

    assert(tmp_a >= b.decimal[j]);

    c.decimal[j] += tmp_a - b.decimal[j];

    assert(c.decimal[j] >= 0);
    assert(c.decimal[j] < 9);
  }

  //integer
  n = max(a.integer_len, b.integer_len);
  for (int i = 0; i < n; i++) {
    c.integer_len++;
    int tmp_a = a.integer[i] - borrow;
    if (tmp_a < b.integer[i]) {
      borrow = (b.integer[i] - tmp_a) / 10 + 1;
      tmp_a += borrow * 10;
    }

    assert(tmp_a >= b.integer[i]);

    c.integer[i] += tmp_a - b.integer[i];

    assert(c.integer[i] >= 0);
    assert(c.integer[i] < 9);
  }

  assert(Greater(c, Init()));
  detail::AssertValid(c);
  return c;
}


//乘
Number Multiple(const Number &a, const Number &b) {
  //a b为零
  if (Equal(a, Init()))
    return Init();
  if (Equal(b, Init()))
    return Init();

  Number a2 = detail::FloatUp(a), b2 = detail::FloatUp(b);
  Number c = Init();
  int n;

  n = max(a2.integer_len, b2.integer_len);
  for (int i = 0; i < n; i++) {
    c.integer[i] += (a2.integer[i] + b2.integer[i]) % 10;
    c.integer[i+1] += (a2.integer[i] + b2.integer[i]) / 10;
  }
  c = detail::FloatDown(c, a.decimal_len + b.decimal_len);
  c.negative = (a.negative == b.negative)? false : true;

  detail::AssertValid(c);
  return c;
}

//除
//TODO
Number Devide(const Number &a, const Number &b) {
  return Init();
}

//输出整数
std::string IntString(const Number &a) {
  detail::AssertValid(a);
  std::string ret(a.integer_len + 5, '*');
  int int_index = 0;
  for (int i = 0; i < ret.size(); i++) {
    if (i == 0 && a.negative) {
        ret[i] = '-';
    } else {
      if (a.integer_len == 0 && int_index >= 0) {
        ret[i] = '0';
      } else if (int_index < a.integer_len) {
        ret[i] = (char)(a.integer[int_index] + (int)'0');
      }
      int_index++;
    }
  }
  return ret;
}

//输出小数
std::string FloatString(const Number &a) {
  detail::AssertValid(a);
  std::string ret(a.integer_len + a.decimal_len + 5, '*');
  int index;
  int i;

  index = 0;
  for (i = 0; i < ret.size() && index < a.integer_len; i++) {
    if (i == 0 && a.negative) {
        ret[i] = '-';
    } else {
      if (a.integer_len == 0 && index >= 0) {
        ret[i] = '0';
      } else if (index < a.integer_len) {
        ret[i] = (char)(a.integer[index] + (int)'0');
      }
      index++;
    }
  }

  index = 0;
  ret[i++] = '.';

  do {
    if (a.decimal_len == 0) {
      ret[i++] = '0';
    } else {
      ret[i] = (char)(a.decimal[index] + (int)'0');
      i++;
      index++;
    }
  } while (i < ret.size() && index < a.decimal_len);

  return ret;
}

namespace detail {

  auto AssertValid(const Number &a) -> void {
    for (int i = 0; i < a.integer_len; i++) {
      assert(a.integer[i] >= 0);
      assert(a.integer[i] < 9);
    }
    for (int i = 0; i < a.decimal_len; i++) {
      assert(a.decimal[i] >= 0);
      assert(a.decimal[i] < 9);
    }
  }

  auto FloatUp(const Number &a) -> Number {
    Number c = a;
    memmove(c.integer + c.decimal_len, c.integer, c.integer_len);
    for (int i = 0; i < c.decimal_len; i++) {
      c.integer[i] = c.decimal[c.decimal_len-i-1];
    }
    c.integer_len = a.integer_len + a.decimal_len;
    c.decimal_len = 0;

    AssertValid(c);
    return c;
  }

  auto FloatDown(const Number &a, int decimal_len) -> Number {
    assert(decimal_len <= a.integer_len);
    Number c = a;
    memmove(c.decimal, c.decimal + decimal_len, decimal_len);
    for (int i = 0; i < c.decimal_len; i++) {
      c.decimal[decimal_len-i-1] = c.integer[i];
    }
    memmove(c.integer + decimal_len, c.integer, c.integer_len - decimal_len);

    c.integer_len = a.integer_len - decimal_len;
    c.decimal_len += decimal_len;

    AssertValid(c);
    return c;
  }

}
