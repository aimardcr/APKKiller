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
std::vector<std::vector<uint8_t>> apk_signatures;

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

        jstring getName() {
            return (jstring) g_env->CallObjectMethod(method, g_env->GetMethodID(g_env->FindClass("java/lang/reflect/Method"), "getName", "()Ljava/lang/String;"));
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

        auto mPackageManagerField = contextImplClass->getDeclaredField("mPackageManager");
        mPackageManagerField->setAccessible(true);
        mPackageManagerField->set(obj, g_proxy);
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
    g_proxy = g_env->NewGlobalRef(g_env->CallStaticObjectMethod(proxyClass, newProxyInstanceMethod, classLoader, classArray, myInvocationHandler));

    sPackageManagerField->set(sCurrentActivityThread, g_proxy);

    auto pm = getPackageManager(obj);
    auto mPMField = Class::forName("android.app.ApplicationPackageManager")->getDeclaredField("mPM");
    mPMField->setAccessible(true);
    mPMField->set(pm, g_proxy);
}

bool (*orig_IsInstanceOf)(JNIEnv *env, jobject obj, jclass clazz);
bool IsInstanceOf(JNIEnv *env, jobject obj, jclass clazz) {
    jclass proxyClass = env->FindClass("java/lang/reflect/Proxy");
    if (clazz == proxyClass) {
        return false;
    }
    return orig_IsInstanceOf(env, obj, clazz);
}

void doBypass(JNIEnv *env) {
    ElfImg art("libart.so");
    auto setHiddenApiExemptionsMethod = (void (*)(JNIEnv*, jobject, jobjectArray)) art.getSymbolAddress("_ZN3artL32VMRuntime_setHiddenApiExemptionsEP7_JNIEnvP7_jclassP13_jobjectArray");

    auto objectClass = env->FindClass("java/lang/Object");
    auto objectArray = env->NewObjectArray(1, objectClass, NULL);
    env->SetObjectArrayElement(objectArray, 0, env->NewStringUTF("L"));

    auto VMRuntimeClass = env->FindClass("dalvik/system/VMRuntime");
    setHiddenApiExemptionsMethod(env, VMRuntimeClass, objectArray);

    auto IsInstanceOfMethod = (void *) art.getSymbolAddress("_ZN3art3JNI12IsInstanceOfEP7_JNIEnvP8_jobjectP7_jclass");
    WInlineHookFunction(IsInstanceOfMethod, (void *) IsInstanceOf, (void **) &orig_IsInstanceOf);
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
    auto m_APKSignField = g_env->GetStaticFieldID(apkKillerClass, "m_APKSign", "Ljava/lang/String;");
    auto m_APKSign = g_env->GetStringUTFChars((jstring) g_env->GetStaticObjectField(apkKillerClass, m_APKSignField), NULL);
    {
        auto decodedSignData = base64_decode(m_APKSign);
        BinaryReader reader(decodedSignData.data(), decodedSignData.size());
        apk_signatures.resize(reader.readInt());
        for (int i = 0; i < apk_signatures.size(); i++) {
            apk_signatures[i].resize(reader.readInt());
            auto sign = reader.readBytes(apk_signatures[i].size());
            memcpy(apk_signatures[i].data(), sign.data(), sign.size());
        }
    }

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

    auto Integer_intValue = [env](jobject integer) {
        return env->CallIntMethod(integer, env->GetMethodID(env->FindClass("java/lang/Integer"), "intValue", "()I"));
    };

    Method *mMethod = new Method(method);
    auto mName = mMethod->getName();

    const char *Name = env->GetStringUTFChars(mName, NULL);
    env->DeleteLocalRef(mName);
    if (!strcmp(Name, "getPackageInfo")) {
        const char *packageName = env->GetStringUTFChars((jstring) env->GetObjectArrayElement(args, 0), NULL);
        int flags = Integer_intValue(env->GetObjectArrayElement(args, 1));
        if (!strcmp(packageName, g_apkPkg.c_str())) {
            if ((flags & 0x40) != 0) {
                auto packageInfo = mMethod->invoke(g_packageManager, args);
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
                auto packageInfo = mMethod->invoke(g_packageManager, args);
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
                auto packageInfo = mMethod->invoke(g_packageManager, args);
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
            auto applicationInfo = mMethod->invoke(g_packageManager, args);
            if (applicationInfo) {
                patch_ApplicationInfo(applicationInfo);
            }
            return applicationInfo;
        }
    }
    return mMethod->invoke(g_packageManager, args);
}
