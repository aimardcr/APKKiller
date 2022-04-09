package com.kuro;

import android.content.Context;

import java.lang.reflect.InvocationHandler;
import java.lang.reflect.Method;

@SuppressWarnings("all")
public class APKKiller {
    static {
        System.loadLibrary("kuro");
    }

    public static native void Start(Context context);
    public static native Object nativeInvoke(Object obj, Method method, Object[] args);

    private static InvocationHandler myInvocationHandler = new InvocationHandler() {
        @Override
        public Object invoke(Object proxy, Method method, Object[] args) throws Throwable {
            return nativeInvoke(proxy, method, args);
        }
    };
}