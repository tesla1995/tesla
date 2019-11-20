#include "tutil/signal_slot.h"
#include <iostream>

using namespace std;
using namespace tesla::tutil;

class String
{
 public:
  String(const char* str)
  {
    printf("String ctor this %p\n", this);
  }

  String(const String& rhs)
  {
    printf("String copy ctor this %p, rhs %p\n", this, &rhs);
  }

  String(String&& rhs)
  {
    printf("String move ctor this %p, rhs %p\n", this, &rhs);
  }
};

class Foo
{
 public:
  void zero();
  void zeroc() const;
  void one(int);
  void oner(int&);
  void onec(int) const;
  void oneString(const String& str);
  // void oneStringRR(String&& str);
  static void szero();
  static void sone(int);
  static void soneString(const String& str);
};

void Foo::zero()
{
  printf("Foo::zero()\n");
}

void Foo::zeroc() const
{
  printf("Foo::zeroc()\n");
}

void Foo::szero()
{
  printf("Foo::szero()\n");
}

void Foo::one(int x)
{
  printf("Foo::one() x=%d\n", x);
}

void Foo::onec(int x) const
{
  printf("Foo::onec() x=%d\n", x);
}

void Foo::sone(int x)
{
  printf("Foo::sone() x=%d\n", x);
}

void Foo::oneString(const String& str)
{
  printf("Foo::oneString\n");
}

void Foo::soneString(const String& str)
{
  printf("Foo::soneString\n");
}

int main(void)
{
  Foo data;

  {
    Signal<void ()> signal; 

    cout << "===== no data =====" << endl;
    signal.call();
    cout << "===== no data =====" << endl;

    cout << "===== no slot =====" << endl;
    signal.connect([&]{ data.zero(); });
    signal.call();
    cout << "===== no slot =====" << endl;

    cout << "===== slot =====" << endl;
    Slot slot = signal.connect([&]{ data.zero(); });
    signal.call();
    cout << "===== slot =====" << endl;
  }

  {
    Signal<void(int)> signal; 

    cout << "===== no data =====" << endl;
    signal.call(0);
    cout << "===== no data =====" << endl;

    cout << "===== no slot =====" << endl;
    signal.connect([&](int arg){ data.one(arg); });
    signal.call(1);
    cout << "===== no slot =====" << endl;

    cout << "===== slot =====" << endl;
    Slot slot = signal.connect([&](int arg){ data.one(arg); });
    signal.call(1);
    cout << "===== slot =====" << endl;
  }

  return 0;
}
