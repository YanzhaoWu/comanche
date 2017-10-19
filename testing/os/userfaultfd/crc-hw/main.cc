#include <stdio.h>
#include <isa-l/crc.h> // Intel ISA-L
#include <common/cycles.h>
#include <common/logging.h>
#include <common/utils.h>
#include <cpuid.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
//#include "sse_intrinsics.h"
#include <smmintrin.h>

// buffer size is 1024 (= 336 * 3 + 16)
#define BLOCKSIZE  336
#define BLOCKSIZE8 (BLOCKSIZE / 8)
 
uint32_t mul_table1_336[256] = {
  0x00000000,0x8f158014,0x1bc776d9,0x94d2f6cd, 
  0x378eedb2,0xb89b6da6,0x2c499b6b,0xa35c1b7f,
  0x6f1ddb64,0xe0085b70,0x74daadbd,0xfbcf2da9,
  0x589336d6,0xd786b6c2,0x4354400f,0xcc41c01b,
  0xde3bb6c8,0x512e36dc,0xc5fcc011,0x4ae94005,
  0xe9b55b7a,0x66a0db6e,0xf2722da3,0x7d67adb7,
  0xb1266dac,0x3e33edb8,0xaae11b75,0x25f49b61,
  0x86a8801e,0x09bd000a,0x9d6ff6c7,0x127a76d3,
  0xb99b1b61,0x368e9b75,0xa25c6db8,0x2d49edac,
  0x8e15f6d3,0x010076c7,0x95d2800a,0x1ac7001e,
  0xd686c005,0x59934011,0xcd41b6dc,0x425436c8,
  0xe1082db7,0x6e1dada3,0xfacf5b6e,0x75dadb7a,
  0x67a0ada9,0xe8b52dbd,0x7c67db70,0xf3725b64,
  0x502e401b,0xdf3bc00f,0x4be936c2,0xc4fcb6d6,
  0x08bd76cd,0x87a8f6d9,0x137a0014,0x9c6f8000,
  0x3f339b7f,0xb0261b6b,0x24f4eda6,0xabe16db2,
  0x76da4033,0xf9cfc027,0x6d1d36ea,0xe208b6fe,
  0x4154ad81,0xce412d95,0x5a93db58,0xd5865b4c,
  0x19c79b57,0x96d21b43,0x0200ed8e,0x8d156d9a,
  0x2e4976e5,0xa15cf6f1,0x358e003c,0xba9b8028,
  0xa8e1f6fb,0x27f476ef,0xb3268022,0x3c330036,
  0x9f6f1b49,0x107a9b5d,0x84a86d90,0x0bbded84,
  0xc7fc2d9f,0x48e9ad8b,0xdc3b5b46,0x532edb52,
  0xf072c02d,0x7f674039,0xebb5b6f4,0x64a036e0,
  0xcf415b52,0x4054db46,0xd4862d8b,0x5b93ad9f,
  0xf8cfb6e0,0x77da36f4,0xe308c039,0x6c1d402d,
  0xa05c8036,0x2f490022,0xbb9bf6ef,0x348e76fb,
  0x97d26d84,0x18c7ed90,0x8c151b5d,0x03009b49,
  0x117aed9a,0x9e6f6d8e,0x0abd9b43,0x85a81b57,
  0x26f40028,0xa9e1803c,0x3d3376f1,0xb226f6e5,
  0x7e6736fe,0xf172b6ea,0x65a04027,0xeab5c033,
  0x49e9db4c,0xc6fc5b58,0x522ead95,0xdd3b2d81,
  0xedb48066,0x62a10072,0xf673f6bf,0x796676ab,
  0xda3a6dd4,0x552fedc0,0xc1fd1b0d,0x4ee89b19,
  0x82a95b02,0x0dbcdb16,0x996e2ddb,0x167badcf,
  0xb527b6b0,0x3a3236a4,0xaee0c069,0x21f5407d,
  0x338f36ae,0xbc9ab6ba,0x28484077,0xa75dc063,
  0x0401db1c,0x8b145b08,0x1fc6adc5,0x90d32dd1,
  0x5c92edca,0xd3876dde,0x47559b13,0xc8401b07,
  0x6b1c0078,0xe409806c,0x70db76a1,0xffcef6b5,
  0x542f9b07,0xdb3a1b13,0x4fe8edde,0xc0fd6dca,
  0x63a176b5,0xecb4f6a1,0x7866006c,0xf7738078,
  0x3b324063,0xb427c077,0x20f536ba,0xafe0b6ae,
  0x0cbcadd1,0x83a92dc5,0x177bdb08,0x986e5b1c,
  0x8a142dcf,0x0501addb,0x91d35b16,0x1ec6db02,
  0xbd9ac07d,0x328f4069,0xa65db6a4,0x294836b0,
  0xe509f6ab,0x6a1c76bf,0xfece8072,0x71db0066,
  0xd2871b19,0x5d929b0d,0xc9406dc0,0x4655edd4,
  0x9b6ec055,0x147b4041,0x80a9b68c,0x0fbc3698,
  0xace02de7,0x23f5adf3,0xb7275b3e,0x3832db2a,
  0xf4731b31,0x7b669b25,0xefb46de8,0x60a1edfc,
  0xc3fdf683,0x4ce87697,0xd83a805a,0x572f004e,
  0x4555769d,0xca40f689,0x5e920044,0xd1878050,
  0x72db9b2f,0xfdce1b3b,0x691cedf6,0xe6096de2,
  0x2a48adf9,0xa55d2ded,0x318fdb20,0xbe9a5b34,
  0x1dc6404b,0x92d3c05f,0x06013692,0x8914b686,
  0x22f5db34,0xade05b20,0x3932aded,0xb6272df9,
  0x157b3686,0x9a6eb692,0x0ebc405f,0x81a9c04b,
  0x4de80050,0xc2fd8044,0x562f7689,0xd93af69d,
  0x7a66ede2,0xf5736df6,0x61a19b3b,0xeeb41b2f,
  0xfcce6dfc,0x73dbede8,0xe7091b25,0x681c9b31,
  0xcb40804e,0x4455005a,0xd087f697,0x5f927683,
  0x93d3b698,0x1cc6368c,0x8814c041,0x07014055,
  0xa45d5b2a,0x2b48db3e,0xbf9a2df3,0x308fade7
};

