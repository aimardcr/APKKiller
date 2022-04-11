#include <stdio.h>
#include <iostream>
#include <vector>
#include <jni.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dlfcn.h>

#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <android/log.h>

#include "ElfImg.h"

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, "APKKiller", __VA_ARGS__)

#define apk_asset_path "original.apk" // assets/original.apk
#define apk_fake_name "original.apk" // /data/data/<package_name/cache/original.apk
std::vector<std::vector<uint8_t>> apk_signatures {{0x30, 0x82, 0x03, 0x51, 0x30, 0x82, 0x02, 0x39, 0xA0, 0x03, 0x02, 0x01, 0x02, 0x02, 0x04, 0x2F, 0x81, 0x91, 0x99, 0x30, 0x0D, 0x06, 0x09, 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x01, 0x0B, 0x05, 0x00, 0x30, 0x59, 0x31, 0x0B, 0x30, 0x09, 0x06, 0x03, 0x55, 0x04, 0x06, 0x13, 0x02, 0x49, 0x44, 0x31, 0x0D, 0x30, 0x0B, 0x06, 0x03, 0x55, 0x04, 0x08, 0x13, 0x04, 0x42, 0x61, 0x6C, 0x69, 0x31, 0x0F, 0x30, 0x0D, 0x06, 0x03, 0x55, 0x04, 0x07, 0x13, 0x06, 0x42, 0x61, 0x64, 0x75, 0x6E, 0x67, 0x31, 0x12, 0x30, 0x10, 0x06, 0x03, 0x55, 0x04, 0x0A, 0x13, 0x09, 0x4B, 0x75, 0x72, 0x6F, 0x20, 0x54, 0x65, 0x6B, 0x2E, 0x31, 0x16, 0x30, 0x14, 0x06, 0x03, 0x55, 0x04, 0x03, 0x13, 0x0D, 0x41, 0x69, 0x6D, 0x61, 0x72, 0x20, 0x41, 0x64, 0x68, 0x69, 0x74, 0x79, 0x61, 0x30, 0x1E, 0x17, 0x0D, 0x32, 0x32, 0x30, 0x33, 0x32, 0x30, 0x32, 0x32, 0x33, 0x35, 0x35, 0x31, 0x5A, 0x17, 0x0D, 0x34, 0x37, 0x30, 0x33, 0x31, 0x34, 0x32, 0x32, 0x33, 0x35, 0x35, 0x31, 0x5A, 0x30, 0x59, 0x31, 0x0B, 0x30, 0x09, 0x06, 0x03, 0x55, 0x04, 0x06, 0x13, 0x02, 0x49, 0x44, 0x31, 0x0D, 0x30, 0x0B, 0x06, 0x03, 0x55, 0x04, 0x08, 0x13, 0x04, 0x42, 0x61, 0x6C, 0x69, 0x31, 0x0F, 0x30, 0x0D, 0x06, 0x03, 0x55, 0x04, 0x07, 0x13, 0x06, 0x42, 0x61, 0x64, 0x75, 0x6E, 0x67, 0x31, 0x12, 0x30, 0x10, 0x06, 0x03, 0x55, 0x04, 0x0A, 0x13, 0x09, 0x4B, 0x75, 0x72, 0x6F, 0x20, 0x54, 0x65, 0x6B, 0x2E, 0x31, 0x16, 0x30, 0x14, 0x06, 0x03, 0x55, 0x04, 0x03, 0x13, 0x0D, 0x41, 0x69, 0x6D, 0x61, 0x72, 0x20, 0x41, 0x64, 0x68, 0x69, 0x74, 0x79, 0x61, 0x30, 0x82, 0x01, 0x22, 0x30, 0x0D, 0x06, 0x09, 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x01, 0x01, 0x05, 0x00, 0x03, 0x82, 0x01, 0x0F, 0x00, 0x30, 0x82, 0x01, 0x0A, 0x02, 0x82, 0x01, 0x01, 0x00, 0x8C, 0x9F, 0xE6, 0x34, 0x79, 0x99, 0xC6, 0x58, 0x0E, 0xD9, 0x80, 0xBB, 0x9B, 0xE0, 0x9B, 0x10, 0x94, 0xDA, 0xFF, 0x0E, 0xC1, 0xE3, 0x22, 0xCC, 0xEC, 0x74, 0x11, 0x46, 0x28, 0x0C, 0x6C, 0x91, 0x92, 0x55, 0x67, 0x09, 0x61, 0x8C, 0x88, 0xE7, 0x01, 0x4D, 0x44, 0x89, 0x1F, 0x8C, 0xB0, 0xE3, 0x39, 0x52, 0x42, 0x83, 0xB3, 0xBE, 0x79, 0x45, 0x2E, 0x6B, 0x01, 0x88, 0xC4, 0xF5, 0x70, 0x26, 0xD7, 0x1D, 0x7C, 0xD4, 0xF9, 0xD6, 0xB9, 0xEA, 0xFE, 0x6D, 0x5B, 0x53, 0x60, 0x3F, 0xE3, 0x72, 0x9C, 0x50, 0xFB, 0xA3, 0x98, 0x1E, 0x78, 0x88, 0x73, 0x6C, 0xDE, 0x49, 0x94, 0xBD, 0x7D, 0x7B, 0xDA, 0x0C, 0xEF, 0x83, 0x17, 0xC3, 0xD9, 0xCD, 0x17, 0xB6, 0x07, 0x8B, 0x4C, 0xAA, 0x66, 0x69, 0xC7, 0xC2, 0x5E, 0x48, 0x30, 0xE1, 0xEA, 0xAD, 0x8B, 0x89, 0x93, 0xDE, 0xD5, 0x6F, 0xDC, 0x69, 0x6F, 0x1F, 0xB2, 0x11, 0xDF, 0xB0, 0x0D, 0x7C, 0xD2, 0x00, 0x01, 0x88, 0x8E, 0xE3, 0x8C, 0x3E, 0xC5, 0x9E, 0x9C, 0x69, 0x85, 0xB2, 0x3D, 0x42, 0x70, 0xF2, 0x69, 0xF9, 0x5B, 0xE3, 0xC7, 0x3F, 0x15, 0x43, 0x6F, 0x8F, 0xD9, 0xE7, 0x8C, 0xBB, 0x04, 0x90, 0x42, 0xAC, 0xCB, 0xCE, 0x52, 0x69, 0xAF, 0x63, 0xA4, 0xAD, 0xF7, 0x3F, 0xAC, 0xC9, 0xD6, 0xE5, 0x09, 0x96, 0xE9, 0x0F, 0x8B, 0xB5, 0x06, 0x4C, 0xB1, 0x7E, 0xCC, 0xBE, 0xD0, 0x05, 0xFE, 0xD8, 0x6C, 0x5C, 0xCE, 0xFC, 0x93, 0x7D, 0x8E, 0xE1, 0x24, 0x7A, 0xF9, 0x8E, 0x2F, 0x1F, 0xAD, 0xD2, 0x8B, 0x8D, 0x83, 0x40, 0xF7, 0xA9, 0x77, 0x36, 0xBF, 0x51, 0x3D, 0x98, 0x01, 0x3B, 0x80, 0xEF, 0x5A, 0x13, 0xCF, 0xF9, 0x42, 0xDF, 0xE8, 0x1E, 0x9A, 0x6A, 0x0C, 0xF2, 0x0A, 0x88, 0x00, 0x41, 0x36, 0x02, 0x89, 0xED, 0xBE, 0xCB, 0x02, 0x03, 0x01, 0x00, 0x01, 0xA3, 0x21, 0x30, 0x1F, 0x30, 0x1D, 0x06, 0x03, 0x55, 0x1D, 0x0E, 0x04, 0x16, 0x04, 0x14, 0x4A, 0x26, 0xAF, 0x44, 0x37, 0x07, 0xB6, 0x2B, 0x80, 0x0E, 0xC9, 0xC9, 0xB5, 0x13, 0x08, 0x0E, 0x8F, 0x3B, 0x6D, 0xC6, 0x30, 0x0D, 0x06, 0x09, 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x01, 0x0B, 0x05, 0x00, 0x03, 0x82, 0x01, 0x01, 0x00, 0x7E, 0x38, 0x10, 0xCA, 0xC4, 0xAB, 0x6E, 0x8C, 0xAB, 0xCC, 0xB9, 0xD9, 0xA6, 0xD1, 0x04, 0x61, 0xC3, 0x58, 0xAD, 0xAC, 0x4F, 0x23, 0xC6, 0xC5, 0x83, 0x99, 0x7D, 0x5E, 0xAA, 0x92, 0xEB, 0x45, 0xD7, 0xAB, 0xA8, 0xC0, 0xA3, 0x1A, 0x44, 0x14, 0x68, 0x7D, 0x2C, 0xCC, 0xE9, 0xEB, 0xD3, 0xAE, 0x98, 0xD0, 0x7E, 0x4A, 0xC7, 0x74, 0x66, 0xA6, 0xB7, 0x2E, 0xC1, 0xE2, 0x82, 0xF1, 0x9F, 0x93, 0x4B, 0x13, 0x20, 0x68, 0xCB, 0xDD, 0x40, 0xEC, 0x9F, 0xC5, 0x9C, 0xB7, 0xD4, 0x51, 0x5F, 0xEF, 0xEA, 0x99, 0x46, 0x86, 0x90, 0x54, 0xF1, 0xEC, 0xDB, 0xE1, 0x63, 0x3D, 0x83, 0xBF, 0x3A, 0xEF, 0x0A, 0x11, 0x98, 0x22, 0x89, 0x3E, 0x17, 0xBE, 0x12, 0x69, 0x05, 0xFE, 0x2E, 0x14, 0x75, 0x9E, 0xB9, 0x3D, 0x7A, 0x4E, 0x8D, 0xDA, 0xEC, 0x50, 0x70, 0xCC, 0x4A, 0xB5, 0xF5, 0xB1, 0x0C, 0x52, 0xBC, 0xF6, 0xBB, 0xC2, 0x32, 0x11, 0xF4, 0x17, 0xE8, 0x95, 0x3E, 0x2B, 0x54, 0xD8, 0x30, 0x78, 0xCB, 0x2E, 0xBA, 0x94, 0xA6, 0x96, 0x00, 0x21, 0x7B, 0xA9, 0x52, 0x13, 0x05, 0x9F, 0x4C, 0xC0, 0x94, 0x0B, 0xC7, 0xFB, 0xE6, 0xA8, 0x7F, 0x02, 0x60, 0x64, 0x18, 0xB8, 0x16, 0x44, 0x22, 0xCB, 0xB6, 0xE2, 0x03, 0xC6, 0x3B, 0x19, 0x80, 0x85, 0xA2, 0x5D, 0xC9, 0x23, 0x53, 0xE2, 0xE4, 0xF4, 0x1C, 0x1F, 0x67, 0xE6, 0xE6, 0xC0, 0xB9, 0x50, 0xE1, 0x61, 0x53, 0x27, 0x2A, 0x74, 0x2E, 0x9C, 0xA7, 0x45, 0x01, 0xA9, 0x9D, 0x44, 0x18, 0x62, 0xDF, 0x57, 0xEB, 0xE7, 0x65, 0xB3, 0x52, 0x2C, 0x52, 0xBD, 0xDA, 0xC4, 0x90, 0x48, 0x6E, 0x0B, 0xEF, 0x99, 0xB3, 0xBE, 0x82, 0xD8, 0x46, 0xC6, 0x3B, 0x7F, 0x4C, 0x84, 0x67, 0xE6, 0x87, 0xD5, 0xC2, 0x70, 0x74, 0x2B, 0x75, 0x17, 0x6A, 0x49}};

