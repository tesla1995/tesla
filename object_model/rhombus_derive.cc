#include <iostream>
#include <memory>

using namespace std;

class A {
 public:
  A(int a = 1) : a_(a) {}

  virtual ~A() {
    cout << "A::~A()" << endl;
  }
 public:
  virtual void A1() {
    cout << "A::A1()" << endl;
  }

  virtual void A2() {
    cout << "A::A2()" << endl;
  }

  virtual void A3() {
    cout << "A::A3()" << endl;
  }
 public:
  int a_{0};
};

class B : virtual public A {
 public:
  B(int b = 2) : b_(b) {}

  ~B() override {
    cout << "B::~B()" << endl;
  }
 public:
  void A1() override {
    cout << "B::A1()" << endl;
  }

  virtual void B1() {
    cout << "B::B1()" << endl;
  }

  virtual void B2() {
    cout << "B::B2()" << endl;
  }
 public:
  int b_{0};
};

class C : virtual public A {
 public:
  C(int c = 3) : c_(c) {}

  ~C() override {
    cout << "C::~C()" << endl;
  }
 public:
  void A1() override {
    cout << "C::A1()" << endl;
  }

  void A2() override {
    cout << "C::A2()" << endl;
  }

  virtual void C1() {
    cout << "C::C1()" << endl;
  }
 public:
  int c_{0};
};

class D : public B, public C {
 public:
  D(int d = 3) : d_(d) {}

  ~D() override {
    cout << "D::~D()" << endl;
  }
 public:
  void A1() override {
    cout << "D::A1()" << endl;
  }

  void A3() override {
    cout << "D::A3()" << endl;
  }

  virtual void D1() {
    cout << "D::D1()" << endl;
  }
 public:
  int d_{0};
};

using Func = void(*)();

int main(void)
{
  cout << "sizeof(D) = " << sizeof(D) << endl;
  std::unique_ptr<A> base = std::make_unique<D>(); 
  base->A1();

  return 0;
}
