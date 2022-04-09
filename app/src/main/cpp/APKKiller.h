#include <stdio.h>
#include <iostream>
#include <jni.h>

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, "APKKiller", __VA_ARGS__)

#define apk_asset_path "original.apk" // assets/original.apk
#define apk_fake_name "original.apk" // /data/data/<package_name/cache/original.apk
std::vector<std::vector<uint8_t>> apk_signatures {{}}; // Use APKSignReader to replace this

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
            this->reference = reference;
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
            this->arrayList = arrayList;
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
            this->arrayMap = arrayMap;
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
            this->field = field;
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
            this->method = method;
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
            this->clazz = clazz;
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

            return field;
        }

        Method *getDeclaredMethod(const char *s, jobjectArray args = 0) {
            auto str = g_env->NewStringUTF(s);

            auto classClass = g_env->FindClass("java/lang/Class");
            auto getDeclaredMethodMethod = g_env->GetMethodID(classClass, "getDeclaredMethod", "(Ljava/lang/String;[Ljava/lang/Class;)Ljava/lang/reflect/Method;");

            auto method = new Method(g_env->CallObjectMethod(clazz, getDeclaredMethodMethod, str, args));

            return method;
        }
    };
}

using namespace APKKiller;

int getAPILevel() {
    static int api_level = -1;
    if (api_level == -1) {
        char prop_value[PROP_VALUE_MAX];
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
    for(int i = 0; i < list->size(); i++) {
        auto application = list->get(i);
        patch_Application(application);
        list->set(i, application);
    }
    mAllApplicationsField->set(sCurrentActivityThread, list->getObj());

    auto mPackagesField = activityThreadClass->getDeclaredField("mPackages");
    mPackagesField->setAccessible(true);
    auto mPackages = mPackagesField->get(sCurrentActivityThread);
    ArrayMap *map = new ArrayMap(mPackages);
    for(int i = 0; i < map->size(); i++) {
        auto loadedApk = new WeakReference(map->valueAt(i));
        patch_LoadedApk(loadedApk->getObj());
        map->setValueAt(i, WeakReference::Create(loadedApk->getObj()));
    }
    mPackagesField->set(sCurrentActivityThread, map->getObj());

    auto mResourcePackagesField = activityThreadClass->getDeclaredField("mResourcePackages");
    mResourcePackagesField->setAccessible(true);
    auto mResourcePackages = mResourcePackagesField->get(sCurrentActivityThread);
    map = new ArrayMap(mResourcePackages);
    for(int i = 0; i < map->size(); i++) {
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
    if(!strcmp(Name, "getPackageInfo")) {
        const char *packageName = env->GetStringUTFChars((jstring) env->GetObjectArrayElement(args, 0), NULL);
        int flags = Integer_intValue(env->GetObjectArrayElement(args, 1));
        if (!strcmp(packageName, g_apkPkg.c_str())) {
            if ((flags & 0x40) != 0) {
                auto packageInfo = Method_invoke(method, g_packageManager, args);
                if (packageInfo) {
                    auto packageInfoClass = Class::forName("android.content.pm.PackageInfo");
                    auto signaturesField = packageInfoClass->getDeclaredField("signatures");

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
            }
            if ((flags & 0x8000000) != 0) {
                auto packageInfo = Method_invoke(method, g_packageManager, args);
                if (packageInfo) {
                    auto packageInfoClass = Class::forName("android.content.pm.PackageInfo");
                    auto signingInfoField = packageInfoClass->getDeclaredField("signingInfo");
                    auto signingInfo = signingInfoField->get(packageInfo);

                    auto signingInfoClass = Class::forName("android.content.pm.SigningInfo");
                    auto mSigningDetailsField = signingInfoClass->getDeclaredField("mSigningDetails");
                    auto mSigningDetails = mSigningDetailsField->get(signingInfo);

                    auto signingDetailsClass = Class::forName("android.content.pm.PackageParser$SigningDetails");
                    auto signaturesField = signingDetailsClass->getDeclaredField("signatures");
                    auto pastSigningCertificatesField = signingDetailsClass->getDeclaredField("pastSigningCertificates");

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
