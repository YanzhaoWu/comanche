#include <gtest/gtest.h>

/* note: we do not include component source, only the API definition */
#include <api/components.h>
#include <api/kvstore_itf.h>
#include <common/utils.h>
#include <common/str_utils.h>
#include <core/task.h>
//#include <chrono>
#include <iostream>

#define FILESTORE_PATH "libcomanche-storefile.so"

using namespace Component;

TEST(BasicFunctionality, PutGet)
{
    printf("load_test running\n");

    Component::IKVStore * g_store;
    Component::IBase * comp;

    printf("variables declared\n");

    comp = Component::load_component(FILESTORE_PATH, Component::filestore_factory);
    assert(comp); // TODO: assert message possible here?

    printf("component loaded\n");

    IKVStore_factory * fact = (IKVStore_factory *)comp->query_interface(IKVStore_factory::iid());

    printf("query_interface completed\n");

    g_store = fact->create("owner", "name");  // TODO: what does this do?

    printf("create done\n");

    printf("setting up cores\n");
    
    cpu_mask_t cpus;
    unsigned core = 1;
    static unsigned core_count = 1;

    for(unsigned core = 0; core < core_count; core++)
    {
        cpus.add_core(core);
    }

    printf("starting tests\n");
//    Core::Per_core_tasking<Experiment_Put, Component::IKVStore*> exp(cpus, g_store);
    std::string pool_path = "./data";
    std::string pool_name = "test.pool.0";
    size_t pool_size = MB(100);
    const Component::IKVStore::pool_t pool = g_store->create_pool(pool_path, pool_name, pool_size);

    // randomly generate key and value
    int key_length = 8;
    int value_length = 64;
    const std::string key = Common::random_string(key_length);
    std::string value = Common::random_string(value_length);
    printf("random value: %s, size %lu\n", value.c_str(), value_length);

    printf("generated random key and value\n");
    status_t rc = g_store->put(pool, key, value.c_str(), value_length);
//    status_t rc = 0;
    ASSERT_EQ(rc, S_OK); 

    void * pval;
    size_t pval_len;
    
    rc  = g_store->get(pool, key, pval, pval_len);

    printf("pval = %s, size %lu\n", static_cast<std::string*>(pval), pval_len);

    ASSERT_EQ(rc, S_OK);
    //BOOST_CHECK(strcmp(((const char*)pval), value.c_str()) == 0);
    ASSERT_STREQ((const char*)pval, value.c_str());

    free(pval);

    printf("testing done\n");
    fact->release_ref();  // TODO: what does this do?

    printf("release_ref done\n");
}

int main(int argc, char **argv) 
{
    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}

