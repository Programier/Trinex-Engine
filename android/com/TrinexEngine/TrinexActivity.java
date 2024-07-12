package com.TrinexEngine;

import android.app.NativeActivity;
import android.content.res.Configuration;
import android.graphics.Point;
import android.os.Bundle;
import android.util.Log;
import android.view.Display;
import android.view.View;
import android.view.Surface;
import android.view.WindowManager;
import android.content.Context;

public class TrinexActivity extends NativeActivity
{
    static
    {
        try
        {
            System.loadLibrary("TrinexEngine");
        }
        catch (UnsatisfiedLinkError error)
        {
            Log.e("TrinexEngine", error.getMessage());
        }
    }

    private int getOrientation()
    {
        Display display = ((WindowManager) getSystemService(Context.WINDOW_SERVICE)).getDefaultDisplay();
        int rotation    = display.getRotation();
        int orientation = getResources().getConfiguration().orientation;

        switch (orientation)
        {
            case Configuration.ORIENTATION_LANDSCAPE:
                if (rotation == Surface.ROTATION_0 || rotation == Surface.ROTATION_180)
                {
                    return 0;// Landscape
                }
                else
                {
                    return 1;// LandscapeFlipped
                }
            case Configuration.ORIENTATION_PORTRAIT:
                if (rotation == Surface.ROTATION_0 || rotation == Surface.ROTATION_180)
                {
                    return 2;// Portrait
                }
                else
                {
                    return 3;// PortraitFlipped
                }
            default:
                return -1;// Undefined
        }
    }

    @Override
    protected void onCreate(Bundle saved_instance_state)
    {
        String app_package_name    = getPackageName();
        String device_manufacturer = android.os.Build.MANUFACTURER;
        String device_model        = android.os.Build.MODEL;
        String device_build_number = android.os.Build.DISPLAY;
        String system_version      = android.os.Build.VERSION.RELEASE;
        String system_language     = java.util.Locale.getDefault().toString();
        Display display            = getWindowManager().getDefaultDisplay();
        Point display_real_size    = new Point();
        display.getRealSize(display_real_size);
        int screen_width       = display_real_size.x;
        int screen_height      = display_real_size.y;
        String cache_dir       = "None";
        String executable_path = getApplicationInfo().nativeLibraryDir + "/libTrinexEngine.so";

        initializePlatformInfo(app_package_name, device_manufacturer, device_model, device_build_number, system_version,
                               system_language, cache_dir, executable_path, getApplicationInfo().nativeLibraryDir, screen_width,
                               screen_height);

        updateOrientation(getOrientation());
        super.onCreate(saved_instance_state);
        makeFullscreen();

    }

    @Override 
    public void onConfigurationChanged(Configuration new_сonfig)
    {
        super.onConfigurationChanged(new_сonfig);
        updateOrientation(getOrientation());
    }

    void makeFullscreen()
    {    
        View decor_view = getWindow().getDecorView();
        decor_view.setSystemUiVisibility(View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY | View.SYSTEM_UI_FLAG_LAYOUT_STABLE |
                                         View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION | View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN |
                                         View.SYSTEM_UI_FLAG_HIDE_NAVIGATION | View.SYSTEM_UI_FLAG_FULLSCREEN);
    }

    public native void initializePlatformInfo(String app_package_name, String device_manufacturer, String device_model,
                                              String device_build_number, String system_version, String system_language,
                                              String cache_dir, String executable_path, String libraries_path, int screen_Wwdth,
                                              int screen_height);
    public native void updateOrientation(int orientation);
}
