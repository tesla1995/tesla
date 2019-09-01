#include "tutil/files/file_path.h"

using namespace tutil;

int main(void)
{
  FilePath path("test"); 
  FilePath new_path(path.DirName());

  return 0;
}