namespace APKKiller {
    JNIEnv *g_env;
    jstring g_apkPath;
    jobject g_packageManager;
    std::string g_apkPkg;

    class Reference {
    public:
        jobject reference;
    public:
        Reference(jobject reference) {
            this->reference = g_env->NewGlobalRef(reference);
        }

        jobject getObj() {
            return g_env->CallObjectMethod(reference, g_env->GetMethodID(g_env->FindClass("java/lang/ref/Reference"), "get", "()Ljava/lang/Object;"));
        }
    };

    class WeakReference : public Reference {
    public:
        WeakReference(jobject weakReference) : Reference(weakReference) {
        }

        static jobject Create(jobject obj) {
            auto weakReferenceClass = g_env->FindClass("java/lang/ref/WeakReference");
            auto weakReferenceClassConstructor = g_env->GetMethodID(weakReferenceClass, "<init>", "(Ljava/lang/Object;)V");
            return g_env->NewObject(weakReferenceClass, weakReferenceClassConstructor, obj);
        }
    };

    class ArrayList {
    private:
        jobject arrayList;
    public:
        ArrayList(jobject arrayList) {
            this->arrayList = g_env->NewGlobalRef(arrayList);
        }

        jobject getObj() {
            return arrayList;
        }

        jobject get(int index) {
            return g_env->CallObjectMethod(arrayList, g_env->GetMethodID(g_env->FindClass("java/util/ArrayList"), "get", "(I)Ljava/lang/Object;"), index);
        }

