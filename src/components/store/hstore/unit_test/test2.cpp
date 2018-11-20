#include "store_map.h"

#include <gtest/gtest.h>
#include <common/utils.h>
#include <api/components.h>
/* note: we do not include component source, only the API definition */
#include <api/kvstore_itf.h>

#include <random>
#include <string>
#include <sstream>
#include <stdexcept>

using namespace Component;

namespace {

// The fixture for testing class Foo.
class KVStore_test : public ::testing::Test {

  static constexpr std::size_t estimated_object_count_large = 64000000;
  /* More testing of table splits, at a performance cost */
  static constexpr std::size_t estimated_object_count_small = 1;

  static constexpr std::size_t many_count_target_large = 2000000;
  /* Shorter test: use when PMEM_IS_PMEM_FORCE=0 */
  static constexpr std::size_t many_count_target_small = 400;
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
  static bool pmem_simulated;
  static bool pmem_effective;
  static Component::IKVStore * _kvstore;
  static Component::IKVStore::pool_t pool;

  static const std::size_t estimated_object_count;

  static constexpr unsigned many_key_length = 8;
  static constexpr unsigned many_value_length = 16;

  static const std::size_t many_count_target;
  static std::size_t many_count_actual;
};

constexpr std::size_t KVStore_test::many_count_target_large;
constexpr std::size_t KVStore_test::many_count_target_small;

bool KVStore_test::pmem_simulated = getenv("PMEM_IS_PMEM_FORCE") && getenv("PMEM_IS_PMEM_FORCE") == std::string("1");
bool KVStore_test::pmem_effective = ! getenv("PMEM_IS_PMEM_FORCE") || getenv("PMEM_IS_PMEM_FORCE") == std::string("0");
Component::IKVStore * KVStore_test::_kvstore;
Component::IKVStore::pool_t KVStore_test::pool;

const std::size_t KVStore_test::estimated_object_count = pmem_simulated ? estimated_object_count_large : estimated_object_count_small;

constexpr unsigned KVStore_test::many_key_length;
constexpr unsigned KVStore_test::many_value_length;

const std::size_t KVStore_test::many_count_target = pmem_simulated ? many_count_target_large : many_count_target_small;
std::size_t KVStore_test::many_count_actual;

#define PMEM_PATH "/mnt/pmem0/pool/0/"
//#define PMEM_PATH "/dev/pmem0"

TEST_F(KVStore_test, Instantiate)
{
  /* create object instance through factory */
  auto link_library = "libcomanche-" + store_map::impl->name + ".so";
  Component::IBase * comp = Component::load_component(link_library,
                                                      store_map::impl->factory_id);

  ASSERT_TRUE(comp);
  auto fact = static_cast<IKVStore_factory *>(comp->query_interface(IKVStore_factory::iid()));

  _kvstore = fact->create("owner","name");

  fact->release_ref();
}

TEST_F(KVStore_test, RemoveOldPool)
{
  if ( _kvstore )
  {
    try
    {
      pool = _kvstore->open_pool(PMEM_PATH, "test-" + store_map::impl->name + ".pool", MB(128UL));
      if ( 0 < int64_t(pool) )
      {
        _kvstore->delete_pool(pool);
      }
    }
    catch ( Exception & )
    {
    }
  }
}

TEST_F(KVStore_test, CreatePool)
{
  ASSERT_TRUE(_kvstore);
  pool = _kvstore->create_pool(PMEM_PATH, "test-" + store_map::impl->name + ".pool", MB(128UL), 0, estimated_object_count);
  ASSERT_LT(0, int64_t(pool));
  _kvstore->close_pool(pool);
}

TEST_F(KVStore_test, PutMany)
{
  /* We will try the inserts many times, as the perishable timer will abort all but the last attempt */
  bool finished = false;

  _kvstore->debug(0, 0 /* enable */, 0);
  /*
   * We would like to generate "crashes" with some reasonable frequency,
   * but not at every store. (Every store would be too slow, at least
   * when using mmap to simulate persistent store). We use a Fibonacci
   * series to produce crashes at decreasingly frequent intervals.
   *
   * p0 and p1 produce a Fibonacci series in perishable_count
   */
  unsigned p0 = 0;
  unsigned p1 = 1;
  for (
    unsigned perishable_count = p0 + p1
    ; ! finished
    ; perishable_count = p0 + p1, p0 = p1, p1 = perishable_count
    )
  {
    unsigned fail_count = 0;
    unsigned succeed_count = 0;
    _kvstore->debug(0, 1 /* reset */, perishable_count);
    _kvstore->debug(0, 0 /* enable */, true);
    try
    {
      pool = _kvstore->open_pool(PMEM_PATH, "test-hstore.pool", MB(128));

      std::mt19937_64 r0{};

      for ( auto i = 0; i != many_count_target; ++i )
      {
        auto ukey = r0();
        std::ostringstream s;
        s << std::hex << ukey % (std::mt19937_64::result_type(1) << many_key_length*4U);
        auto key = s.str();
        key.resize(many_key_length, '.');

        auto value = std::to_string(i);
        value.resize(many_value_length, '.');
        auto r = _kvstore->put(pool, key, value.c_str(), value.length());
        if ( r == S_OK )
        {
          ++succeed_count;
        }
        else
        {
          ++fail_count;
        }
      }
      EXPECT_EQ(succeed_count + fail_count, many_count_target);
      many_count_actual = succeed_count + fail_count;
      finished = true;
      /* Done with forcing crashes */
      _kvstore->debug(0, 0 /* enable */, false);
    }
    catch ( const std::runtime_error &e )
    {
      if ( e.what() != std::string("perishable timer expired") ) { throw; }
      std::cerr << __func__ << " Perishable pass " << perishable_count << " exists " << fail_count << " inserts " << succeed_count << " total " << many_count_actual << "\n";
      /* We cannot bring the system all the way down without aborting the test case
         * driver. Come close.
       * We do not yet free and recreate the component. Perhaps we should.
       */
      _kvstore->close_pool(pool);
    }
  }
}

TEST_F(KVStore_test, ClosePool)
{
  if ( pmem_effective )
  {
    _kvstore->close_pool(pool);
  }
}

TEST_F(KVStore_test, OpenPool)
{
  ASSERT_TRUE(_kvstore);
  if ( pmem_effective )
  {
    pool = _kvstore->open_pool(PMEM_PATH, "test-hstore.pool", MB(128));
  }
  ASSERT_LT(0, int64_t(pool));
}

TEST_F(KVStore_test, Size2a)
{
  auto count = _kvstore->count(pool);
  /* count should reflect PutMany */
  EXPECT_EQ(count, many_count_actual);
}

TEST_F(KVStore_test, GetMany)
{
  std::mt19937_64 r0{};

  for ( auto i = 0; i != many_count_target; ++i )
  {
    auto ukey = r0();
    std::ostringstream s;
    s << std::hex << ukey % (std::mt19937_64::result_type(1) << many_key_length*4U);
    auto key = s.str();
    key.resize(many_key_length, '.');
    void * value = nullptr;
    size_t value_len = 0;
    auto r = _kvstore->get(pool, key, value, value_len);
    EXPECT_EQ(r, S_OK);
    EXPECT_EQ(value_len, many_value_length);
    _kvstore->free_memory(value);
  }
}

TEST_F(KVStore_test, Size2c)
{
  auto count = _kvstore->count(pool);
  /* count should reflect PutMany, and Allocate */
  EXPECT_EQ(count, many_count_actual);
}

TEST_F(KVStore_test, EraseMany)
{
  std::mt19937_64 r0{};

  auto erase_count = 0;
  for ( auto i = 0; i != many_count_target; ++i )
  {
    auto ukey = r0();
    std::ostringstream s;
    s << std::hex << ukey % (std::mt19937_64::result_type(1) << many_key_length*4U);
    auto key = s.str();
    key.resize(many_key_length, '.');
    auto r = _kvstore->erase(pool, key);
    if ( r == S_OK )
    {
      ++erase_count;
    }
  }
  EXPECT_LE(many_count_actual, erase_count);
  auto count = _kvstore->count(pool);
  EXPECT_EQ(count, 0U);
}

TEST_F(KVStore_test, DeletePool)
{
  _kvstore->delete_pool(pool);
}

} // namespace

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  auto r = RUN_ALL_TESTS();

  return r;
}
