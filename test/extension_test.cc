// Copyright (c) 2019 Tesla, Inc.
//
// Licensed under the Apache License, Version 2.0 (the License);
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an AS IS BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// Author: Michael Tesla (michaeltesla1995@gmail.com)
// Date: Wed Aug 14 07:43:21 CST 2019

#include "tutil/extension.h"

#include <iostream>
#include <memory>

using namespace std;
using namespace tesla::tutil;

class Human {
 public:
  virtual ~Human() {}
  virtual void talk() const = 0;
  virtual void color() const = 0;
};

class BlackHuman : public Human {
 public:
  static Human* New() {
    return new BlackHuman();
  }

 public:
  ~BlackHuman() override {}

  void color() const override {
    std::cout << "the skin color of a black person is black." << std::endl;
  }

  void talk() const override {
    std::cout << "In general, people don't understand what black people say."
         << std::endl;
  }
};

int main(void)
{
  Extension<Human>::GetInstance().RegisterOrDir("black", BlackHuman::New);

  Extension<Human>::CreateFunc black_func;
  std::unique_ptr<Human> black_human;
  if (Extension<Human>::GetInstance().Find("black", black_func)) {
    black_human.reset(black_func());
    black_human->color();
    black_human->talk();
  }

  return 0;
}
