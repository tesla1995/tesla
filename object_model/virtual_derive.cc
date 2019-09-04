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
  void A2() override {
    cout << "B::A2()" << endl;
  }

  virtual void B1() {
    cout << "B::B1()" << endl;
  }
 public:
  long b_{0};
};

using Func = void(*)();

int main(void)
{
  cout << "sizeof(B) = " << sizeof(B) << endl;
  std::unique_ptr<A> base = std::make_unique<B>(); 
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
