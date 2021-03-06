/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
/* Header for class com_yahoo_ycsb_db_DawnClient */

#ifndef _Included_com_yahoo_ycsb_db_DawnClient
#define _Included_com_yahoo_ycsb_db_DawnClient
#ifdef __cplusplus
extern "C" {
#endif
/*
 * Class:     com_yahoo_ycsb_db_DawnClient
 * Method:    init
 * Signature: (ILjava/lang/String;Ljava/lang/String;Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_com_yahoo_ycsb_db_DawnClient_init(JNIEnv *,
                                                              jobject,
                                                              jint,
                                                              jstring,
                                                              jstring,
                                                              jstring);

/*
 * Class:     com_yahoo_ycsb_db_DawnClient
 * Method:    put
 * Signature: (Ljava/lang/String;Ljava/lang/String;[BZ)I
 */
JNIEXPORT jint JNICALL Java_com_yahoo_ycsb_db_DawnClient_put(JNIEnv *,
                                                             jobject,
                                                             jstring,
                                                             jstring,
                                                             jbyteArray,
                                                             jboolean);

/*
 * Class:     com_yahoo_ycsb_db_DawnClient
 * Method:    get
 * Signature: (Ljava/lang/String;Ljava/lang/String;[BZ)I
 */
JNIEXPORT jint JNICALL Java_com_yahoo_ycsb_db_DawnClient_get(JNIEnv *,
                                                             jobject,
                                                             jstring,
                                                             jstring,
                                                             jbyteArray,
                                                             jboolean);

/*
 * Class:     com_yahoo_ycsb_db_DawnClient
 * Method:    erase
 * Signature: (Ljava/lang/String;Ljava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_com_yahoo_ycsb_db_DawnClient_erase(JNIEnv *,
                                                               jobject,
                                                               jstring,
                                                               jstring);

/*
 * Class:     com_yahoo_ycsb_db_DawnClient
 * Method:    clean
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_com_yahoo_ycsb_db_DawnClient_clean(JNIEnv *,
                                                               jobject);
#ifdef __cplusplus
}
#endif
#endif
