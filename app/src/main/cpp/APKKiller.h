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

#include <whale.h>
#include "ElfImg.h"

#include "Utils.h"
#include "BinaryReader.h"

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, "APKKiller", __VA_ARGS__)

#define apk_asset_path "original.apk" // assets/original.apk
#define apk_fake_name "original.apk" // /data/data/<package_name/cache/original.apk
std::vector<std::vector<uint8_t>> apk_signatures {{0x30,0x82,0x05,0x89,0x30,0x82,0x03,0x71,0xA0,0x03,0x02,0x01,0x02,0x02,0x15,0x00,0xBF,0x53,0xFA,0xBB,0x0B,0x4E,0x90,0x7F,0x8F,0x2A,0x22,0xA1,0xB0,0xE0,0xD7,0x7F,0x6D,0xEF,0x10,0x60,0x30,0x0D,0x06,0x09,0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x01,0x0B,0x05,0x00,0x30,0x74,0x31,0x0B,0x30,0x09,0x06,0x03,0x55,0x04,0x06,0x13,0x02,0x55,0x53,0x31,0x13,0x30,0x11,0x06,0x03,0x55,0x04,0x08,0x13,0x0A,0x43,0x61,0x6C,0x69,0x66,0x6F,0x72,0x6E,0x69,0x61,0x31,0x16,0x30,0x14,0x06,0x03,0x55,0x04,0x07,0x13,0x0D,0x4D,0x6F,0x75,0x6E,0x74,0x61,0x69,0x6E,0x20,0x56,0x69,0x65,0x77,0x31,0x14,0x30,0x12,0x06,0x03,0x55,0x04,0x0A,0x13,0x0B,0x47,0x6F,0x6F,0x67,0x6C,0x65,0x20,0x49,0x6E,0x63,0x2E,0x31,0x10,0x30,0x0E,0x06,0x03,0x55,0x04,0x0B,0x13,0x07,0x41,0x6E,0x64,0x72,0x6F,0x69,0x64,0x31,0x10,0x30,0x0E,0x06,0x03,0x55,0x04,0x03,0x13,0x07,0x41,0x6E,0x64,0x72,0x6F,0x69,0x64,0x30,0x20,0x17,0x0D,0x32,0x30,0x30,0x35,0x31,0x39,0x31,0x30,0x33,0x35,0x32,0x31,0x5A,0x18,0x0F,0x32,0x30,0x35,0x30,0x30,0x35,0x31,0x39,0x31,0x30,0x33,0x35,0x32,0x31,0x5A,0x30,0x74,0x31,0x0B,0x30,0x09,0x06,0x03,0x55,0x04,0x06,0x13,0x02,0x55,0x53,0x31,0x13,0x30,0x11,0x06,0x03,0x55,0x04,0x08,0x13,0x0A,0x43,0x61,0x6C,0x69,0x66,0x6F,0x72,0x6E,0x69,0x61,0x31,0x16,0x30,0x14,0x06,0x03,0x55,0x04,0x07,0x13,0x0D,0x4D,0x6F,0x75,0x6E,0x74,0x61,0x69,0x6E,0x20,0x56,0x69,0x65,0x77,0x31,0x14,0x30,0x12,0x06,0x03,0x55,0x04,0x0A,0x13,0x0B,0x47,0x6F,0x6F,0x67,0x6C,0x65,0x20,0x49,0x6E,0x63,0x2E,0x31,0x10,0x30,0x0E,0x06,0x03,0x55,0x04,0x0B,0x13,0x07,0x41,0x6E,0x64,0x72,0x6F,0x69,0x64,0x31,0x10,0x30,0x0E,0x06,0x03,0x55,0x04,0x03,0x13,0x07,0x41,0x6E,0x64,0x72,0x6F,0x69,0x64,0x30,0x82,0x02,0x22,0x30,0x0D,0x06,0x09,0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x01,0x01,0x05,0x00,0x03,0x82,0x02,0x0F,0x00,0x30,0x82,0x02,0x0A,0x02,0x82,0x02,0x01,0x00,0xD8,0x67,0x46,0x0C,0x79,0xD0,0x43,0xBA,0x8B,0x0C,0xB0,0xB0,0x56,0x15,0x9B,0x95,0x3E,0x9E,0xFE,0x59,0x81,0xEA,0x62,0x93,0xFE,0x99,0x61,0xA3,0x35,0xBD,0x5F,0xCB,0xB8,0x3C,0x60,0x82,0x4D,0xED,0xE5,0x9F,0xF3,0x7C,0x8D,0x4E,0x4E,0xC4,0xBC,0xC5,0x53,0x97,0xEA,0x86,0xFF,0xF9,0x0D,0x58,0x9A,0x18,0xD9,0x90,0xAE,0x9F,0x7F,0x24,0x3D,0xAA,0xA7,0x4A,0xBC,0x23,0x03,0x34,0x07,0xA5,0x19,0x43,0xBB,0x87,0x33,0x21,0x16,0xBF,0xD0,0x58,0x36,0x6B,0xAE,0x07,0x80,0x0E,0x40,0x62,0xCE,0x20,0xCE,0x44,0x56,0x00,0x7A,0x1A,0xAA,0x56,0x80,0xB2,0x39,0x5C,0xDA,0x7A,0xED,0x6E,0x3F,0xD3,0x02,0xEF,0x02,0xD6,0xB7,0x45,0xAE,0xDF,0x3B,0xA8,0x6F,0xFF,0x50,0x77,0x7F,0x14,0x3A,0x69,0x29,0xC9,0x8C,0x53,0x34,0xFF,0x34,0x5F,0x2A,0x9A,0x34,0x49,0xB1,0xD7,0x9E,0x59,0xBD,0x69,0xCD,0x3D,0x52,0xD0,0x14,0x59,0x0E,0x2F,0xD9,0x73,0xFA,0x12,0x90,0xDA,0x60,0x67,0x27,0x0C,0xD8,0x5F,0xD2,0x12,0x40,0x2C,0x67,0xAC,0x7F,0xC8,0x32,0xCD,0xEA,0xD9,0x76,0xCC,0x23,0xBE,0x70,0xA0,0xBA,0x9D,0x51,0xDD,0xC2,0xA8,0x84,0x45,0x10,0xA2,0x52,0x8D,0xB9,0xAC,0xA8,0x4B,0x21,0xF3,0x7D,0xE6,0xCC,0xDF,0x51,0xD2,0x69,0x94,0x90,0x67,0xC7,0x53,0x27,0x09,0xDF,0xD5,0x79,0x8C,0xB4,0x30,0x01,0x83,0x80,0x56,0xCB,0xD7,0xE5,0xC7,0xA6,0xC1,0x98,0xBE,0x49,0x13,0x39,0xFC,0x0C,0x33,0xEE,0x1A,0xE4,0xB1,0xA3,0xFE,0x7F,0xF1,0x6F,0xCD,0xCC,0x44,0xC5,0x0D,0x9A,0x90,0xE7,0xCA,0xB6,0x8C,0x61,0xEF,0xAB,0x92,0x2B,0x51,0xE7,0xE3,0x52,0x14,0x32,0xF3,0xC5,0xE8,0x5F,0xA1,0x72,0x04,0x54,0xBE,0x4C,0x29,0xC4,0x67,0x4A,0x06,0x0F,0x55,0xF7,0x5D,0x82,0xE4,0x04,0x10,0xA6,0x31,0x11,0x84,0x62,0xA6,0x99,0x78,0x8C,0x1D,0xB3,0x2E,0x2A,0x80,0x1C,0xC2,0x14,0x67,0x17,0xCD,0xE2,0xE0,0xF5,0x2C,0xB7,0x40,0xEB,0x97,0x9A,0xC2,0x17,0x6C,0x85,0xD0,0x60,0x0A,0x66,0x3A,0xE7,0x5E,0xF2,0x86,0x88,0x7B,0x7C,0x13,0x8C,0x1F,0xAE,0xDC,0xA8,0xF8,0x15,0xCB,0x94,0x96,0xC1,0x85,0xBB,0x43,0xFB,0xC0,0xF2,0x76,0x09,0x7C,0xA7,0x1C,0x13,0x34,0x2D,0xE5,0x97,0xCB,0x69,0x91,0x92,0x80,0x91,0x51,0x5D,0xC8,0x57,0xC3,0x1F,0x14,0xC1,0x1D,0xFA,0x41,0xD3,0x98,0xC0,0x24,0x16,0x70,0x5B,0x08,0xD0,0xFD,0x7F,0x98,0x6A,0x9D,0x87,0xC6,0x1B,0x61,0xFB,0x32,0x36,0x5B,0x09,0x29,0x8F,0xFE,0x20,0x6F,0xA6,0x49,0xC6,0xDE,0xC4,0x7E,0x3D,0xA5,0x8E,0x80,0x0C,0xB2,0xCF,0xB1,0x40,0x3E,0xBB,0xA8,0x01,0x08,0xDD,0x71,0x60,0x7F,0x68,0xBC,0x57,0x6C,0xD2,0x16,0x38,0x92,0x57,0x0B,0x64,0xEA,0x5F,0x5C,0xA6,0xBA,0xA9,0x3F,0xF3,0x0C,0x66,0x7F,0x27,0x85,0xD6,0xEC,0x4B,0x88,0x75,0xF4,0x21,0x7A,0x0C,0x6C,0xEF,0x4E,0xFA,0x8A,0xA5,0x44,0xE9,0x0B,0xAC,0x99,0x94,0x0A,0xC1,0xFF,0x51,0x5D,0x36,0x6D,0xC1,0xD4,0x4C,0x0A,0xD7,0xB2,0xB2,0x80,0xB4,0x25,0xDF,0xBF,0xE9,0x7C,0x9C,0x8A,0xF7,0x19,0x34,0x50,0xBF,0xCB,0x02,0x03,0x01,0x00,0x01,0xA3,0x10,0x30,0x0E,0x30,0x0C,0x06,0x03,0x55,0x1D,0x13,0x04,0x05,0x30,0x03,0x01,0x01,0xFF,0x30,0x0D,0x06,0x09,0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x01,0x0B,0x05,0x00,0x03,0x82,0x02,0x01,0x00,0x67,0xA6,0xBC,0x59,0x21,0x7C,0xA6,0xD7,0xC3,0xC3,0x5B,0xA5,0xB1,0xF0,0xD7,0xF5,0xB4,0xA3,0xB7,0x23,0x0F,0xD9,0x47,0xEF,0xB6,0x49,0x3A,0xCB,0x6D,0x4D,0x56,0x3C,0xAE,0x33,0x84,0x8D,0x15,0x60,0xFF,0x53,0x6D,0x92,0xE0,0x48,0xAE,0x43,0xE8,0x6B,0xE6,0x92,0xBD,0x34,0x85,0xA0,0x9F,0x9C,0xC1,0x59,0x04,0x20,0xBB,0xC6,0x1D,0x0F,0x16,0x3F,0x3E,0x8E,0x3E,0xE1,0x9E,0x1D,0x0E,0x32,0xFF,0x1A,0x31,0x58,0x84,0xB7,0xD2,0xAA,0x01,0x24,0x15,0x0D,0x75,0xC6,0xEC,0x91,0xAE,0xE8,0x55,0x04,0x69,0x75,0x35,0xFA,0xC3,0x70,0x68,0x5E,0xC7,0x01,0xC2,0x4B,0xA4,0x58,0x81,0xB0,0x63,0x8F,0x19,0xB1,0x59,0xC6,0xEC,0x0A,0x40,0xFB,0x53,0x92,0x9B,0xB1,0xD1,0x70,0xDA,0x06,0x0C,0x99,0x0E,0x2C,0x26,0x14,0x37,0x3F,0x38,0xF5,0x1A,0x71,0xCA,0xF4,0x3B,0x49,0x26,0x39,0x71,0xC6,0x7D,0xCE,0x72,0x2A,0x9A,0xD9,0x3F,0xD1,0xD2,0xC0,0x84,0xE8,0x5C,0xF7,0x5B,0xD4,0xA3,0xAC,0xE4,0xD9,0x18,0x91,0x5B,0x39,0x36,0x6D,0x35,0xB9,0x0E,0x73,0xD0,0x21,0xF4,0x9C,0x81,0x0E,0xE8,0xF3,0xA7,0x1E,0x17,0x25,0x1A,0xE2,0xC9,0xE0,0xCF,0xA8,0x1E,0x18,0x09,0xF7,0x3E,0xA5,0x12,0x72,0x9C,0xE5,0xDE,0x13,0xB8,0x40,0x37,0x64,0xDE,0x34,0x4D,0x88,0x56,0xF5,0x87,0x56,0xCF,0x8E,0x29,0xE4,0x2B,0xC0,0x36,0xDD,0xC0,0x03,0x8A,0x7C,0xF4,0xA4,0x55,0xCE,0xBF,0xE6,0x51,0xC7,0xC2,0x53,0xAB,0x40,0x75,0x27,0xD8,0x0F,0xAE,0xEA,0x2A,0xA2,0xA6,0x8A,0xCA,0xD9,0xA3,0x58,0x9A,0xF2,0xD4,0x29,0x47,0xEB,0xB7,0xD3,0x8B,0x84,0xE5,0x62,0xEE,0x41,0xFF,0x67,0x5D,0xDA,0x6C,0xF7,0x28,0x1B,0x90,0x57,0x9C,0x4B,0xC5,0x78,0x23,0x08,0x5E,0x7A,0x9B,0xEE,0x18,0x75,0xC7,0x6B,0x04,0x1C,0x16,0x72,0x67,0xB6,0xED,0x51,0x4D,0xD8,0x2F,0x69,0x0E,0xF9,0x77,0x59,0x31,0x33,0xB3,0x6B,0x4F,0x03,0xDF,0xAB,0xAF,0x69,0x98,0x1C,0x91,0x52,0x92,0xF1,0xE3,0x1D,0xED,0x03,0x88,0x79,0x0E,0x62,0x8F,0x5E,0xF5,0x9C,0xBC,0x1F,0x51,0x9C,0x78,0xEE,0x2F,0x82,0xD9,0xDA,0xB7,0xBE,0xF2,0xCA,0x0F,0x3B,0xAF,0xC7,0xBC,0x01,0xC9,0x6B,0x33,0xC4,0x27,0x46,0xD7,0x5F,0x48,0x07,0x39,0xF4,0xF2,0xC5,0xA1,0xAD,0xE4,0xE4,0x9D,0x15,0x49,0xF9,0x8C,0x48,0x5A,0x53,0x5F,0x4A,0xBF,0x18,0xBE,0xA0,0xFE,0xBC,0x63,0x06,0x97,0xA4,0xAF,0xF1,0x70,0xAA,0xB6,0x18,0x84,0x55,0xAE,0x4B,0x7C,0x22,0x63,0x5A,0x04,0xCB,0x0C,0x14,0x68,0x7E,0x8E,0x41,0x89,0xE6,0x6F,0x04,0x56,0x16,0x3E,0xCC,0xE2,0xCB,0x7A,0x73,0x04,0xC0,0x5B,0xD8,0x0C,0x46,0x0E,0x15,0x03,0x80,0xFB,0xBC,0x93,0x98,0x56,0x3F,0xDB,0x4F,0x9F,0x79,0x3F,0xF2,0x5D,0x13,0xAA,0xFD,0x77,0x62,0x62,0xBE,0x8A,0xC1,0x07,0x36,0x70,0x27,0xAF,0xC9,0xB7,0x67,0x00,0x54,0x07,0xEA,0x80,0x7C,0x90,0x93,0xF6,0xCC,0xD5,0x3B,0x15,0x51,0x67,0x66,0xC1,0x74,0x70,0x38,0xA0,0x3C,0x57,0xA4,0xA2,0xA3,0x75,0x29,0xEF,0x0C,0xD1,0x42,0xD2,0x40,0x26,0x47,0x6A,0xD7,0xB1,0xD8}}; // if you fill this, it will ignore the m_APKSign from APKKiller.java

