#include "stdafx.h"
#include "APKKiller.h"

int RegisterFunctions(JNIEnv *env) {
    JNINativeMethod methods[2];
    methods[0].name = "Start";
    methods[0].signature = "(Landroid/content/Context;)V";
    methods[0].fnPtr = (void *) APKKill;

    methods[1].name = "nativeInvoke";
    methods[1].signature = "(Ljava/lang/Object;Ljava/lang/reflect/Method;[Ljava/lang/Object;)Ljava/lang/Object;";
    methods[1].fnPtr = (void *) nativeInvoke;

    jclass clazz = env->FindClass("com/kuro/APKKiller");
    if (!clazz)
        return -1;

    if (env->RegisterNatives(clazz, methods, sizeof(methods) / sizeof(methods[0])) != 0)
        return -1;

    return 0;
}

JNIEXPORT jint JNI_OnLoad(JavaVM *vm, void *reserved) {
    JNIEnv *env;
    if (vm->GetEnv(reinterpret_cast<void **>(&env), JNI_VERSION_1_6) != JNI_OK) {
        return -1;
    }

    if (RegisterFunctions(env) != 0) {
        return -1;
    }
    return JNI_VERSION_1_6;
}