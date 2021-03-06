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
  virtual void B1() {
    cout << "B::B1()" << endl;
  }
 public:
  int b_{0};
};

class D : virtual public A {
 public:
  void D1() {
     cout << "D::D1()" << endl;
  }

  int d_{0};
};

class C : public B {
 public:
  C(int c = 3) : c_(c) {}

  ~C() override {
    cout << "C::~C()" << endl;
  }
 public:
  void A2() override {
    cout << "C::A2()" << endl;
  }

  virtual void C1() {
    cout << "C::C1()" << endl;
  }
 public:
  int c_{0};
};

using Func = void(*)();

int main(void)
{
  cout << "sizeof(C) = " << sizeof(C) << endl;
  std::unique_ptr<A> base = std::make_unique<C>(); 
  base->A2();

  //D d = D();
  //C* c = &d;
  //c->C3();
  
  //C c = C();
  //ExecuteC3(&c);

  //size_t* c_vptr = reinterpret_cast<size_t*>(*reinterpret_cast<size_t*>(base_d));
  //Func func = reinterpret_cast<Func>(c_vptr[1]);
  //func();
  //cout << "----------" << endl;

  //delete base_d;

  //D* d = nullptr;
  //cout << "offset of a_: " << &(d->a_) << endl;
  //cout << "offset of c_: " << &(d->c_) << endl;
  //cout << "offset of d_: " << &(d->d_) << endl;

  return 0;
}