        void set(int index, jobject value) {
            g_env->CallObjectMethod(arrayList, g_env->GetMethodID(g_env->FindClass("java/util/ArrayList"), "set", "(ILjava/lang/Object;)Ljava/lang/Object;"), index, value);
        }

        int size() {
            return g_env->CallIntMethod(arrayList, g_env->GetMethodID(g_env->FindClass("java/util/ArrayList"), "size", "()I"));
        }
    };

    class ArrayMap {
    private:
        jobject arrayMap;
    public:
        ArrayMap(jobject arrayMap) {
            this->arrayMap = g_env->NewGlobalRef(arrayMap);
        }

        jobject getObj() {
            return arrayMap;
        }

        jobject keyAt(int index) {
            return g_env->CallObjectMethod(arrayMap, g_env->GetMethodID(g_env->FindClass("android/util/ArrayMap"), "keyAt", "(I)Ljava/lang/Object;"), index);
        }

        jobject valueAt(int index) {
            return g_env->CallObjectMethod(arrayMap, g_env->GetMethodID(g_env->FindClass("android/util/ArrayMap"), "valueAt", "(I)Ljava/lang/Object;"), index);
        }

        jobject setValueAt(int index, jobject value) {
            return g_env->CallObjectMethod(arrayMap, g_env->GetMethodID(g_env->FindClass("android/util/ArrayMap"), "setValueAt", "(ILjava/lang/Object;)Ljava/lang/Object;"), index, value);
        }