namespace APKKiller {
    JNIEnv *g_env;
    jstring g_apkPath;
    jobject g_packageManager;
    jobject g_proxy;
    std::string g_apkPkg;

    class Reference {
    public:
        jobject reference;
    public:
        Reference(jobject reference) {
            this->reference = g_env->NewGlobalRef(reference);
        }

        jobject get() {
            auto referenceClass = g_env->FindClass("java/lang/ref/Reference");
            auto get = g_env->GetMethodID(referenceClass, "get", "()Ljava/lang/Object;");
            auto result = g_env->CallObjectMethod(reference, get);
            g_env->DeleteLocalRef(referenceClass);
            return result;
        }
    };

    class WeakReference : public Reference {
    public:
        WeakReference(jobject weakReference) : Reference(weakReference) {
        }

        static jobject Create(jobject obj) {
            auto weakReferenceClass = g_env->FindClass("java/lang/ref/WeakReference");
            auto weakReferenceClassConstructor = g_env->GetMethodID(weakReferenceClass, "<init>", "(Ljava/lang/Object;)V");
            auto result = g_env->NewObject(weakReferenceClass, weakReferenceClassConstructor, obj);
            g_env->DeleteLocalRef(weakReferenceClass);
            return result;
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
            auto arrayListClass = g_env->FindClass("java/util/ArrayList");
            auto getMethod = g_env->GetMethodID(arrayListClass, "get", "(I)Ljava/lang/Object;");
            auto result = g_env->NewGlobalRef(g_env->CallObjectMethod(arrayList, getMethod, index));
            g_env->DeleteLocalRef(arrayListClass);
            return result;
        }

