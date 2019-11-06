// Licensed to the Apache Software Foundation (ASF) under one
// or more contributor license agreements.  See the NOTICE file
// distributed with this work for additional information
// regarding copyright ownership.  The ASF licenses this file
// to you under the Apache License, Version 2.0 (the
// "License"); you may not use this file except in compliance
// with the License.  You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.  See the License for the
// specific language governing permissions and limitations
// under the License.

// A server to receive RegisterUserRequest and send back GeneralResponse.

#include <gflags/gflags.h>
#include <butil/logging.h>
#include <brpc/server.h>
#include "authentication/register_user.pb.h"

DEFINE_bool(echo_attachment, true, "RegisterUser attachment as well");
DEFINE_int32(port, 8000, "TCP Port of this server");
DEFINE_int32(idle_timeout_s, -1, "Connection will be closed if there is no "
             "read/write operations during the last `idle_timeout_s'");
DEFINE_int32(logoff_ms, 2000, "Maximum duration of server's LOGOFF state "
             "(waiting for client to close connection before server stops)");

struct MySessionLocalData {
  MySessionLocalData() : x(23) {}
  int x;
};

// Implement your session data fatory derived from brpc::DataFactory.
// Note that CreateData() and DestroyData() need to be thread-safe, as
// they will be called in multiple threads during the running of server.
class MySessionLocalDataFactory : public brpc::DataFactory {
  public:
    void* CreateData() const {
      return new MySessionLocalData;
    }  
    void DestroyData(void* d) const {
      delete static_cast<MySessionLocalData*>(d);
    }  
};

// Your implementation of example::RegisterUserService
// Notice that implementing brpc::Describable grants the ability to put
// additional information in /status.
namespace tesla {
namespace authentication {
class RegisterUserServiceImpl : public RegisterUserService {
public:
    RegisterUserServiceImpl() {};
    virtual ~RegisterUserServiceImpl() override {};
    virtual void RegisterUser(google::protobuf::RpcController* cntl_base,
                      const RegisterUserRequest* request,
                      GeneralResponse* response,
                      google::protobuf::Closure* done) override {
        // This object helps you to call done->Run() in RAII style. If you need
        // to process the request asynchronously, pass done_guard.release().
        brpc::ClosureGuard done_guard(done);

        brpc::Controller* cntl =
            static_cast<brpc::Controller*>(cntl_base);

        // The purpose of following logs is to help you to understand
        // how clients interact with servers more intuitively. You should 
        // remove these logs in performance-sensitive servers.
        LOG(INFO) << "Received request[log_id=" << cntl->log_id() 
                  << "] from " << cntl->remote_side() 
                  << " to " << cntl->local_side()
                  << ": "
                  << "user_name[" << request->user_name() << "],"
                  << "password[" << request->password() << "],"
                  << "email[" << request->email() << "],"
                  << "mobile_phone[" << request->mobile_phone() << "],"
                  << " (attached=" << cntl->request_attachment() << ")";

        // Get the session-local data which is created by
        // ServerOptions.session_local_data_factory and
        // reused between different RPC.
        MySessionLocalData* session_data =
          static_cast<MySessionLocalData*>(cntl->session_local_data());

        if (session_data == nullptr) {
          cntl->SetFailed("Require ServerOptions.session_local_data_factory to be set with a correctly implemented instance");
          return;
        } else {
          LOG(INFO) << "session local data: " << session_data->x;
        }
        
        // Fill response.
        //response->set_message(request->message());
        
        if (request->password() != std::string("123456")) {
          response->set_result(1); 
          response->set_error_description("Wrong password");
          LOG(ERROR) << "Wrong password";
          cntl->SetFailed("Wrong password");
          return;
        }

        // You can compress the response by setting Controller, but be aware
        // that compression may be costly, evaluate before turning on.
        // cntl->set_response_compress_type(brpc::COMPRESS_TYPE_GZIP);

        if (FLAGS_echo_attachment) {
            // Set attachment which is wired to network directly instead of
            // being serialized into protobuf messages.
            cntl->response_attachment().append(cntl->request_attachment());
        }
    }
};

}  // namespace authentication
}  // namespace tesla

int main(int argc, char* argv[]) {
    // Parse gflags. We recommend you to use gflags as well.
    gflags::ParseCommandLineFlags(&argc, &argv, true);

    // Generally you only need one Server.
    brpc::Server server;

    // Instance of your service.
    tesla::authentication::RegisterUserServiceImpl echo_service_impl;

    // Instance of your session data factory.
    MySessionLocalDataFactory session_local_data_factory;

    // Add the service into server. Notice the second parameter, because the
    // service is put on stack, we don't want server to delete it, otherwise
    // use brpc::SERVER_OWNS_SERVICE.
    if (server.AddService(&echo_service_impl, 
                          brpc::SERVER_DOESNT_OWN_SERVICE) != 0) {
        LOG(ERROR) << "Fail to add service";
        return -1;
    }

    // Start the server.
    brpc::ServerOptions options;
    options.idle_timeout_sec = FLAGS_idle_timeout_s;
    options.session_local_data_factory = &session_local_data_factory;
    if (server.Start(FLAGS_port, &options) != 0) {
        LOG(ERROR) << "Fail to start RegisterUserServer";
        return -1;
    }

    // Wait until Ctrl-C is pressed, then Stop() and Join() the server.
    server.RunUntilAskedToQuit();
    return 0;
}