        int size() {
            return g_env->CallIntMethod(arrayMap, g_env->GetMethodID(g_env->FindClass("android/util/ArrayMap"), "size", "()I"));
        }
    };

    class Field {
    private:
        jobject field;
    public:
        Field(jobject field) {
            this->field = g_env->NewGlobalRef(field);
        }

        jobject getField() {
            return field;
        }

        void setAccessible(jboolean accessible) {
            g_env->CallVoidMethod(field, g_env->GetMethodID(g_env->FindClass("java/lang/reflect/Field"), "setAccessible", "(Z)V"), accessible);
        }

        jobject get(jobject object) {
            return g_env->CallObjectMethod(field, g_env->GetMethodID(g_env->FindClass("java/lang/reflect/Field"), "get", "(Ljava/lang/Object;)Ljava/lang/Object;"), object);
        }

        void set(jobject object, jobject value) {
            g_env->CallVoidMethod(field, g_env->GetMethodID(g_env->FindClass("java/lang/reflect/Field"), "set", "(Ljava/lang/Object;Ljava/lang/Object;)V"), object, value);
        }
    };

    class Method {
    private:
        jobject method;
    public:
        Method(jobject method) {
            this->method = g_env->NewGlobalRef(method);
        }

        jobject getMethod() {
            return method;
        }

        void setAccessible(jboolean accessible) {
            g_env->CallVoidMethod(method, g_env->GetMethodID(g_env->FindClass("java/lang/reflect/Method"), "setAccessible", "(Z)V"), accessible);
        }

        jobject invoke(jobject object, jobjectArray args = 0) {
            return g_env->CallObjectMethod(method, g_env->GetMethodID(g_env->FindClass("java/lang/reflect/Method"), "invoke", "(Ljava/lang/Object;[Ljava/lang/Object;)Ljava/lang/Object;"), object, args);
        }
    };

    class Class {
    private:
        jobject clazz;
    public:
        Class(jobject clazz) {
            this->clazz = g_env->NewGlobalRef(clazz);
        }

        jobject getClass() {
            return clazz;
        }

        static Class *forName(const char *s) {
            auto str = g_env->NewStringUTF(s);

            auto classClass = g_env->FindClass("java/lang/Class");
            auto forNameMethod = g_env->GetStaticMethodID(classClass, "forName", "(Ljava/lang/String;)Ljava/lang/Class;");

            auto clazz = new Class(g_env->CallStaticObjectMethod(classClass, forNameMethod, str));

            return clazz;
        }

        Field *getDeclaredField(const char *s) {
            auto str = g_env->NewStringUTF(s);

            auto classClass = g_env->FindClass("java/lang/Class");
            auto getDeclaredFieldMethod = g_env->GetMethodID(classClass, "getDeclaredField", "(Ljava/lang/String;)Ljava/lang/reflect/Field;");

            auto field = new Field(g_env->CallObjectMethod(clazz, getDeclaredFieldMethod, str));

            if (g_env->ExceptionCheck()) {
                g_env->ExceptionDescribe();
                g_env->ExceptionClear();
            }

            return field;
        }

        Method *getDeclaredMethod(const char *s, jobjectArray args = 0) {
            auto str = g_env->NewStringUTF(s);

            auto classClass = g_env->FindClass("java/lang/Class");
            auto getDeclaredMethodMethod = g_env->GetMethodID(classClass, "getDeclaredMethod", "(Ljava/lang/String;[Ljava/lang/Class;)Ljava/lang/reflect/Method;");

            auto method = new Method(g_env->CallObjectMethod(clazz, getDeclaredMethodMethod, str, args));

            if (g_env->ExceptionCheck()) {
                g_env->ExceptionDescribe();
                g_env->ExceptionClear();
            }

            return method;
        }
    };
}

using namespace APKKiller;

int getAPILevel() {
    static int api_level = -1;
    if (api_level == -1) {
        char prop_value[256];
        __system_property_get("ro.build.version.sdk", prop_value);
        api_level = atoi(prop_value);
    }
    return api_level;
}

jobject getApplicationContext(jobject obj) {
    auto contextWrapperClass = g_env->FindClass("android/content/ContextWrapper");
    auto getApplicationContextMethod = g_env->GetMethodID(contextWrapperClass, "getApplicationContext", "()Landroid/content/Context;");
    return g_env->CallObjectMethod(obj, getApplicationContextMethod);
}

jobject getPackageManager(jobject obj) {
    auto contextClass = g_env->FindClass("android/content/Context");
    auto getPackageManagerMethod = g_env->GetMethodID(contextClass, "getPackageManager", "()Landroid/content/pm/PackageManager;");
    return g_env->CallObjectMethod(obj, getPackageManagerMethod);
}

