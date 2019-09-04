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

class B : public A {
 public:
  B(int b = 2) : b_(b) {}

  ~B() override {
    cout << "B::~B()" << endl;
  }
 public:
  virtual void B1() {
    cout << "B::B1()" << endl;
  }

  void A2() override {
    cout << "B::A2()" << endl;
  }

  void B3() {
    cout << "B::B3()" << endl;
  }
 public:
  long b_{0};
};

class C {
 public:
  C(int c = 1) : c_(c) {}

  virtual ~C() {
    cout << "C::~C()" << endl;
  }
 public:
  virtual void C1() {
    cout << "C::C1()" << endl;
  }

  virtual void C2() {
    cout << "C::C2()" << endl;
  }

  virtual void C3() {
    cout << "C::C3()" << endl;
  }
 public:
  int c_{0};
};

class D : public A, public C {
 public:
  D(int d = 1) : d_(d) {}

  ~D() override {
    cout << "D::~D()" << endl;
  }
 public:
  void A1() override {
    cout << "D:A1()" << endl;
  }

  void C3() override {
    cout << "D:C3()" << endl;
  }

  virtual void D1() {
    cout << "D::D1()" << endl;
  }
 public:
  int d_{0};
};

class E final : public D {
 public:
  ~E() {
    cout << "E::~E()" << endl;
  }

  virtual void E1() final {
    cout << "E::E1()" << endl;
  }

  void C1() override {
    cout << "E::C1()" << endl;
  }
};

void ExecuteC3(C* c) {
  c->C3();
}

using Func = void(*)();

int main(void)
{
  //cout << "sizeof(B) = " << sizeof(B) << endl;
  //std::unique_ptr<A> base = std::make_unique<B>(); 
  //base->A2();

  cout << "sizeof(D) = " << sizeof(D) << endl;
  std::unique_ptr<C> c = std::make_unique<D>(); 
  c->C3();

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
