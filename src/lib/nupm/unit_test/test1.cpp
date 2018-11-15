/* note: we do not include component source, only the API definition */
#include <chrono>
#include <gtest/gtest.h>
#include <common/utils.h>
#include <common/rand.h>
#include "vmem_numa.h"
#include "arena_alloc.h"
#include "rp_alloc.h"

namespace {

// The fixture for testing class Foo.
class Libnupm_test : public ::testing::Test {

 protected:

  // If the constructor and destructor are not enough for setting up
  // and cleaning up each test, you can define the following methods:
  
  virtual void SetUp() {
    // Code here will be called immediately after the constructor (right
    // before each test).
  }

  virtual void TearDown() {
    // Code here will be called immediately after each test (right
    // before the destructor).
  }
  
  // Objects declared here can be used by all tests in the test case
};

static bool memcheck(void * p, char val, size_t len)
{
  for(size_t i=0; i<len; i++) {
    if(((char *)p)[i] != val)
      return false;
  }
  return true;
}

  //#define RUN_RPALLOCATOR_TESTS
//#define RUN_VMEM_ALLOCATOR_TESTS

TEST_F(Libnupm_test, NdControl)
{
  nupm::ND_control ctrl;
  PLOG("ND_control init complete.");
}

#ifdef RUN_RPALLOCATOR_TESTS
TEST_F(Libnupm_test, RpAllocatorPerf)
{
  nupm::Rp_allocator_volatile<1> allocator;
  allocator.initialize_thread();

  std::chrono::system_clock::time_point start, end;
  
  const unsigned ITERATIONS = 10000000;
  std::vector<void*> alloc_v;
  alloc_v.reserve(ITERATIONS);

  start = std::chrono::high_resolution_clock::now();
  for(unsigned i=0;i<ITERATIONS;i++) {
    void * p = allocator.alloc(64, i % 2);
    ASSERT_TRUE(p);
    alloc_v.push_back(p);
  }
  end = std::chrono::high_resolution_clock::now();
  double secs = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() / 1000.0;
  PINF("Arena::SingleThreadAlloc: %lu allocs/sec",
       (unsigned long) (((double) ITERATIONS) / secs));

  for(auto& a : alloc_v)
    allocator.free(a);
}

TEST_F(Libnupm_test, RpAllocatorPerf2)
{
  nupm::Rp_allocator_volatile<0> allocator;
  allocator.initialize_thread();

  std::chrono::system_clock::time_point start, end;
  
  const unsigned ITERATIONS = 10000000;
  std::vector<void*> alloc_v;
  alloc_v.reserve(ITERATIONS);

  start = std::chrono::high_resolution_clock::now();
  for(unsigned i=0;i<ITERATIONS;i++) {
    size_t s = genrand64_int64() % 8096;
    void * p = allocator.alloc(s, i % 2);
    ASSERT_TRUE(p);
    alloc_v.push_back(p);
  }
  end = std::chrono::high_resolution_clock::now();
  double secs = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() / 1000.0;
  PINF("Arena::SingleThreadAlloc: %lu allocs/sec",
       (unsigned long) (((double) ITERATIONS) / secs));

  for(auto& a : alloc_v)
    allocator.free(a);
}


TEST_F(Libnupm_test, RpAllocatorIntegrity)
{
  nupm::Rp_allocator_volatile<0> allocator;
  allocator.initialize_thread();

  const unsigned ITERATIONS = 1000000;
  std::vector<void*> alloc_v;
  alloc_v.reserve(ITERATIONS);

  for(unsigned i=0;i<ITERATIONS;i++) {
    void * p = allocator.alloc(1024, i % 2);
    ASSERT_TRUE(p);
    alloc_v.push_back(p);
    char m = ((char*)p)[7];
    memset(p, m, 1024);
  }

  for(auto& a : alloc_v) {
    char m = ((char*)a)[7];
    ASSERT_TRUE(memcheck(a,m,1024));
    allocator.free(a);
  }
}
#endif

#if 0
TEST_F(Libnupm_test, Arena)
{
  nupm::Arena_allocator_volatile allocator;

  std::chrono::system_clock::time_point start, end;
  PLOG("SingleThreadAlloc running...");
  
  const unsigned ITERATIONS = 3000;
  std::vector<void*> alloc_v;
  alloc_v.reserve(ITERATIONS);

  start = std::chrono::high_resolution_clock::now();
  for(unsigned i=0;i<ITERATIONS;i++) {
    void * p = allocator.alloc(i % 2);
    ASSERT_TRUE(p);
    alloc_v.push_back(p);
    //memset(p, 0xe, s);
    ((char*)p)[0] = 0xe;
  }
  end = std::chrono::high_resolution_clock::now();
  double secs = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() / 1000.0;
  PINF("Arena::SingleThreadAlloc: %lu allocs/sec", (unsigned long) (((double) ITERATIONS) / secs));

  for(auto& a : alloc_v) {
    allocator.free(a);
  }
}
#endif

#if 0
TEST_F(Libnupm_test, Basic)
{
  nupm::Vmem_allocator allocator;

  void * ptr0 = allocator.alloc(0, MB(2));
  PLOG("ptr0: %p", ptr0);
  ASSERT_TRUE(ptr0);
  memset(ptr0, 0xe, MB(2));
  void * ptr1 = allocator.alloc(1, MB(2));
  ASSERT_TRUE(ptr1);
  PLOG("ptr1: %p", ptr1);
  memset(ptr1, 0xe, MB(2));

  allocator.free(ptr0);
  allocator.free(ptr1);
  PLOG("frees OK!");
}
#endif

#ifdef RUN_VMEM_ALLOCATOR_TESTS

TEST_F(Libnupm_test, VmemSingleThreadAlloc)
{
  std::chrono::system_clock::time_point start, end;
  nupm::Vmem_allocator allocator;
  PLOG("Vmem::SingleThreadAlloc running...");
  
  const unsigned ITERATIONS = 10000000;
  std::vector<void*> alloc_v;
  alloc_v.reserve(ITERATIONS);

  start = std::chrono::high_resolution_clock::now();
  for(unsigned i=0;i<ITERATIONS;i++) {
    size_t s = genrand64_int64() % 8096;
    void * p = allocator.alloc(i % 2 /* numa zone */, s);
    ASSERT_TRUE(p);
    alloc_v.push_back(p);
    //memset(p, 0xe, s);
    //    ((char*)p)[0] = 0xe;
  }
  end = std::chrono::high_resolution_clock::now();
  double secs = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() / 1000.0;
  PINF("Vmem::SingleThreadAlloc: %lu allocs/sec", (unsigned long) (((double) ITERATIONS) / secs));

  PLOG("Vmem::SingleThreadAlloc: Freeing allocotions...");
  for(auto& a : alloc_v) {
    allocator.free(a);
  }
}

#endif

} // namespace

int main(int argc, char **argv) {
  
  ::testing::InitGoogleTest(&argc, argv);
  auto r = RUN_ALL_TESTS();

  return r;
}