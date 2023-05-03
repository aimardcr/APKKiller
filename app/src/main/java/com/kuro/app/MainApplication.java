package com.kuro.app;

import android.app.Application;
import android.content.Context;

import com.kuro.APKKiller;

public class MainApplication extends Application {
    @Override
    protected void attachBaseContext(Context base) {
        super.attachBaseContext(base);
        APKKiller.Start(base);
    }
}