std::string getPackageName(jobject obj) {
    auto contextClass = g_env->FindClass("android/content/Context");
    auto getPackageNameMethod = g_env->GetMethodID(contextClass, "getPackageName", "()Ljava/lang/String;");
    auto packageName = (jstring) g_env->CallObjectMethod(obj, getPackageNameMethod);
    return g_env->GetStringUTFChars(packageName, 0);
}

bool Class_isInstanceOf(jobject obj, const char *className) {
    auto clazz = Class::forName(className);
    auto isInstanceOfMethod = g_env->GetMethodID(g_env->FindClass("java/lang/Class"), "isInstance", "(Ljava/lang/Object;)Z");
    return g_env->CallBooleanMethod(clazz->getClass(), isInstanceOfMethod, obj);
}

void patch_ApplicationInfo(jobject obj) {
    if (!obj) return;
    LOGI("-------- Patching ApplicationInfo - %p", obj);
    auto applicationInfoClass = Class::forName("android.content.pm.ApplicationInfo");

    auto sourceDirField = applicationInfoClass->getDeclaredField("sourceDir");
    sourceDirField->setAccessible(true);

    auto publicSourceDirField = applicationInfoClass->getDeclaredField("publicSourceDir");
    publicSourceDirField->setAccessible(true);

    sourceDirField->set(obj, g_apkPath);
    publicSourceDirField->set(obj, g_apkPath);

    if (getAPILevel() >= 21) {
        auto splitSourceDirsField = applicationInfoClass->getDeclaredField("splitSourceDirs");
        splitSourceDirsField->setAccessible(true);
        auto splitPublicSourceDirsField = applicationInfoClass->getDeclaredField("splitPublicSourceDirs");
        splitPublicSourceDirsField->setAccessible(true);

        // print both source dirs
        auto splitSourceDirs = (jobjectArray) splitSourceDirsField->get(obj); // jstringArray
        auto splitPublicSourceDirs = (jobjectArray) splitPublicSourceDirsField->get(obj); // jstringArray
        if (splitSourceDirs) {
            for (int i = 0; i < g_env->GetArrayLength(splitSourceDirs); i++) {
                auto splitSourceDir = (jstring) g_env->GetObjectArrayElement(splitSourceDirs, i);
                LOGI("-------- Split source dir[%d]: %s", i, g_env->GetStringUTFChars(splitSourceDir, 0));
                g_env->SetObjectArrayElement(splitSourceDirs, i, g_apkPath);
            }
            splitSourceDirsField->set(obj, splitSourceDirs);
        }
        if (splitSourceDirs) {
            for (int i = 0; i < g_env->GetArrayLength(splitPublicSourceDirs); i++) {
                auto splitPublicSourceDir = (jstring) g_env->GetObjectArrayElement(splitPublicSourceDirs, i);
                LOGI("-------- Split public source dir[%d]: %s", i, g_env->GetStringUTFChars(splitPublicSourceDir, 0));
                g_env->SetObjectArrayElement(splitPublicSourceDirs, i, g_apkPath);
            }
            splitPublicSourceDirsField->set(obj, splitPublicSourceDirs);
        }
    }
}

void patch_LoadedApk(jobject obj) {
    if (!obj) return;
    LOGI("-------- Patching LoadedApk - %p", obj);
    auto loadedApkClass = Class::forName("android.app.LoadedApk");

    auto mApplicationInfoField = loadedApkClass->getDeclaredField("mApplicationInfo");
    mApplicationInfoField->setAccessible(true);
    patch_ApplicationInfo(mApplicationInfoField->get(obj));

    auto mAppDirField = loadedApkClass->getDeclaredField("mAppDir");
    mAppDirField->setAccessible(true);

    auto mResDirField = loadedApkClass->getDeclaredField("mResDir");
    mResDirField->setAccessible(true);

    mAppDirField->set(obj, g_apkPath);
    mResDirField->set(obj, g_apkPath);
}

void patch_AppBindData(jobject obj) {
    if (!obj) return;
    LOGI("-------- Patching AppBindData - %p", obj);
    auto appBindDataClass = Class::forName("android.app.ActivityThread$AppBindData");

    auto infoField = appBindDataClass->getDeclaredField("info");
    infoField->setAccessible(true);
    patch_LoadedApk(infoField->get(obj));

    auto appInfoField = appBindDataClass->getDeclaredField("appInfo");
    appInfoField->setAccessible(true);
    patch_ApplicationInfo(appInfoField->get(obj));
}

void patch_ContextImpl(jobject obj) {
    if (!obj) return;
    if (Class_isInstanceOf(obj, "android.app.ContextImpl")) {
        LOGI("-------- Patching ContextImpl - %p", obj);
        auto contextImplClass = Class::forName("android.app.ContextImpl");
        auto mPackageInfoField = contextImplClass->getDeclaredField("mPackageInfo");
        mPackageInfoField->setAccessible(true);
    }
}

