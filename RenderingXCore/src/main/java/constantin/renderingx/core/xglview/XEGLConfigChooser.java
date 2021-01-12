package constantin.renderingx.core.xglview;

import android.opengl.EGL14;
import android.opengl.EGLConfig;
import android.opengl.EGLDisplay;
import android.opengl.EGLExt;
import android.opengl.EGLSurface;
import android.util.Log;

import javax.annotation.Nullable;


public class XEGLConfigChooser{
    private static final String TAG="MyEGLConfigChooser";
    private static final boolean DEBUG=true;

    public static final int EGL_ANDROID_front_buffer_auto_refresh = 0x314C;

    private static final int  EGL_KHR_mutable_render_buffer=0x1000;

    // Try to find an EGLConfig that exactly matches what was specified in XSurfaceParams.
    // If the first time fails, disable MSAA and try again
    // If the second time fails, disable the "mutable" extension and try again
    // Now, if the third time fails, there is no other solution than to crash the app
    public static EGLConfig chooseConfig(EGLDisplay display,final XSurfaceParams surfaceParams) {
        try{
            return getExactMatch(display,surfaceParams);
        }catch (IllegalArgumentException unused){
            printDebug(unused.toString());
        }
        //first, disable MSAA and try again
        printDebug("Disabling MSAA");
        surfaceParams.mWantedMSAALevel=0;
        try{
            return getExactMatch(display,surfaceParams);
        }catch (IllegalArgumentException unused){
            printDebug(unused.toString());
        }
        //then,disable the mutable flag and try again
        printDebug("Disabling mutable flag");
        surfaceParams.mUseMutableFlag=false;
        try {
            return getExactMatch(display,surfaceParams);
        }catch (IllegalArgumentException unused){
            throw new RuntimeException("Cannot find config that matches XSurfaceParams on this phone"+unused.getMessage());
        }
    }

    private static EGLConfig getExactMatch(EGLDisplay display,final XSurfaceParams surfaceParams) throws IllegalArgumentException{
        final int[] configSpec = new int[] {
                EGL14.EGL_RED_SIZE, surfaceParams.mR,
                EGL14.EGL_GREEN_SIZE, surfaceParams.mG,
                EGL14.EGL_BLUE_SIZE, surfaceParams.mB,
                EGL14.EGL_ALPHA_SIZE, surfaceParams.mA,
                EGL14.EGL_RENDERABLE_TYPE, surfaceParams.mUseMutableFlag ? EGLExt.EGL_OPENGL_ES3_BIT_KHR : EGL14.EGL_OPENGL_ES2_BIT, //when using mutable we request OpenGL ES 3.0
                EGL14.EGL_SURFACE_TYPE, surfaceParams.mUseMutableFlag  ? (EGL14.EGL_WINDOW_BIT | EGL_KHR_mutable_render_buffer): EGL14.EGL_WINDOW_BIT,
                //
                EGL14.EGL_CONFORMANT,EGL14.EGL_OPENGL_ES2_BIT,
                //EGL14.EGL_SAMPLE_BUFFERS, (surfaceParams.mWantedMSAALevel>0) ? 1 : 0, //if we want msaa use 1, else 0
                //EGL14.EGL_SAMPLES, surfaceParams.mWantedMSAALevel,
                EGL14.EGL_NONE
        };

        //11.01.2021: I have the suspicion that some driver(s) crash when calling eglChooseConfig with @param configs==null even though the Khronos documentation says that it should
        //be possible

        /*int[] num_config = new int[1];
        if (!EGL14.eglChooseConfig(display,configSpec,0,null,0,0,num_config,0)) {
            throw new IllegalArgumentException("eglChooseConfig failed");
        }
        int numConfigs = num_config[0];
        if (numConfigs < 1) {
            throw new IllegalArgumentException("No configs match configSpec"+numConfigs);
        }
        EGLConfig[] configs = new EGLConfig[numConfigs];
        if (!EGL14.eglChooseConfig(display,configSpec,0,configs,0,numConfigs,num_config,0)) {
            throw new IllegalArgumentException("eglChooseConfig#2 failed");
        }*/
        EGLConfig[] configs = helperEglChooseConfig(display,configSpec);
        EGLConfig config = selectConfig(display, configs, surfaceParams);
        Log.d(TAG,"Successfully selected a config");
        if (config == null) {
            throw new IllegalArgumentException("No config chosen");
        }
        return config;
    }

