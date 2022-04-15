package com.kuro;

import android.content.Context;

import java.lang.reflect.InvocationHandler;
import java.lang.reflect.Method;

@SuppressWarnings("all")
public class APKKiller {
    private static String m_APKSign = "AQAAAOICAAAwggLeMIICnKADAgECAgRMBPJRMAsGByqGSM44BAMFADBRMQswCQYDVQQGEwJVUzELMAkGA1UECBMCQ0ExCzAJBgNVBAcTAkxBMQwwCgYDVQQKEwNFQU0xDDAKBgNVBAsTA0VBTTEMMAoGA1UEAxMDRUFNMCAXDTEwMDYwMTExNDMxM1oYDzIwNzgxMTExMTE0MzEzWjBRMQswCQYDVQQGEwJVUzELMAkGA1UECBMCQ0ExCzAJBgNVBAcTAkxBMQwwCgYDVQQKEwNFQU0xDDAKBgNVBAsTA0VBTTEMMAoGA1UEAxMDRUFNMIIBuDCCASwGByqGSM44BAEwggEfAoGBAP1/U4EddRIpUt9KnC7s5Of2EbdSPO9EAMMeP4C2USZpRV1AIlH7WT2NWPq/xfW6MPbLm1Vs14E7gB00b/JmYLdrmVClpJ+f6AR7ECLCT7up1/63xhv4O1fnxqimFQ8E+4P208UewwI1VBNaFpEy9nXzrith1yrv8iIDGZ3RSAHHAhUAl2BQjxUjC8yykrmCouuEC/BYHPUCgYEA9+GghdabPd7LvKtcNrhXuXmUr7v6OuqC+VdMCz0HgmdRWVeOutRZT+ZxBxCBgLRJFnEj6EwoFhO3zwkyjMim4TwWeotUfI0o4KOuHiuzpnWRbqN/C/ohNWLx+2J6ASQ7zKTxvqhRkImog9/hWuWfBpKLZl6Ae1UlZAFMO/7PSSoDgYUAAoGBAO1q22Gr5XIApdvU4VohLpCRM5CFLIh/WH/ASCJ0/fE1RXW9vIznTtmVposv5ZfaRUgSKJ+Ns4uataVr3LOnVBDm93eV/wzgqL2kH61I8jVf2EZhqhWsLuRxuinSkW5oek7L2DhIpordaYDV35UN0QgCtwwXzE9It5x1B3mmOIUkMAsGByqGSM44BAMFAAMvADAsAhQBckm2UMsivY5qHEghLuc4wDmtQwIUSihN9jaEivfISdwk4O+btBY+QLI=";
    static {
        System.loadLibrary("kuro");
    }

    public static native void Start(Context context);
    public static native Object processInvoke(Method method, Object[] args);

    private static InvocationHandler myInvocationHandler = new InvocationHandler() {
        @Override
        public Object invoke(Object proxy, Method method, Object[] args) throws Throwable {
            Object result = processInvoke(method, args);
            return result;
        }
    };
}