        void set(int index, jobject value) {
            auto arrayListClass = g_env->FindClass("java/util/ArrayList");
            auto setMethod = g_env->GetMethodID(arrayListClass, "set", "(ILjava/lang/Object;)Ljava/lang/Object;");
            g_env->CallObjectMethod(arrayList, setMethod, index, value);
            g_env->DeleteLocalRef(arrayListClass);
        }

        int size() {
            auto arrayListClass = g_env->FindClass("java/util/ArrayList");
            auto sizeMethod = g_env->GetMethodID(arrayListClass, "size", "()I");
            auto result = g_env->CallIntMethod(arrayList, sizeMethod);
            g_env->DeleteLocalRef(arrayListClass);
            return result;
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

        jobject valueAt(int index) {
            auto arrayMapClass = g_env->FindClass("android/util/ArrayMap");
            auto valueAtMethod = g_env->GetMethodID(arrayMapClass, "valueAt", "(I)Ljava/lang/Object;");
            auto result = g_env->CallObjectMethod(arrayMap, valueAtMethod, index);
            g_env->DeleteLocalRef(arrayMapClass);
            return result;
        }

        jobject setValueAt(int index, jobject value) {
            auto arrayMapClass = g_env->FindClass("android/util/ArrayMap");
            auto setValueAtMethod = g_env->GetMethodID(arrayMapClass, "setValueAt", "(ILjava/lang/Object;)Ljava/lang/Object;");
            auto result = g_env->CallObjectMethod(arrayMap, setValueAtMethod, index, value);
            g_env->DeleteLocalRef(arrayMapClass);
            return result;
        }

        int size() {
            auto arrayMapClass = g_env->FindClass("android/util/ArrayMap");
            auto sizeMethod = g_env->GetMethodID(arrayMapClass, "size", "()I");
            auto result = g_env->CallIntMethod(arrayMap, sizeMethod);
            return result;
        }
    };

