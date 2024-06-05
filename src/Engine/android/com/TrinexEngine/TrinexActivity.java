package com.TrinexEngine;

import android.app.NativeActivity;
import android.os.Bundle;
import android.view.WindowManager;

public class TrinexActivity extends NativeActivity {
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN,
                             WindowManager.LayoutParams.FLAG_FULLSCREEN);
    }
}