void patch_Application(jobject obj) {
    if (!obj) return;
    if (Class_isInstanceOf(obj, "android.app.Application")) {
        LOGI("-------- Patching Application - %p", obj);
        auto applicationClass = Class::forName("android.app.Application");
        auto mLoadedApkField = applicationClass->getDeclaredField("mLoadedApk");
        mLoadedApkField->setAccessible(true);
        patch_LoadedApk(mLoadedApkField->get(obj));
    }

    patch_ContextImpl(getApplicationContext(obj)); // Don't use this if crashes
}

AAssetManager *g_assetManager;

void extractAsset(std::string assetName, std::string extractPath) {
    LOGI("-------- Extracting %s to %s", assetName.c_str(), extractPath.c_str());
    AAssetManager *assetManager = g_assetManager;
    AAsset *asset = AAssetManager_open(assetManager, assetName.c_str(), AASSET_MODE_UNKNOWN);
    if (!asset) {
        return;
    }

    int fd = open(extractPath.c_str(), O_CREAT | O_WRONLY, 0644);
    if (fd < 0) {
        AAsset_close(asset);
        return;
    }

    const int BUFFER_SIZE = 512;
    char buffer[BUFFER_SIZE];
    int bytesRead;
    while ((bytesRead = AAsset_read(asset, buffer, BUFFER_SIZE)) > 0) {
        int bytesWritten = write(fd, buffer, bytesRead);
        if (bytesWritten != bytesRead) {
            AAsset_close(asset);
            close(fd);
            return;
        }
    }

    AAsset_close(asset);
    close(fd);
}

void patch_PackageManager(jobject obj) {
    if (!obj) return;

    auto activityThreadClass = Class::forName("android.app.ActivityThread");
    auto sCurrentActivityThreadField = activityThreadClass->getDeclaredField("sCurrentActivityThread");
    sCurrentActivityThreadField->setAccessible(true);
    auto sCurrentActivityThread = sCurrentActivityThreadField->get(NULL);

    auto sPackageManagerField = activityThreadClass->getDeclaredField("sPackageManager");
    sPackageManagerField->setAccessible(true);
    g_packageManager = g_env->NewGlobalRef(sPackageManagerField->get(sCurrentActivityThread));

    auto iPackageManagerClass = Class::forName("android.content.pm.IPackageManager");

    auto classClass = g_env->FindClass("java/lang/Class");
    auto getClassLoaderMethod = g_env->GetMethodID(classClass, "getClassLoader", "()Ljava/lang/ClassLoader;");

    auto classLoader = g_env->CallObjectMethod(iPackageManagerClass->getClass(), getClassLoaderMethod);
    auto classArray = g_env->NewObjectArray(1, classClass, NULL);
    g_env->SetObjectArrayElement(classArray, 0, iPackageManagerClass->getClass());

    auto apkKillerClass = g_env->FindClass("com/kuro/APKKiller");
    auto myInvocationHandlerField = g_env->GetStaticFieldID(apkKillerClass, "myInvocationHandler", "Ljava/lang/reflect/InvocationHandler;");
    auto myInvocationHandler = g_env->GetStaticObjectField(apkKillerClass, myInvocationHandlerField);

    auto proxyClass = g_env->FindClass("java/lang/reflect/Proxy");
    auto newProxyInstanceMethod = g_env->GetStaticMethodID(proxyClass, "newProxyInstance", "(Ljava/lang/ClassLoader;[Ljava/lang/Class;Ljava/lang/reflect/InvocationHandler;)Ljava/lang/Object;");
    auto proxy = g_env->CallStaticObjectMethod(proxyClass, newProxyInstanceMethod, classLoader, classArray, myInvocationHandler);

    sPackageManagerField->set(sCurrentActivityThread, proxy);

    auto pm = getPackageManager(obj);
    auto mPMField = Class::forName("android.app.ApplicationPackageManager")->getDeclaredField("mPM");
    mPMField->setAccessible(true);
    mPMField->set(pm, proxy);
}

void bypassRestriction(JNIEnv *env) {
    ElfImg art("libart.so");
    auto setHiddenApiExemptionsMethod =
            (void (*)(JNIEnv*, jobject, jobjectArray))art.getSymbolAddress("_ZN3artL32VMRuntime_setHiddenApiExemptionsEP7_JNIEnvP7_jclassP13_jobjectArray");

    auto objectClass = env->FindClass("java/lang/Object");
    auto objectArray = env->NewObjectArray(1, objectClass, NULL);
    env->SetObjectArrayElement(objectArray, 0, env->NewStringUTF("L"));

    auto VMRuntimeClass = env->FindClass("dalvik/system/VMRuntime");
    setHiddenApiExemptionsMethod(env, VMRuntimeClass, objectArray);
}

