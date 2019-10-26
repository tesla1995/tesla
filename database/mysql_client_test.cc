#include <stdlib.h>
#include <stdint.h>

#include <iostream>
#include <memory>
#include <string>

#include "database/mysql_client.h"

using namespace std;
using namespace tesla::database;

int main(void)
{
  auto& client = MysqlClient::Singleton();

  std::string statement;
  int ret = 0;

  statement.assign("INSERT INTO employee (username, position, leaf, parent) values ('qiuy', 'CEO', 0, 'CEO')");
  ret = client->Execute(statement);
  if (ret < 0) {
    cout << "Fail to execute sql[" << statement << "]" << endl;
    exit(EXIT_FAILURE);
  } else {
    cout << "Affected items count: " << ret << endl;
  }

  statement.assign("SELECT * FROM employee WHERE username = 'qiuy'");
  ret = client->Execute(statement);
  if (ret < 0) {
    cout << "Fail to execute sql[" << statement << "]" << endl;
    exit(EXIT_FAILURE);
  } else {
    cout << "rows count: " << ret << endl;
  }

  statement.assign("DELETE FROM employee WHERE username = 'qiuy'");
  ret = client->Execute(statement);
  if (ret < 0) {
    cout << "Fail to execute sql[" << statement << "]" << endl;
    exit(EXIT_FAILURE);
  } else {
    cout << "Affected items count: " << ret << endl;
  }

  return 0;
}