    class Field {
    private:
        jclass clazz;
        jfieldID field;
        bool isStatic;
    public:
        Field(jclass clazz, jfieldID field, bool isStatic) {
            this->clazz = clazz;
            this->field = field;
            this->isStatic = isStatic;
        }
        ~Field() {
            g_env->DeleteLocalRef(clazz);
        }

        jobject get(jobject obj) {
            if (isStatic) {
                return g_env->GetStaticObjectField(clazz, field);
            } else {
                return g_env->GetObjectField(obj, field);
            }
        }

        void set(jobject obj, jobject value) {
            if (isStatic) {
                g_env->SetStaticObjectField(clazz, field, value);
            } else {
                g_env->SetObjectField(obj, field, value);
            }
        }
    };

    // This is for Java Reflection Method, so getMethod and getStaticMethod in Class class is not used here.
    class Method {
    private:
        jobject method;
    public:
        Method(jobject method) {
            this->method = g_env->NewGlobalRef(method);
        }
        ~Method() {
            g_env->DeleteLocalRef(method);
        }

        const char *getName() {
            auto methodClass = g_env->FindClass("java/lang/reflect/Method");
            auto getNameMethod = g_env->GetMethodID(methodClass, "getName", "()Ljava/lang/String;");
            auto methodName = g_env->CallObjectMethod(method, getNameMethod);
            auto result = g_env->GetStringUTFChars((jstring)methodName, 0);
            g_env->DeleteLocalRef(methodName);
            g_env->DeleteLocalRef(methodClass);
            return result;
        }

