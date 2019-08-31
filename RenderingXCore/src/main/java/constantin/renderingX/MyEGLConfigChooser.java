package constantin.renderingX;

import android.opengl.EGL14;
import android.opengl.GLSurfaceView;
import android.util.Log;

import javax.microedition.khronos.egl.EGL10;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.egl.EGLDisplay;

//https://android.googlesource.com/platform/cts/+/347b539/tests/vr/src/android/vr/cts/OpenGLESActivity.java
//EGL10.EGL_SURFACE_TYPE, EGL10.EGL_WINDOW_BIT | EGL_MUTABLE_RENDER_BUFFER_BIT,
//EGL10.EGL_SURFACE_TYPE, EGL10.EGL_WINDOW_BIT,
//Allows setting the 'mutable' flag for changing the frame buffer between single/double buffered

public class MyEGLConfigChooser implements GLSurfaceView.EGLConfigChooser {
    private static final String TAG="MyEGLConfigChooser";
    private static final boolean DEBUG=true;

    private static final int  EGL_KHR_mutable_render_buffer=0x1000;
    private static final int EGL_OPENGL_ES3_BIT_KHR = 0x40;
    private static final int EGL_OPENGL_ES2_BIT = 0x0004;

    private final Params mParams;

    private class Params{
        final int mR=8,mG=8,mB=8;
        final int mA;
        int mWantedMSAALevel;
        boolean mUseMutableFlag;
        Params(final int a,final int msaaLevel,final boolean useMutable){
            mA=a;
            mWantedMSAALevel=msaaLevel;
            mUseMutableFlag=useMutable;
        }
    }

    public MyEGLConfigChooser(boolean useMutable, int wantedMSAALevel, boolean needAlpha){
        mParams=new Params(needAlpha ? 8 : 0,wantedMSAALevel,useMutable);
    }

    public MyEGLConfigChooser(boolean useMutable, int wantedMSAALevel){
        mParams=new Params(0,wantedMSAALevel,useMutable);
    }

    @Override
    public EGLConfig chooseConfig(EGL10 egl, EGLDisplay display) {

        try{
            return getExactMatch(egl,display,mParams);
        }catch (IllegalArgumentException unused){
            printDebug(unused.toString());
        }

        //first, disable MSAA and try again
        printDebug("Disabling MSAA");
        mParams.mWantedMSAALevel=0;
        try{
            return getExactMatch(egl,display,mParams);
        }catch (IllegalArgumentException unused){
            printDebug(unused.toString());
        }

        //then,disable the mutable flag and try again
        printDebug("Disabling mutable flag");
        mParams.mUseMutableFlag=false;
        return getExactMatch(egl,display,mParams);
    }


    private static EGLConfig getExactMatch(EGL10 egl,EGLDisplay display,final Params params) throws IllegalArgumentException{
        final int[] configSpec = new int[] {
                EGL10.EGL_RED_SIZE, params.mR,
                EGL10.EGL_GREEN_SIZE, params.mG,
                EGL10.EGL_BLUE_SIZE, params.mB,
                EGL10.EGL_ALPHA_SIZE, params.mA,
                EGL10.EGL_RENDERABLE_TYPE, params.mUseMutableFlag ? EGL_OPENGL_ES3_BIT_KHR : EGL_OPENGL_ES2_BIT, //when using mutable we request OpenGL ES 3.0
                EGL10.EGL_SURFACE_TYPE,params.mUseMutableFlag  ? (EGL14.EGL_WINDOW_BIT | EGL_KHR_mutable_render_buffer): EGL14.EGL_WINDOW_BIT,
                //
                EGL14.EGL_SAMPLE_BUFFERS, (params.mWantedMSAALevel>0) ? 1 : 0, //if we want msaa use 1, else 0
                EGL14.EGL_SAMPLES, params.mWantedMSAALevel,
                EGL10.EGL_NONE
        };
        int[] num_config = new int[1];
        if (!egl.eglChooseConfig(display, configSpec, null, 0, num_config)) {
            throw new IllegalArgumentException("eglChooseConfig failed");
        }
        int numConfigs = num_config[0];
        if (numConfigs <= 0) {
            throw new IllegalArgumentException("No configs match configSpec");
        }
        EGLConfig[] configs = new EGLConfig[numConfigs];
        if (!egl.eglChooseConfig(display, configSpec, configs, numConfigs,
                num_config)) {
            throw new IllegalArgumentException("eglChooseConfig#2 failed");
        }
        EGLConfig config = selectConfig(egl, display, configs,params);
        if (config == null) {
            throw new IllegalArgumentException("No config chosen");
        }
        return config;
    }

    private static EGLConfig selectConfig(EGL10 egl, EGLDisplay display,
                                  EGLConfig[] configs,final Params params) {

        for (EGLConfig config : configs) {
            // We want at least as much r,g,b,a
            int r = findConfigAttrib(egl, display, config,
                    EGL10.EGL_RED_SIZE, 0);
            int g = findConfigAttrib(egl, display, config,
                    EGL10.EGL_GREEN_SIZE, 0);
            int b = findConfigAttrib(egl, display, config,
                    EGL10.EGL_BLUE_SIZE, 0);
            int a = findConfigAttrib(egl, display, config,
                    EGL10.EGL_ALPHA_SIZE, 0);
            if(!(r >= params.mR && g >= params.mG && b >= params.mB && a >= params.mA )){
                printDebug("RGBA does not match");
                continue;
            }
            printDebug("RGBA okay");

            int msaaLevel=findConfigAttrib(egl,display,config,EGL10.EGL_SAMPLES,0);
            if(msaaLevel!=params.mWantedMSAALevel){
                printDebug("MSAA level does not match"+msaaLevel);
                continue;
            }else{
                printDebug("MSAA level okay"+msaaLevel);
            }

            int mask=findConfigAttrib(egl,display,config,
                    EGL10.EGL_SURFACE_TYPE,0);
            final boolean mutableMaskSet=((mask & EGL_KHR_mutable_render_buffer) != 0);
            printDebug("Mutable mask set:"+mutableMaskSet);
            if(params.mUseMutableFlag && !mutableMaskSet){
                continue;
            }
            return config;
        }

        return null;
    }

    public static int findConfigAttrib(EGL10 egl, EGLDisplay display,
                                 EGLConfig config, int attribute, int defaultValue) {
        int[] value = new int[1];
        if (egl.eglGetConfigAttrib(display, config, attribute, value)) {
            return value[0];
        }
        return defaultValue;
    }

    public static void setEglSurfaceAttrib(int var1, int var2) {
        android.opengl.EGLDisplay var3 = EGL14.eglGetCurrentDisplay();
        android.opengl.EGLSurface var4 = EGL14.eglGetCurrentSurface(EGL14.EGL_DRAW);
        if (!EGL14.eglSurfaceAttrib(var3, var4, var1, var2)) {
            System.out.println("eglSurfaceAttrib() failed. attribute=" + var1 + " value=" + var2);
        }
    }


    private static void printDebug(final String s){
        if(DEBUG){
            Log.d(TAG,s);
        }
    }
}
