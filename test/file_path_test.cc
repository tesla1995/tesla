#include "tutil/files/file_path.h"

#include <string.h>
#include <iostream>
#include <gtest/gtest.h>

using namespace std;
using namespace tutil;

namespace {

// The fixture for testing FilePath.
class FilePathTest : public ::testing::Test {
 protected:
  // You can remove any or all of the following functions if its body
  // is empty.

  FilePathTest() {
     // You can do set-up work for each test here.
  }

  ~FilePathTest() override {
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
  
}; // namespace FilePathTest

TEST_F(FilePathTest, DirName) {
  {
    FilePath path(".");
    FilePath dirname(path.DirName());
    ASSERT_EQ(dirname, FilePath("."));
  }

  {
    FilePath path("./");
    FilePath dirname(path.DirName());
    ASSERT_EQ(dirname, FilePath("."));
  }

  {
    FilePath path(".//");
    FilePath dirname(path.DirName());
    ASSERT_EQ(dirname, FilePath("."));
  }

  {
    FilePath path(".///");
    FilePath dirname(path.DirName());
    ASSERT_EQ(dirname, FilePath("."));
  }

  {
    FilePath path("..");
    FilePath dirname(path.DirName());
    ASSERT_EQ(dirname, FilePath("."));
  }

  {
    FilePath path("test");
    FilePath dirname(path.DirName());
    ASSERT_EQ(dirname, FilePath("."));
  }

  {
    FilePath path("/test");
    FilePath dirname(path.DirName());
    ASSERT_EQ(dirname, FilePath("/"));
  }

  {
    FilePath path("/");
    FilePath dirname(path.DirName());
    ASSERT_EQ(dirname, FilePath("/"));
  }

  {
    FilePath path("//test");
    FilePath dirname(path.DirName());
    ASSERT_EQ(dirname, FilePath("//"));
  }

  {
    FilePath path("/home/test");
    FilePath dirname(path.DirName());
    ASSERT_EQ(dirname, FilePath("/home"));
  }

  {
    FilePath path("/home////test");
    FilePath dirname(path.DirName());
    ASSERT_EQ(dirname, FilePath("/home"));
  }
}

TEST_F(FilePathTest, BaseName) {
  {
    FilePath path(".");
    FilePath dirname(path.BaseName());
    ASSERT_EQ(dirname, FilePath("."));
  }

  {
    FilePath path("..");
    FilePath dirname(path.BaseName());
    ASSERT_EQ(dirname, FilePath(".."));
  }

  {
    FilePath path("/");
    FilePath dirname(path.BaseName());
    ASSERT_EQ(dirname, FilePath("/"));
  }

  {
    FilePath path("test");
    FilePath dirname(path.BaseName());
    ASSERT_EQ(dirname, FilePath("test"));
  }

  {
    FilePath path("/home/test");
    FilePath dirname(path.BaseName());
    ASSERT_EQ(dirname, FilePath("test"));
  }

  {
    FilePath path("/home//test");
    FilePath dirname(path.BaseName());
    ASSERT_EQ(dirname, FilePath("test"));
  }
}

TEST_F(FilePathTest, Extension) {
  {
    FilePath path("root.cer");
    ASSERT_EQ(path.Extension(), ".cer");
  }

  {
    FilePath path("root.txt.cer");
    ASSERT_EQ(path.Extension(), ".cer");
    ASSERT_NE(path.Extension(), ".txt.cer");
  }

  {
    FilePath path("root.tar.gz");
    ASSERT_EQ(path.Extension(), ".tar.gz");
  }

  {
    FilePath path("root.tar.z");
    ASSERT_EQ(path.Extension(), ".tar.z");
  }

  {
    FilePath path("root.tar.bz2");
    ASSERT_EQ(path.Extension(), ".tar.bz2");
  }
}

}  // namespace

int main(int argc, char **argv) {

  // Parses the command line for googletest flags, and removes all recognized flags.
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