void APKKill(JNIEnv *env, jclass clazz, jobject context) {
    LOGI("-------- Killing APK");

    APKKiller::g_env = env;
    g_assetManager = AAssetManager_fromJava(env, env->CallObjectMethod(context, env->GetMethodID(env->FindClass("android/content/Context"), "getAssets", "()Landroid/content/res/AssetManager;")));

    std::string apkPkg = getPackageName(context);
    APKKiller::g_apkPkg = apkPkg;

    LOGI("-------- Killing %s", apkPkg.c_str());

    char apkDir[512];
    sprintf(apkDir, "/data/data/%s/cache", apkPkg.c_str());
    mkdir(apkDir, 0777);

    std::string apkPath = "/data/data/";
    apkPath += apkPkg;
    apkPath += "/cache/";
    apkPath += apk_fake_name;

    if (access(apkPath.c_str(), F_OK) == -1) {
        extractAsset(apk_asset_path, apkPath);
    }

    APKKiller::g_apkPath = (jstring) env->NewGlobalRef(g_env->NewStringUTF(apkPath.c_str()));

    bypassRestriction(env);

    auto activityThreadClass = Class::forName("android.app.ActivityThread");
    auto sCurrentActivityThreadField = activityThreadClass->getDeclaredField("sCurrentActivityThread");
    sCurrentActivityThreadField->setAccessible(true);
    auto sCurrentActivityThread = sCurrentActivityThreadField->get(NULL);

    auto mBoundApplicationField = activityThreadClass->getDeclaredField("mBoundApplication");
    mBoundApplicationField->setAccessible(true);
    patch_AppBindData(mBoundApplicationField->get(sCurrentActivityThread));

    auto mInitialApplicationField = activityThreadClass->getDeclaredField("mInitialApplication");
    mInitialApplicationField->setAccessible(true);
    patch_Application(mInitialApplicationField->get(sCurrentActivityThread));

    auto mAllApplicationsField = activityThreadClass->getDeclaredField("mAllApplications");
    mAllApplicationsField->setAccessible(true);
    auto mAllApplications = mAllApplicationsField->get(sCurrentActivityThread);
    ArrayList *list = new ArrayList(mAllApplications);
    for (int i = 0; i < list->size(); i++) {
        auto application = list->get(i);
        patch_Application(application);
        list->set(i, application);
    }
    mAllApplicationsField->set(sCurrentActivityThread, list->getObj());

    auto mPackagesField = activityThreadClass->getDeclaredField("mPackages");
    mPackagesField->setAccessible(true);
    auto mPackages = mPackagesField->get(sCurrentActivityThread);
    ArrayMap *map = new ArrayMap(mPackages);
    for (int i = 0; i < map->size(); i++) {
        auto loadedApk = new WeakReference(map->valueAt(i));
        patch_LoadedApk(loadedApk->getObj());
        map->setValueAt(i, WeakReference::Create(loadedApk->getObj()));
    }
    mPackagesField->set(sCurrentActivityThread, map->getObj());

    auto mResourcePackagesField = activityThreadClass->getDeclaredField("mResourcePackages");
    mResourcePackagesField->setAccessible(true);
    auto mResourcePackages = mResourcePackagesField->get(sCurrentActivityThread);
    map = new ArrayMap(mResourcePackages);
    for (int i = 0; i < map->size(); i++) {
        auto loadedApk = new WeakReference(map->valueAt(i));
        patch_LoadedApk(loadedApk->getObj());
        map->setValueAt(i, WeakReference::Create(loadedApk->getObj()));
    }
    mResourcePackagesField->set(sCurrentActivityThread, map->getObj());

    patch_ContextImpl(context); // Don't use this if crashes
    patch_PackageManager(context);
}

