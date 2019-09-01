#include "crypto/ssl_helper.h"
#include "log/logging.h"

using namespace tesla::crypto;
using namespace std;

int main(int argc, char *argv[])
{
  bool ret = true;

  (void)argc;
  if (argc < 3) {
    LOG_ERROR << "Usage: " << argv[0] << " certfile private_key"; 
    exit(1);
  }

  ret = LoadCertificate(argv[1], argv[2]);
  if (ret == false) {
    LOG_ERROR << "LoadCertificate";
    exit(1);
  }

  return 0;
}