        jobject invoke(jobject object, jobjectArray args = 0) {
            auto methodClass = g_env->FindClass("java/lang/reflect/Method");
            auto invokeMethod = g_env->GetMethodID(methodClass, "invoke", "(Ljava/lang/Object;[Ljava/lang/Object;)Ljava/lang/Object;");
            auto result = g_env->CallObjectMethod(method, invokeMethod, object, args);
            g_env->DeleteLocalRef(methodClass);
            return result;
        }
    };

    class Class {
    private:
        jclass clazz;
    public:
        Class(const char *className) {
            clazz = g_env->FindClass(className);
        }
        ~Class() {
            g_env->DeleteLocalRef(clazz);
        }

        jclass getClass() {
            return clazz;
        }

        Field *getField(const char *fieldName, const char *fieldSig) {
            auto field = g_env->GetFieldID(clazz, fieldName, fieldSig);
            if (g_env->ExceptionCheck()) {
                g_env->ExceptionDescribe();
                g_env->ExceptionClear();
            }
            LOGI("Field %s(%s): %p", fieldName, fieldSig, field);
            return new Field(clazz, field, false);
        }

        Field *getStaticField(const char *fieldName, const char *fieldSig) {
            auto field = g_env->GetStaticFieldID(clazz, fieldName, fieldSig);
            if (g_env->ExceptionCheck()) {
                g_env->ExceptionDescribe();
                g_env->ExceptionClear();
            }
            LOGI("Field %s(%s): %p", fieldName, fieldSig, field);
            return new Field(clazz, field, true);
        }

        jmethodID getMethod(const char *methodName, const char *methodSig) {
            auto method = g_env->GetMethodID(clazz, methodName, methodSig);
            if (g_env->ExceptionCheck()) {
                g_env->ExceptionDescribe();
                g_env->ExceptionClear();
            }
            LOGI("Method %s(%s): %p", methodName, methodSig, method);
            return method;
        }