uint32_t mul_table1_672[256] = {
  0x00000000,0xe417f38a,0xcdc391e5,0x29d4626f,
  0x9e6b553b,0x7a7ca6b1,0x53a8c4de,0xb7bf3754,
  0x393adc87,0xdd2d2f0d,0xf4f94d62,0x10eebee8,
  0xa75189bc,0x43467a36,0x6a921859,0x8e85ebd3,
  0x7275b90e,0x96624a84,0xbfb628eb,0x5ba1db61,
  0xec1eec35,0x08091fbf,0x21dd7dd0,0xc5ca8e5a,
  0x4b4f6589,0xaf589603,0x868cf46c,0x629b07e6,
  0xd52430b2,0x3133c338,0x18e7a157,0xfcf052dd,
  0xe4eb721c,0x00fc8196,0x2928e3f9,0xcd3f1073,
  0x7a802727,0x9e97d4ad,0xb743b6c2,0x53544548,
  0xddd1ae9b,0x39c65d11,0x10123f7e,0xf405ccf4,
  0x43bafba0,0xa7ad082a,0x8e796a45,0x6a6e99cf,
  0x969ecb12,0x72893898,0x5b5d5af7,0xbf4aa97d,
  0x08f59e29,0xece26da3,0xc5360fcc,0x2121fc46,
  0xafa41795,0x4bb3e41f,0x62678670,0x867075fa,
  0x31cf42ae,0xd5d8b124,0xfc0cd34b,0x181b20c1,
  0xcc3a92c9,0x282d6143,0x01f9032c,0xe5eef0a6,
  0x5251c7f2,0xb6463478,0x9f925617,0x7b85a59d,
  0xf5004e4e,0x1117bdc4,0x38c3dfab,0xdcd42c21,
  0x6b6b1b75,0x8f7ce8ff,0xa6a88a90,0x42bf791a,
  0xbe4f2bc7,0x5a58d84d,0x738cba22,0x979b49a8,
  0x20247efc,0xc4338d76,0xede7ef19,0x09f01c93,
  0x8775f740,0x636204ca,0x4ab666a5,0xaea1952f,
  0x191ea27b,0xfd0951f1,0xd4dd339e,0x30cac014,
  0x28d1e0d5,0xccc6135f,0xe5127130,0x010582ba,
  0xb6bab5ee,0x52ad4664,0x7b79240b,0x9f6ed781,
  0x11eb3c52,0xf5fccfd8,0xdc28adb7,0x383f5e3d,
  0x8f806969,0x6b979ae3,0x4243f88c,0xa6540b06,
  0x5aa459db,0xbeb3aa51,0x9767c83e,0x73703bb4,
  0xc4cf0ce0,0x20d8ff6a,0x090c9d05,0xed1b6e8f,
  0x639e855c,0x878976d6,0xae5d14b9,0x4a4ae733,
  0xfdf5d067,0x19e223ed,0x30364182,0xd421b208,
  0x9d995363,0x798ea0e9,0x505ac286,0xb44d310c,
  0x03f20658,0xe7e5f5d2,0xce3197bd,0x2a266437,
  0xa4a38fe4,0x40b47c6e,0x69601e01,0x8d77ed8b,
  0x3ac8dadf,0xdedf2955,0xf70b4b3a,0x131cb8b0,
  0xefecea6d,0x0bfb19e7,0x222f7b88,0xc6388802,
  0x7187bf56,0x95904cdc,0xbc442eb3,0x5853dd39,
  0xd6d636ea,0x32c1c560,0x1b15a70f,0xff025485,
  0x48bd63d1,0xacaa905b,0x857ef234,0x616901be,
  0x7972217f,0x9d65d2f5,0xb4b1b09a,0x50a64310,
  0xe7197444,0x030e87ce,0x2adae5a1,0xcecd162b,
  0x4048fdf8,0xa45f0e72,0x8d8b6c1d,0x699c9f97,
  0xde23a8c3,0x3a345b49,0x13e03926,0xf7f7caac,
  0x0b079871,0xef106bfb,0xc6c40994,0x22d3fa1e,
  0x956ccd4a,0x717b3ec0,0x58af5caf,0xbcb8af25,
  0x323d44f6,0xd62ab77c,0xfffed513,0x1be92699,
  0xac5611cd,0x4841e247,0x61958028,0x858273a2,
  0x51a3c1aa,0xb5b43220,0x9c60504f,0x7877a3c5,
  0xcfc89491,0x2bdf671b,0x020b0574,0xe61cf6fe,
  
  0x68991d2d,0x8c8eeea7,0xa55a8cc8,0x414d7f42,
  0xf6f24816,0x12e5bb9c,0x3b31d9f3,0xdf262a79,
  0x23d678a4,0xc7c18b2e,0xee15e941,0x0a021acb,
  0xbdbd2d9f,0x59aade15,0x707ebc7a,0x94694ff0,
  0x1aeca423,0xfefb57a9,0xd72f35c6,0x3338c64c,
  0x8487f118,0x60900292,0x494460fd,0xad539377,
  0xb548b3b6,0x515f403c,0x788b2253,0x9c9cd1d9,
  0x2b23e68d,0xcf341507,0xe6e07768,0x02f784e2,
  0x8c726f31,0x68659cbb,0x41b1fed4,0xa5a60d5e,
  0x12193a0a,0xf60ec980,0xdfdaabef,0x3bcd5865,
  0xc73d0ab8,0x232af932,0x0afe9b5d,0xeee968d7,
  0x59565f83,0xbd41ac09,0x9495ce66,0x70823dec,
  0xfe07d63f,0x1a1025b5,0x33c447da,0xd7d3b450,
  0x606c8304,0x847b708e,0xadaf12e1,0x49b8e16b  
};

