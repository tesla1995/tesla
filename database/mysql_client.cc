#include "database/mysql_client.h"
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <gflags/gflags.h>
#include "log/logging.h"

using namespace tesla::log;

namespace tesla {
namespace database {

// Client Configuration
DEFINE_string(mysqlx_url, "tesla:tesla1995@127.0.0.1/test",
              "The connection info used to connect the MySQL server");

DEFINE_int32(max_size, 5, "The maximum number of connections allowed in the pool");

DEFINE_int32(max_idle_time, 0, "The maximum number of milliseconds a connection"
             "is allowed to idle in the queue before being closed. A zero value means infinite.");

DEFINE_int32(queue_timeout, 0, "The maximum number of milliseconds a request is"
             "allowed to wait for a connection to become available. A zero value means infinite");

MysqlClient::MysqlClientPtr MysqlClient::mysql_client_ = nullptr;
pthread_once_t MysqlClient::once_control = PTHREAD_ONCE_INIT;

void MysqlClient::Init() {
  internal_client_.reset(new  mysqlx::Client(FLAGS_mysqlx_url,
                              mysqlx::ClientOption::POOL_MAX_SIZE, FLAGS_max_size,
                              mysqlx::ClientOption::POOL_MAX_IDLE_TIME, FLAGS_max_idle_time,
                              mysqlx::ClientOption::POOL_QUEUE_TIMEOUT, FLAGS_queue_timeout));

  if (!internal_client_) {
    fprintf(stderr, "Fail to construct Client object");
    exit(EXIT_FAILURE);
  }

  LOG_DEBUG << "Creating client connection pool on " << FLAGS_mysqlx_url.c_str();

  mysqlx::Session sess = internal_client_->getSession();

  {
    mysqlx::RowResult res = sess.sql("show variables like 'version'").execute();
    std::stringstream version;

    version << res.fetchOne().get(1).get<mysqlx::string>();
    int major_version;
    version >> major_version;

    if (major_version < 8)
    {
      fprintf(stderr, "The version of MySQL server must be greater than or equal to 8"); 
      exit(EXIT_FAILURE);
    }
  }
}

//void MysqlClient::Close() {
//  if (internal_client_) {
//    internal_client_.reset();
//    LOG_DEBUG << "Close client";
//  }
//}

void MysqlClient::CreateMysqlClient() {
  mysql_client_.reset(new MysqlClient);
  if (!mysql_client_) {
    fprintf(stderr, "Fail to construct MysqlClient object");
    exit(EXIT_FAILURE);
  }

  mysql_client_->Init();
}

MysqlClient::~MysqlClient() {
  LOG_DEBUG << "Close the clients to the MySQL server";
}

MysqlClient::MysqlClientPtr& MysqlClient::Singleton() {
  pthread_once(&once_control, CreateMysqlClient);
  return mysql_client_;
}

int MysqlClient::DefaultSelectCallback(mysqlx::SqlResult& result, void *args)
{
  (void)(args);

  LOG_DEBUG << "List of rows in the resultset.";
  for (mysqlx::Row row; (row = result.fetchOne()); ) {
    std::stringstream line;
    line << "next row: ";
    for (size_t i = 0; i < row.colCount(); i++) {
      line << row[i] << ",";
    }
    LOG_DEBUG << line.str();
  }

  return 0;
}


int MysqlClient::Execute(const std::string& statement,
                         const DataSetCallback& callback,
                         void* args) {
  try {
    mysqlx::Session sess = internal_client_->getSession();
    mysqlx::SqlResult result;

    try {
      sess.startTransaction();
      result = sess.sql(statement).execute();
      sess.commit();
    } catch (...) {
      sess.rollback();
      throw;
    }

    while (true) {
      // Use the hasData() method to learn that whether a SqlResult is a data
      // set or a result ret. If hasData() returns true, then the SqlResult
      // is a data set, which origins from a SELECT or similar command that
      // can return rows. Otherwise, it is a result set produced by INSERT,
      // UPDATE, DELETE, ...
      if (result.hasData()) {
        // A return value of true does not indicate whether the data set contains
        // any data. The data set can be empty, for example, it is empty if
        // fetchOne() NULL or fetchAll() returns an empty list.
        if (callback) {
          int ret = callback(result, args);
          if (ret < 0) {
            LOG_ERROR << "Fail to handle the data set with callback";
            return ret;
          }
        }
        // FIXME Can callback be null ?
      } else {
        LOG_DEBUG << "No rows in the resultset.";
        return result.getAffectedItemsCount();
      }

      if (!result.nextResult()) {
        break;
      }

      LOG_DEBUG << "Next resultset.";
    }
  } catch (const mysqlx::Error& e) {
    LOG_ERROR << "MySQL EXCEPTION: " << e.what();
    return -1;
  } catch (std::exception& ex) {
    LOG_ERROR <<"STD EXCEPTION: " << ex.what();
    return -1;
  } catch (const char *ex) {
    LOG_ERROR <<"EXCEPTION: " << ex;
    return -1;
  }

  return 0;
}

}  // namespace database
}  // namespace tesla
