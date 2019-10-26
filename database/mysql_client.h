// Copyright (c) 2019 Tesla, Inc.
//
// Licensed under the Apache License, Version 2.0 (the License);
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an AS IS BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// Author: Michael Tesla (michaeltesla1995@gmail.com)
// Date: Sat Oct 26 06:41:05 CST 2019

#ifndef TESLA_DATABASE_MYSQL_CLIENT_H_
#define TESLA_DATABASE_MYSQL_CLIENT_H_

#include <pthread.h>
#include <memory>
#include <string>
#include <functional>

#include "mysql-cppconn-8/mysqlx/xdevapi.h"

namespace tesla {
namespace database {

class MysqlClient {
 public:
  using MysqlClientPtr = std::unique_ptr<MysqlClient>;
  using DataSetCallback = std::function<int (mysqlx::SqlResult&, void* args)>;

  ~MysqlClient();

  static MysqlClientPtr& Singleton();

  int Execute(const std::string& statement,
              const DataSetCallback& callback = DefaultSelectCallback,
              void* args = nullptr);
 private:
  MysqlClient() = default;

 private:
  using InternalClientPtr = std::unique_ptr<mysqlx::Client>;

  void Init();

  static void CreateMysqlClient();
  static int DefaultSelectCallback(mysqlx::SqlResult& result, void *args);

  static MysqlClientPtr mysql_client_;
  static pthread_once_t once_control;

  InternalClientPtr internal_client_;
};

}  // namespace database
}  // namespace tesla

#endif  // TESLA_DATABASE_MYSQL_CLIENT_H_