/* WARNING: this could be patented by Intel */
uint32_t crc_1024_c(uint8_t *buffer, uint32_t initval)
{
  uint64_t crc0, crc1, crc2, tmp;
  uint64_t *p_buffer;
 
  p_buffer = (uint64_t*)buffer;
  crc1 = crc2 = 0;
 
  // Do first 8 bytes here for better pipelining
  crc0 = _mm_crc32_u64(initval, p_buffer[0]);
 
  for(int i = 0; i < 42; i++){
    crc1 = _mm_crc32_u64(crc1, p_buffer[1 + 1*BLOCKSIZE8 + i]);
    crc2 = _mm_crc32_u64(crc2, p_buffer[1 + 2*BLOCKSIZE8 + i]);
    crc0 = _mm_crc32_u64(crc0, p_buffer[1 + 0*BLOCKSIZE8 + i]);
  }
 
  // merge in crc1
  tmp  = p_buffer[127];
  tmp ^= mul_table1_336[crc1 & 0xFF];
  tmp ^= ((uint64_t)mul_table1_336[(crc1 >>  8) & 0xFF]) <<  8;
  tmp ^= ((uint64_t)mul_table1_336[(crc1 >> 16) & 0xFF]) << 16;
  tmp ^= ((uint64_t)mul_table1_336[(crc1 >> 24) & 0xFF]) << 24;
 
  // merge in crc0
  tmp ^= mul_table1_672[crc0 & 0xFF];
  tmp ^= ((uint64_t)mul_table1_672[(crc0 >>  8) & 0xFF]) <<  8;
  tmp ^= ((uint64_t)mul_table1_672[(crc0 >> 16) & 0xFF]) << 16;
  tmp ^= ((uint64_t)mul_table1_672[(crc0 >> 24) & 0xFF]) << 24;
 
  return (uint32_t) _mm_crc32_u64(crc2, tmp);
}

