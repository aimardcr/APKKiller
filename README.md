# APKKiller
APKKiller is a method to bypass android application integrity and signature check.
# How does it work?
When an android application is loaded, it stores various information regarding current running Application like App Name, Package Name, Signature, APK Path, etc.
You can't access those information using normal code, but with _Reflection_ you access, read and write new data to those internal classes & fields.

APKKiller uses the advantage of _Reflection_ to access hidden information of the android app such as _Application Signature_ or _Application APK Path_ and replace it with a new data so that the application thinks its _Signature_ is still the original one even when the APK file is already being tampered and resigned using a new signature.
# How to use it?
1. Get the target app original _Signature_ using [APKSignReader for Android](https://github.com/aimardcr/APKSignReader) or [APKSignReader for Windows (Only supports for v1 Signature Scheme)](https://github.com/aimardcr/CS-APKSignReader)
2. Change **apk_signatures** in `APKKiller.h` using the resuslt of _APKSignReader_
3. Build the APKKiller Project to APK
4. Decompile both APKKiller APK and Target APK
5. Copy smali from `com/kuro` to the Target APK smali
6. Call `Start` function on the target app `attachBaseContext` (Application) or `onCreate` (Activity)

For example:

**attachBaseContext**

![image](https://user-images.githubusercontent.com/41464808/162587798-6eb4cc25-c1e2-4ed6-a49d-ae11e8b4b5ba.png)

**onCreate**

![image](https://user-images.githubusercontent.com/41464808/162587846-7e00f933-e1b1-4cef-87eb-6120d4e93120.png)

7. Copy Target original APK file to `<decompile_target_app_dir>/assets/original.apk`
8. Compile Target App and test it!