        jmethodID getStaticMethod(const char *methodName, const char *methodSig) {
            auto method = g_env->GetStaticMethodID(clazz, methodName, methodSig);
            if (g_env->ExceptionCheck()) {
                g_env->ExceptionDescribe();
                g_env->ExceptionClear();
            }
            LOGI("Method %s(%s): %p", methodName, methodSig, method);
            return method;
        }
    };
}

using namespace APKKiller;

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


void patch_ApplicationInfo(jobject obj) {
    if (!obj) return;
    LOGI("-------- Patching ApplicationInfo - %p", obj);
    auto applicationInfoClass = new Class("android/content/pm/ApplicationInfo");

    auto sourceDirField = applicationInfoClass->getField("sourceDir", "Ljava/lang/String;");
    auto publicSourceDirField = applicationInfoClass->getField("publicSourceDir", "Ljava/lang/String;");

    sourceDirField->set(obj, g_apkPath);
    publicSourceDirField->set(obj, g_apkPath);
}

void patch_LoadedApk(jobject obj) {
    if (!obj) return;
    LOGI("-------- Patching LoadedApk - %p", obj);
    auto loadedApkClass = new Class("android/app/LoadedApk");

    auto mApplicationInfoField = loadedApkClass->getField("mApplicationInfo", "Landroid/content/pm/ApplicationInfo;");
    patch_ApplicationInfo(mApplicationInfoField->get(obj));

    auto mAppDirField = loadedApkClass->getField("mAppDir", "Ljava/lang/String;");
    auto mResDirField = loadedApkClass->getField("mResDir", "Ljava/lang/String;");

    mAppDirField->set(obj, g_apkPath);
    mResDirField->set(obj, g_apkPath);
}

void patch_AppBindData(jobject obj) {
    if (!obj) return;
    LOGI("-------- Patching AppBindData - %p", obj);
    auto appBindDataClass = new Class("android/app/ActivityThread$AppBindData");

    auto infoField = appBindDataClass->getField("info", "Landroid/app/LoadedApk;");
    patch_LoadedApk(infoField->get(obj));

    auto appInfoField = appBindDataClass->getField("appInfo", "Landroid/content/pm/ApplicationInfo;");
    patch_ApplicationInfo(appInfoField->get(obj));
}

void patch_PackageInfo(jobject obj) {
    if (!obj) return;
    LOGI("-------- Patching PackageInfo - %p", obj);
    auto packageInfoClass = new Class("android/content/pm/PackageInfo");

    auto applicationInfoField = packageInfoClass->getField("applicationInfo", "Landroid/content/pm/ApplicationInfo;");
    patch_ApplicationInfo(applicationInfoField->get(obj));
}

void patch_ContextImpl(jobject obj) {
    if (!obj) return;
    auto contextImplClass = new Class("android/app/ContextImpl");
    if (g_env->IsInstanceOf(obj, contextImplClass->getClass())) {
        LOGI("-------- Patching ContextImpl - %p", obj);
        auto mPackageInfoField = contextImplClass->getField("mPackageInfo", "Landroid/content/pm/PackageInfo;");
        patch_PackageInfo(mPackageInfoField->get(obj));
        auto mPackageManagerField = contextImplClass->getField("mPackageManager", "Landroid/content/pm/PackageManager;");
        mPackageManagerField->set(obj, g_proxy);
    }
}

void patch_Application(jobject obj) {
    if (!obj) return;
    auto applicationClass = new Class("android/app/Application");
    if (g_env->IsInstanceOf(obj, applicationClass->getClass())) {
        LOGI("-------- Patching Application - %p", obj);
        auto mLoadedApkField = applicationClass->getField("mLoadedApk", "Landroid/app/LoadedApk;");
        patch_LoadedApk(mLoadedApkField->get(obj));
    }

    // patch_ContextImpl(getApplicationContext(obj));
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

    auto activityThreadClass = new Class("android/app/ActivityThread");
    auto sCurrentActivityThreadField = activityThreadClass->getStaticField("sCurrentActivityThread", "Landroid/app/ActivityThread;");
    auto sCurrentActivityThread = sCurrentActivityThreadField->get(NULL);

    auto sPackageManagerField = activityThreadClass->getStaticField("sPackageManager", "Landroid/content/pm/IPackageManager;");
    g_packageManager = g_env->NewGlobalRef(sPackageManagerField->get(NULL));

    auto iPackageManagerClass = new Class("android/content/pm/IPackageManager");

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
    g_proxy = g_env->NewGlobalRef(g_env->CallStaticObjectMethod(proxyClass, newProxyInstanceMethod, classLoader, classArray, myInvocationHandler));

    sPackageManagerField->set(sCurrentActivityThread, g_proxy);

    auto pm = getPackageManager(obj);
    auto mPMField = (new Class("android/app/ApplicationPackageManager"))->getField("mPM", "Landroid/content/pm/IPackageManager;");
    mPMField->set(pm, g_proxy);
}

void doBypass(JNIEnv *env) {
    ElfImg art("libart.so");
    auto setHiddenApiExemptionsMethod = (void (*)(JNIEnv*, jobject, jobjectArray)) art.getSymbolAddress("_ZN3artL32VMRuntime_setHiddenApiExemptionsEP7_JNIEnvP7_jclassP13_jobjectArray");
    if (setHiddenApiExemptionsMethod != nullptr) {
        auto objectClass = env->FindClass("java/lang/Object");
        auto objectArray = env->NewObjectArray(1, objectClass, NULL);
        env->SetObjectArrayElement(objectArray, 0, env->NewStringUTF("L"));

        auto VMRuntimeClass = env->FindClass("dalvik/system/VMRuntime");
        auto getRuntimeMethod = env->GetStaticMethodID(VMRuntimeClass, "getRuntime", "()Ldalvik/system/VMRuntime;");
        auto runtime = env->CallStaticObjectMethod(VMRuntimeClass, getRuntimeMethod);
        // setHiddenApiExemptionsMethod(env, runtime, objectArray);
    }
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

    doBypass(env);

    auto apkKillerClass = g_env->FindClass("com/kuro/APKKiller");
    auto javaKillerMethod = g_env->GetStaticMethodID(apkKillerClass, "Kill", "()V");
    g_env->CallStaticVoidMethod(apkKillerClass, javaKillerMethod);

    if (apk_signatures.empty()) {
        auto m_APKSignField = g_env->GetStaticFieldID(apkKillerClass, "m_APKSign", "Ljava/lang/String;");
        auto m_APKSign = g_env->GetStringUTFChars((jstring) g_env->GetStaticObjectField(apkKillerClass, m_APKSignField), NULL);
        {
            auto signs = base64_decode(m_APKSign);
            BinaryReader reader(signs.data(), signs.size());
            apk_signatures.resize((int) reader.readInt8());
            for (int i = 0; i < apk_signatures.size(); i++) {
                size_t size = reader.readInt32(), n;
                apk_signatures[i].resize(size);

                uint8_t *sign = new uint8_t[size];
                if ((n = reader.read(sign, size)) > 0) {
                    memcpy(apk_signatures[i].data(), sign, n);
                }
            }
        }
    }

    auto activityThreadClass = new Class("android/app/ActivityThread");
    auto sCurrentActivityThreadField = activityThreadClass->getStaticField("sCurrentActivityThread", "Landroid/app/ActivityThread;");
    auto sCurrentActivityThread = sCurrentActivityThreadField->get(NULL);

    auto mBoundApplicationField = activityThreadClass->getField("mBoundApplication", "Landroid/app/ActivityThread$AppBindData;");
    patch_AppBindData(mBoundApplicationField->get(sCurrentActivityThread));

    auto mInitialApplicationField = activityThreadClass->getField("mInitialApplication", "Landroid/app/Application;");
    patch_Application(mInitialApplicationField->get(sCurrentActivityThread));

    auto mAllApplicationsField = activityThreadClass->getField("mAllApplications", "Ljava/util/ArrayList;");
    auto mAllApplications = mAllApplicationsField->get(sCurrentActivityThread);
    ArrayList *list = new ArrayList(mAllApplications);
    for (int i = 0; i < list->size(); i++) {
        auto application = list->get(i);
        patch_Application(application);
        list->set(i, application);
    }
    mAllApplicationsField->set(sCurrentActivityThread, list->getObj());

    auto mPackagesField = activityThreadClass->getField("mPackages", "Landroid/util/ArrayMap;");
    auto mPackages = mPackagesField->get(sCurrentActivityThread);
    ArrayMap *map = new ArrayMap(mPackages);
    for (int i = 0; i < map->size(); i++) {
        auto loadedApk = new WeakReference(map->valueAt(i));
        patch_LoadedApk(loadedApk->get());
        map->setValueAt(i, WeakReference::Create(loadedApk->get()));
    }
    mPackagesField->set(sCurrentActivityThread, map->getObj());

    auto mResourcePackagesField = activityThreadClass->getField("mResourcePackages", "Landroid/util/ArrayMap;");
    auto mResourcePackages = mResourcePackagesField->get(sCurrentActivityThread);
    map = new ArrayMap(mResourcePackages);
    for (int i = 0; i < map->size(); i++) {
        auto loadedApk = new WeakReference(map->valueAt(i));
        patch_LoadedApk(loadedApk->get());
        map->setValueAt(i, WeakReference::Create(loadedApk->get()));
    }
    mResourcePackagesField->set(sCurrentActivityThread, map->getObj());

    // patch_ContextImpl(context); // This line crashes on some games.
    patch_PackageManager(context);
}

jobject processInvoke(JNIEnv *env, jobject clazz, jobject method, jobjectArray args) {
    g_env = env;

    auto Integer_intValue = [env](jobject integer) {
        return env->CallIntMethod(integer, env->GetMethodID(env->FindClass("java/lang/Integer"), "intValue", "()I"));
    };
    
    auto mMethod = new Method(method);
    const char *Name = mMethod->getName();
    if (!strcmp(Name, "getPackageInfo")) {
        const char *packageName = env->GetStringUTFChars((jstring) env->GetObjectArrayElement(args, 0), NULL);
        int flags = Integer_intValue(env->GetObjectArrayElement(args, 1));
        if (!strcmp(packageName, g_apkPkg.c_str())) {
            if ((flags & 0x40) != 0) {
                auto packageInfo = mMethod->invoke(g_packageManager, args);
                if (packageInfo) {
                    auto packageInfoClass = new Class("android/content/pm/PackageInfo");
                    auto applicationInfoField = packageInfoClass->getField("applicationInfo", "Landroid/content/pm/ApplicationInfo;");
                    auto applicationInfo = applicationInfoField->get(packageInfo);
                    if (applicationInfo) {
                        patch_ApplicationInfo(applicationInfo);
                    }
                    applicationInfoField->set(packageInfo, applicationInfo);
                    auto signaturesField = packageInfoClass->getField("signatures", "[Landroid/content/pm/Signature;");

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
                auto packageInfo = mMethod->invoke(g_packageManager, args);
                if (packageInfo) {
                    auto packageInfoClass = new Class("android/content/pm/PackageInfo");
                    auto applicationInfoField = packageInfoClass->getField("applicationInfo", "Landroid/content/pm/ApplicationInfo;");
                    auto applicationInfo = applicationInfoField->get(packageInfo);
                    if (applicationInfo) {
                        patch_ApplicationInfo(applicationInfo);
                    }
                    applicationInfoField->set(packageInfo, applicationInfo);
                    auto signingInfoField = packageInfoClass->getField("signingInfo", "Landroid/content/pm/SigningInfo;");
                    auto signingInfo = signingInfoField->get(packageInfo);

                    auto signingInfoClass = new Class("android/content/pm/SigningInfo");
                    auto mSigningDetailsField = signingInfoClass->getField("mSigningDetails", "Landroid/content/pm/PackageParser$SigningDetails;");
                    auto mSigningDetails = mSigningDetailsField->get(signingInfo);

                    auto signingDetailsClass = new Class("android/content/pm/PackageParser$SigningDetails");
                    auto signaturesField = signingDetailsClass->getField("signatures", "[Landroid/content/pm/Signature;");
                    auto pastSigningCertificatesField = signingDetailsClass->getField("pastSigningCertificates", "[Landroid/content/pm/Signature;");

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
                auto packageInfo = mMethod->invoke(g_packageManager, args);
                if (packageInfo) {
                    auto packageInfoClass = new Class("android/content/pm/PackageInfo");
                    auto applicationInfoField = packageInfoClass->getField("applicationInfo", "Landroid/content/pm/ApplicationInfo;");
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
            auto applicationInfo = mMethod->invoke(g_packageManager, args);
            if (applicationInfo) {
                patch_ApplicationInfo(applicationInfo);
            }
            return applicationInfo;
        }
    } else if (!strcmp(Name, "getInstallerPackageName")) {
        const char *packageName = env->GetStringUTFChars((jstring) env->GetObjectArrayElement(args, 0), NULL);
        if (!strcmp(packageName, g_apkPkg.c_str())) {
            return env->NewStringUTF("com.android.vending");
        }
    } else if (!strcmp(Name, "getInstallSourceInfo")) {
        const char *packageName = env->GetStringUTFChars((jstring) env->GetObjectArrayElement(args, 0), NULL);
        if (!strcmp(packageName, g_apkPkg.c_str())) {
            auto result =  mMethod->invoke(g_packageManager, args);
            if (result) {
                auto installSourceInfoClass = new Class("android/content/pm/InstallSourceInfo");
                auto mInitiatingPackageNameField = installSourceInfoClass->getField("mInitiatingPackageName", "Ljava/lang/String;");
                auto mInitiatingPackageSigningInfoField = installSourceInfoClass->getField("mInitiatingPackageSigningInfo", "Landroid/content/pm/SigningInfo;");
                auto mOriginatingPackageNameField = installSourceInfoClass->getField("mOriginatingPackageName", "Ljava/lang/String;");
                auto mInstallingPackageNameField = installSourceInfoClass->getField("mInstallingPackageName", "Ljava/lang/String;");

                auto signingInfoClass = new Class("android/content/pm/SigningInfo");

                auto mInitiatingPackageName = mInitiatingPackageNameField->get(result);
                auto mOriginatingPackageName = mOriginatingPackageNameField->get(result);
                auto mInstallingPackageName = mInstallingPackageNameField->get(result);

                const char *initiatingPackageName = env->GetStringUTFChars((jstring) mInitiatingPackageName, NULL);
                const char *originatingPackageName = env->GetStringUTFChars((jstring) mOriginatingPackageName, NULL);
                const char *installingPackageName = env->GetStringUTFChars((jstring) mInstallingPackageName, NULL);

                // TODO: Write new information then return it
            }
        }
    }
    return mMethod->invoke(g_packageManager, args);
}