    // same like EGL14 eglChooseConfig but with 2 differences:
    // 1) wraps the "C-Style declaration" into a more "java-style declaration"
    // 2) workaround for weird driver bug ?
    private static EGLConfig[] helperEglChooseConfig(EGLDisplay display,final int[] configSpec) {
        try{
            int[] num_config = new int[1];
            //If configs is NULL, no configs will be returned in configs. Instead, the total number of configs matching attrib_list will be returned in *num_config.
            //In this case config_size is ignored. This form of eglChooseConfig is used to determine the number of matching frame buffer configurations,
            // followed by allocating an array of EGLConfig to pass into another call to eglChooseConfig with all other parameters unchanged.
            // https://www.khronos.org/registry/EGL/sdk/docs/man/html/eglChooseConfig.xhtml
            if (!EGL14.eglChooseConfig(display,configSpec,0,null,0,0,num_config,0)) {
                throw new IllegalArgumentException("eglChooseConfig failed");
            }
            int numConfigs = num_config[0];
            if (numConfigs < 1) {
                throw new IllegalArgumentException("No configs match configSpec"+numConfigs);
            }
            // now allocate the array that will hold the possible configuration(s)
            EGLConfig[] configs = new EGLConfig[numConfigs];
            if (!EGL14.eglChooseConfig(display,configSpec,0,configs,0,numConfigs,num_config,0)) {
                throw new IllegalArgumentException("eglChooseConfig#2 failed");
            }
            return configs;
        }catch (IllegalArgumentException e){
            //12.01.2021: Yes, some drivers throw an IllegalArgumentException when calling eglChooseConfig() with @param configs==null
            //An example for that is the Moto G (4), API level 23
            e.printStackTrace();
            Log.e(TAG,"Using weird workaround");
            final int MAX_N_CONFIGS=100;
            int[] num_config = new int[1];
            EGLConfig[] configs = new EGLConfig[MAX_N_CONFIGS];
            if (!EGL14.eglChooseConfig(display,configSpec,0,configs,0,MAX_N_CONFIGS,num_config,0)) {
                throw new IllegalArgumentException("eglChooseConfig#2 failed");
            }
            if(num_config[0]<1){
                throw new IllegalArgumentException("No configs match configSpec"+num_config[0]);
            }
            Log.d(TAG,"Actually got "+num_config[0]+" configs ");
            EGLConfig[] ret=new EGLConfig[num_config[0]];
            for(int i=0;i<num_config[0];i++){
                ret[i]=configs[i];
            }
            return ret;
        }
    }

    // given an array of EGLConfigs return the first config where all parameters match exactly what was specified in XSurfaceParams
    // If no such config can be found, return nullptr
    private static @Nullable EGLConfig selectConfig(EGLDisplay display,
                           EGLConfig[] configs, final XSurfaceParams surfaceParams) {

        for (EGLConfig config : configs) {
            // seems to be a bug on some devices
            // https://github.com/mapbox/mapbox-gl-native-android/issues/374
            // https://github.com/mapbox/mapbox-gl-native-android/pull/389/files/594ab4fe7bb3a36c4eea95780149182c6b7702de
            if(config==null){
                Log.e(TAG,"selectConfig: Config is null for some reason");
                continue;
            }
            // We want at least as much r,g,b,a
            int r = findConfigAttrib(display, config,
                    EGL14.EGL_RED_SIZE, 0);
            int g = findConfigAttrib(display, config,
                    EGL14.EGL_GREEN_SIZE, 0);
            int b = findConfigAttrib(display, config,
                    EGL14.EGL_BLUE_SIZE, 0);
            int a = findConfigAttrib(display, config,
                    EGL14.EGL_ALPHA_SIZE, 0);
            if(!(r >= surfaceParams.mR && g >= surfaceParams.mG && b >= surfaceParams.mB && a >= surfaceParams.mA )){
                printDebug("RGBA does not match");
                continue;
            }
            printDebug("RGBA okay");
            int msaaLevel=findConfigAttrib(display,config,EGL14.EGL_SAMPLES,0);
            if(msaaLevel!= surfaceParams.mWantedMSAALevel){
                printDebug("MSAA level does not match"+msaaLevel);
                continue;
            }else{
                printDebug("MSAA level okay"+msaaLevel);
            }
            int mask=findConfigAttrib(display,config, EGL14.EGL_SURFACE_TYPE,0);
            final boolean mutableMaskSet=((mask & EGL_KHR_mutable_render_buffer) != 0);
            final boolean pbufferAllowed=((mask & EGL14.EGL_PBUFFER_BIT) != 0);
            final boolean windowAllowed=((mask & EGL14.EGL_WINDOW_BIT) != 0);
            printDebug("Pixel buffer allowed :"+pbufferAllowed);
            printDebug("Window allowed:"+windowAllowed);
            // It is possible that the mutable mask is set but not needed. This is not a problem !
            // However, it is a problem if we wanted the mutable flag but didn't get it !
            printDebug("Mutable mask -needed:"+surfaceParams.mUseMutableFlag+" set:"+mutableMaskSet);
            if(surfaceParams.mUseMutableFlag && !mutableMaskSet){
                continue;
            }
            return config;
        }
        return null;
    }

    public static int findConfigAttrib(EGLDisplay display,
                                 EGLConfig config, int attribute, int defaultValue) {
        int[] value = new int[1];
        if (EGL14.eglGetConfigAttrib(display,config,attribute,value,0)) {
            return value[0];
        }
        return defaultValue;
    }

    public static void setEglSurfaceAttrib(int attribute, int value) {
        EGLDisplay eglDisplay = EGL14.eglGetCurrentDisplay();
        EGLSurface eglSurface = EGL14.eglGetCurrentSurface(EGL14.EGL_DRAW);
        if (!EGL14.eglSurfaceAttrib(eglDisplay, eglSurface, attribute, value)) {
            System.out.println("eglSurfaceAttrib() failed. attribute=" + attribute + " value=" + value);
        }
    }

    private static void printDebug(final String s){
        if(DEBUG){
            Log.d(TAG,s);
        }
    }
}
