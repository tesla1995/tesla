#include "tvar/detail/agent_group.h"

#include <string.h>
#include <iostream>
#include <thread>
#include <gtest/gtest.h>

#include "tutil/count_down_latch.h"
#include "tutil/current_thread.h"

using namespace std;
using namespace tesla::tutil;
using namespace tesla::tvar::detail;

constexpr int kThreadNum = 4;

CountDownLatch start_execute(1);
CountDownLatch end_execute(4);

namespace {

struct Agent {
  Agent() = default;
  Agent(int _a, int _b) : a(_a), b(_b) {}
  ~Agent() = default;

  int a{0};
  int b{0};
};

void ThreadFunc(AgentId id, size_t times) {
  start_execute.Wait();
  while (times--) {
    Agent* agent = AgentGroup<Agent>::GetTlsAgent(id);
    if (agent == nullptr) {
      LOG_INFO << "In thread " << CurrentThread::ThreadId() << ", agent is nullptr";
      agent = AgentGroup<Agent>::GetOrCreateTlsAgent(id);
      ASSERT_TRUE(agent != nullptr);

      agent->a = id;
    }
    ASSERT_EQ(agent->a, id);
  }
  end_execute.CountDown();
}

////////////////////////////////////////////////////////////////////////

// The fixture for testing AgentGroup.
class AgentGroupTest : public ::testing::Test {
 protected:
  // You can remove any or all of the following functions if its body
  // is empty.

  AgentGroupTest() {
     // You can do set-up work for each test here.
  }

  ~AgentGroupTest() override {
     // You can do clean-up work that doesn't throw exceptions here.
  }

  // If the constructor and destructor are not enough for setting up
  // and cleaning up each test, you can define the following methods:

  void SetUp() override {
     // Code here will be called immediately after the constructor (right
     // before each test).
     //cout << "===== Set Up =====" << endl;
  }

  void TearDown() override {
     // Code here will be called immediately after each test (right
     // before the destructor).
     //cout << "===== Tear Down =====" << endl;
  }

  // Objects declared here can be used by all tests in the test case for Flags.
}; // namespace AgentGroupTest

TEST_F(AgentGroupTest, GetOrCreateTlsAgent) {
  AgentId id = AgentGroup<Agent>::CreateNewAgent(); 
  ASSERT_TRUE(id == 0);
  ASSERT_TRUE(AgentGroup<Agent>::GetTlsAgent(id) == nullptr);

  Agent* agent = AgentGroup<Agent>::GetOrCreateTlsAgent(id);
  ASSERT_TRUE(agent != nullptr);
  ASSERT_TRUE(agent == AgentGroup<Agent>::GetOrCreateTlsAgent(id));
  ASSERT_TRUE(agent == AgentGroup<Agent>::GetOrCreateTlsAgent(id));
  ASSERT_TRUE(agent == AgentGroup<Agent>::GetOrCreateTlsAgent(id));

  AgentGroup<Agent>::DestroyAgent(id);
}

TEST_F(AgentGroupTest, MultiThread) {
  AgentId id = AgentGroup<Agent>::CreateNewAgent(); 
  LOG_INFO << "agent id=" << id;
  
  std::vector<std::thread> v;
  for (size_t i = 0; i < kThreadNum; ++i) {
    v.push_back(std::thread(ThreadFunc, id, 100000));
  }
  start_execute.CountDown();
  end_execute.Wait();

  for (auto& t : v) {
    if (t.joinable()) {
      t.join();
    }
  }

  AgentGroup<Agent>::DestroyAgent(id);
}

}  // namespace

int main(int argc, char **argv) {

  // Parses the command line for googletest flags, and removes all recognized flags.
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