jobject nativeInvoke(JNIEnv *env, jclass clazz, jobject proxy, jobject method, jobjectArray args) {
    g_env = env;

    auto Method_getName = [env](jobject method) {
        return env->CallObjectMethod(method, env->GetMethodID(env->FindClass("java/lang/reflect/Method"), "getName", "()Ljava/lang/String;"));
    };

    auto Method_invoke = [env](jobject method, jobject obj, jobjectArray args) {
        return env->CallObjectMethod(method, env->GetMethodID(env->FindClass("java/lang/reflect/Method"), "invoke", "(Ljava/lang/Object;[Ljava/lang/Object;)Ljava/lang/Object;"), obj, args);
    };

    auto Integer_intValue = [env](jobject integer) {
        return env->CallIntMethod(integer, env->GetMethodID(env->FindClass("java/lang/Integer"), "intValue", "()I"));
    };

    const char *Name = env->GetStringUTFChars((jstring) Method_getName(method), NULL);
    if (!strcmp(Name, "getPackageInfo")) {
        const char *packageName = env->GetStringUTFChars((jstring) env->GetObjectArrayElement(args, 0), NULL);
        int flags = Integer_intValue(env->GetObjectArrayElement(args, 1));
        if (!strcmp(packageName, g_apkPkg.c_str())) {
            if ((flags & 0x40) != 0) {
                auto packageInfo = Method_invoke(method, g_packageManager, args);
                if (packageInfo) {
                    auto packageInfoClass = Class::forName("android.content.pm.PackageInfo");
                    auto applicationInfoField = packageInfoClass->getDeclaredField("applicationInfo");
                    applicationInfoField->setAccessible(true);
                    auto applicationInfo = applicationInfoField->get(packageInfo);
                    if (applicationInfo) {
                        patch_ApplicationInfo(applicationInfo);
                    }
                    applicationInfoField->set(packageInfo, applicationInfo);
                    auto signaturesField = packageInfoClass->getDeclaredField("signatures");
                    signaturesField->setAccessible(true);

                    auto signatureClass = env->FindClass("android/content/pm/Signature");
                    auto signatureConstructor = env->GetMethodID(signatureClass, "<init>", "([B)V");
                    auto signatureArray = env->NewObjectArray(apk_signatures.size(), signatureClass, NULL);
                    for (int i = 0; i < apk_signatures.size(); i++) {
                        auto signature = env->NewByteArray(apk_signatures[i].size());
                        env->SetByteArrayRegion(signature, 0, apk_signatures[i].size(), (jbyte *) apk_signatures[i].data());
                        env->SetObjectArrayElement(signatureArray, i, env->NewObject(signatureClass, signatureConstructor, signature));
                    }
                    signaturesField->set(packageInfo, signatureArray);
                }
                return packageInfo;
            } else if ((flags & 0x8000000) != 0) {
                auto packageInfo = Method_invoke(method, g_packageManager, args);
                if (packageInfo) {
                    auto packageInfoClass = Class::forName("android.content.pm.PackageInfo");
                    auto applicationInfoField = packageInfoClass->getDeclaredField("applicationInfo");
                    applicationInfoField->setAccessible(true);
                    auto applicationInfo = applicationInfoField->get(packageInfo);
                    if (applicationInfo) {
                        patch_ApplicationInfo(applicationInfo);
                    }
                    applicationInfoField->set(packageInfo, applicationInfo);
                    auto signingInfoField = packageInfoClass->getDeclaredField("signingInfo");
                    signingInfoField->setAccessible(true);
                    auto signingInfo = signingInfoField->get(packageInfo);

                    auto signingInfoClass = Class::forName("android.content.pm.SigningInfo");
                    auto mSigningDetailsField = signingInfoClass->getDeclaredField("mSigningDetails");
                    mSigningDetailsField->setAccessible(true);
                    auto mSigningDetails = mSigningDetailsField->get(signingInfo);

                    auto signingDetailsClass = Class::forName("android.content.pm.PackageParser$SigningDetails");
                    auto signaturesField = signingDetailsClass->getDeclaredField("signatures");
                    signaturesField->setAccessible(true);
                    auto pastSigningCertificatesField = signingDetailsClass->getDeclaredField("pastSigningCertificates");
                    pastSigningCertificatesField->setAccessible(true);

                    auto signatureClass = env->FindClass("android/content/pm/Signature");
                    auto signatureConstructor = env->GetMethodID(signatureClass, "<init>", "([B)V");
                    auto signatureArray = env->NewObjectArray(apk_signatures.size(), signatureClass, NULL);
                    for (int i = 0; i < apk_signatures.size(); i++) {
                        auto signature = env->NewByteArray(apk_signatures[i].size());
                        env->SetByteArrayRegion(signature, 0, apk_signatures[i].size(), (jbyte *) apk_signatures[i].data());
                        env->SetObjectArrayElement(signatureArray, i, env->NewObject(signatureClass, signatureConstructor, signature));
                    }

                    signaturesField->set(mSigningDetails, signatureArray);
                    pastSigningCertificatesField->set(mSigningDetails, signatureArray);
                }
                return packageInfo;
            } else {
                auto packageInfo = Method_invoke(method, g_packageManager, args);
                if (packageInfo) {
                    auto packageInfoClass = Class::forName("android.content.pm.PackageInfo");
                    auto applicationInfoField = packageInfoClass->getDeclaredField("applicationInfo");
                    applicationInfoField->setAccessible(true);
                    auto applicationInfo = applicationInfoField->get(packageInfo);
                    if (applicationInfo) {
                        patch_ApplicationInfo(applicationInfo);
                    }
                }
                return packageInfo;
            }
        }
    } else if (!strcmp(Name, "getApplicationInfo")) {
        const char *packageName = env->GetStringUTFChars((jstring) env->GetObjectArrayElement(args, 0), NULL);
        if (!strcmp(packageName, g_apkPkg.c_str())) {
            auto applicationInfo = Method_invoke(method, g_packageManager, args);
            if (applicationInfo) {
                patch_ApplicationInfo(applicationInfo);
            }
            return applicationInfo;
        }
    }
    return Method_invoke(method, g_packageManager, args);
}
