#include <iostream>
#include <memory>

int main(void)
{
  std::shared_ptr<int> data = std::make_shared<int>(6);

  if (std::atomic_is_lock_free(&data)) {
    std::cout << "lock_free" << std::endl;
  } else {
    std::cout << "not lock_free" << std::endl;
  }

  return 0;
}
