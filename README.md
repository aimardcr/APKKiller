# What is it?
APKKiller is a method to bypass various android application security system check such as Signature Verification, Integrity Check, etc. APKKiller uses JNI & Reflection to bypass _Hidden API Restriction_, however there is no guarantee that APKKiller will bypass all android application security. APKKiller is made for educational purpose only, use with discretion.

## How does it work?
When an android application is loaded, it stores various information regarding current running Application like App Name, Package Name, Signature, APK Path, etc.
You can't access those information normal way, but with _Reflection_ you access, read and write new data to those internal classes & fields. 

These informations are stored in a class like [AppBindData](https://android.googlesource.com/platform/frameworks/base/+/master/core/java/android/app/ActivityThread.java#855), [LoadedApk](https://android.googlesource.com/platform/frameworks/base/+/master/core/java/android/app/LoadedApk.java), [ApplicationInfo](https://android.googlesource.com/platform/frameworks/base/+/master/core/java/android/content/pm/ApplicationInfo.java), etc.

APKKiller changes data on those classes to spoof current application information such APK Path, APK Signatures, APK Installer Information, etc. APKKiller is not guaranteed to work on all apps/games, but it is guaranteed to bypass majorities of application security system.

## How to use it?
1. Get the target app original _Signature_ using [APK Sign Reader](https://github.com/aimardcr/APKSignReader) or [APK Sign Reader (for windows)](https://github.com/aimardcr/APKSignReader-Java)
2. Change **apk_signatures** in `APKKiller.h` using the result of _APKSignReader_
3. Build the APKKiller Project to APK
4. Decompile both APKKiller APK and Target APK
5. Copy smali from `com/kuro` (APKKiller smali) to the Target App smali
6. Copy libs from APKKiller APK to Target APK (Make sure only copy same ABIs as the Target App, for example if Target App has only armeabi-v7a, then you should only copy armeabi-v7a)
6. Locate Target App entry point in the smali, you can do this by taking a look at `AndroidManifest.xml`
7. Call `Start` function on the target app `attachBaseContext` (Application) or `onCreate` (Activity) [Preferrably attachBaseContext]

For example:

**attachBaseContext**

![image](https://user-images.githubusercontent.com/41464808/162587798-6eb4cc25-c1e2-4ed6-a49d-ae11e8b4b5ba.png)

**onCreate**

![image](https://user-images.githubusercontent.com/41464808/162587846-7e00f933-e1b1-4cef-87eb-6120d4e93120.png)

7. Copy Target original APK file to `<decompile_target_app_dir>/assets/original.apk`
8. Compile Target App and test it!
