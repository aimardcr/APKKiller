package com.kuro;

import android.content.Context;

import java.lang.reflect.InvocationHandler;
import java.lang.reflect.Method;

@SuppressWarnings("all")
public class APKKiller {
    private static String m_APKSign = ""; // Paste the base64 encoded signature here or use the C++ one instead
    static {
        System.loadLibrary("kuro");
    }

    public static void Kill() {

    }

    public static native void Start(Context context);
    public static native Object processInvoke(Method method, Object[] args);

    private static InvocationHandler myInvocationHandler = new InvocationHandler() {
        @Override
        public Object invoke(Object proxy, Method method, Object[] args) throws Throwable {
            return processInvoke(method, args);
        }
    };
}