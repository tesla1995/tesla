#include "test/test_comm.h"
#include "tutil/current_thread.h"

#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <stdlib.h>

using namespace tesla::tutil;

namespace tesla {
namespace test {

namespace internal {

unsigned GetRandomNumber() {
  static int initialized = 0;
  if (!initialized) {
    unsigned data = 0;
    int fd = open("/dev/urandom", O_RDONLY); 

    if (fd > 0 && read(fd, &data, sizeof(data)) > 0) {
      srand(data);
    } else {
      srand(time(NULL));
    }
    
    close(fd);
    ++initialized;
  }

  return rand();
}

} // namespace internal

char* RandLowcaseStr(char* str, int len) {
  srand(time(NULL));
	int i;
	for (i = 0; i < len; ++i)
	{
			str[i] = 'a' + internal::GetRandomNumber() % 26;
	}
	str[++i] = '\0';
	return str;
}

} // namespace test
} // namespace tesla
