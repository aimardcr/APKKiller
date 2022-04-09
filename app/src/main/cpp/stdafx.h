#include <jni.h>
#include <errno.h>

#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <inttypes.h>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <sstream>
#include <vector>
#include <map>
#include <iomanip>
#include <thread>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/resource.h>
#include <sys/uio.h>
#include <dlfcn.h>

#include <fcntl.h>
#include <android/log.h>
#include <pthread.h>
#include <dirent.h>
#include <list>
#include <libgen.h>
#include <link.h>

#include <sys/mman.h>
#include <sys/wait.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/un.h>

#include <EGL/egl.h>
#include <GLES/gl.h>
#include <GLES3/gl3.h>

#include <sys/system_properties.h>
#include <android_native_app_glue.h>

#include <android\asset_manager.h>
#include <android\asset_manager_jni.h>

#include <codecvt>
#include <chrono>
#include <queue>

using namespace std::chrono_literals;

#define RegisterHook(target, hook, original) Tools::Hook((void *)(target), (void *)(hook), (void **)(original))
#define WriteBytes(addr, value, size) Tools::WriteAddr((void *)(addr), (void *)(value), (size))
#define DefineHook(ret, name, args) ret (*orig_##name)args; \
                                    ret name args

#define PACKAGE_NAME "com.ea.gp.apexlegendsmobilefps"