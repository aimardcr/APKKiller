package com.kuro;

import android.content.Context;

import java.lang.reflect.InvocationHandler;
import java.lang.reflect.Method;

@SuppressWarnings("all")
public class APKKiller {
    static {
        System.loadLibrary("killer");
    }

    public static native void Start(Context context);
    public static native Object processInvoke(Method method, Object[] args);

    private static InvocationHandler myInvocationHandler = new InvocationHandler() {
        @Override
        public Object invoke(Object proxy, Method method, Object[] args) {
            return processInvoke(method, args);
        }
    };
}