bool check_sse4()
{
  unsigned int eax, ebx, ecx, edx;
  __get_cpuid(1, &eax, &ebx, &ecx, &edx);
  return (ecx & bit_SSE4_2);
}

int main()
{
  size_t data_size = KB(4);
  void * data = malloc(data_size);

  uint64_t * p = (uint64_t *) data;
  for(unsigned i=0;i<data_size/sizeof(uint64_t);i++) {
    p[i] = rand();
  }

  if(check_sse4())
    PLOG("SSE4 is supported OK.");
  else 
    PLOG("SSE4 not supported!");

  cpu_time_t start = rdtsc();
  uint8_t * q = (uint8_t *) data;
  size_t segments = KB(4)/1024;
  
  uint32_t crc = 0;
  for(size_t i=0;i<segments;i++) {
    crc =  crc_1024_c(q,crc);
    q+=1024;
  }
  cpu_time_t end = rdtsc();

  PLOG("crc:%x",crc);
  PLOG("delta: %ld cycles", end - start);

}

void test_isal() {

  size_t data_size = 4096;
  void * data = malloc(data_size);

  uint64_t * p = (uint64_t *) data;
  for(unsigned i=0;i<data_size/sizeof(uint64_t);i++) {
    p[i] = rand();
  }

  cpu_time_t start = rdtsc();
  uint32_t crc = crc32_ieee(0,(const unsigned char *)data,data_size);
  cpu_time_t end = rdtsc();

  PLOG("crc:%x",crc);
  PLOG("delta: %ld cycles", end - start);
}
