#include "crypto/digest.h"
#include "crypto/ssl_error.h"
#include "log/logging.h"

#include <memory>

using namespace tesla::crypto;
using namespace std;

int main(int argc, char *argv[])
{
  bool ret = true;
  char message[] = "Hello World";
  unsigned char md_value[EVP_MAX_MD_SIZE];
  unsigned int md_len;

  (void)argc;
  if (nullptr == argv[1]) {
    LOG_ERROR << "Usage: mdtest digestname"; 
    exit(1);
  }

  const EVP_MD* md(EVP_get_digestbyname(argv[1]));
  if (nullptr == md) {
    LOG_ERROR << "Unknown message digest: " << argv[1];
    exit(1);
  }
  
  ret = CalculateDigest(md, message, strlen(message), md_value, &md_len);
  if (false == ret) {
    LOG_ERROR << "CalculateDigest fail, " << OpenSSLError(ERR_get_error());
    exit(1);
  }

  printf("data is: %s\n", message);
  printf("Digest is: ");
  for (unsigned int i = 0; i < md_len; i++) {
    printf("%02X", md_value[i]);
  }
  printf("\n");

  return 0;
}
