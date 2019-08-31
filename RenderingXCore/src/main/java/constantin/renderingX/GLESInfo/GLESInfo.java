package constantin.renderingX.GLESInfo;

import android.annotation.SuppressLint;
import android.content.Context;
import android.content.SharedPreferences;
import android.opengl.EGL14;
import android.opengl.GLES20;
import android.util.Log;

import java.util.ArrayList;

import constantin.renderingx.core.BuildConfig;

import static android.content.Context.MODE_PRIVATE;

//Holds information about the EGL and GLES20 capabilities of the phone

@SuppressLint("ApplySharedPref")
public class GLESInfo {
    private static final String PREFERENCES_TAG="pref_gl_info";
    private static final String TAG="GLESInfo";

    private static final String SAVED_VERSION_CODE="SAVED_VERSION_CODE";
    private static final String SAVED_BUILD_VERSION="SAVED_BUILD_VERSION";
    //Extensions (OpenGL or EGL)
    public static final String GL_QCOM_tiled_rendering="GL_QCOM_tiled_rendering";
    public static final String EGL_KHR_reusable_sync="EGL_KHR_reusable_sync";
    public static final String EGL_ANDROID_front_buffer_auto_refresh="EGL_ANDROID_front_buffer_auto_refresh";
    public static final String EGL_KHR_mutable_render_buffer="EGL_KHR_mutable_render_buffer";
    //Other information about EGL/OpenGL
    private static final String SingleBufferedSurfaceCreatable="SingleBufferedSurfaceCreatable";
    private static final String AllMSAALevels="AllMSAALevels";
    private static final String MAX_GL_LINE_WIDTH="MAX_GL_LINE_WIDTH";


    public static boolean isExtensionAvailable(final Context c,final String extension){
        final SharedPreferences pref = c.getSharedPreferences(PREFERENCES_TAG, MODE_PRIVATE);
        return pref.getBoolean(extension,false);
    }

    public static ArrayList<Integer> availableMSAALevels(final Context context){
        final SharedPreferences pref = context.getSharedPreferences(PREFERENCES_TAG, MODE_PRIVATE);
        final String allMSAALevels=pref.getString(AllMSAALevels,"0#");
        final String[] asStringArray=allMSAALevels.split("#");
        final ArrayList<Integer> asIntList=new ArrayList<>();
        for(final String s:asStringArray){
            int msaaLevel=Integer.parseInt(s);
            asIntList.add(msaaLevel);
        }
        return asIntList;
    }

    //write values when either a) the library was updated or b) the os (android) was updated
    static boolean shouldWriteValues(final Context c){
        final SharedPreferences pref = c.getSharedPreferences(PREFERENCES_TAG, MODE_PRIVATE);
        return pref.getInt(SAVED_VERSION_CODE, 0) != BuildConfig.VERSION_CODE ||
                pref.getInt(SAVED_BUILD_VERSION, 0) != android.os.Build.VERSION.SDK_INT;
    }
    static void wroteValues(final Context c){
        final SharedPreferences pref = c.getSharedPreferences(PREFERENCES_TAG, MODE_PRIVATE);
        pref.edit().putInt(SAVED_VERSION_CODE, BuildConfig.VERSION_CODE).putInt(SAVED_BUILD_VERSION,android.os.Build.VERSION.SDK_INT).commit();
    }


    static void writeResultsMSAA(final Context c, final String allMSAALevels){
        SharedPreferences pref = c.getSharedPreferences(PREFERENCES_TAG, MODE_PRIVATE);
        SharedPreferences.Editor editor=pref.edit();
        editor.putString(AllMSAALevels,allMSAALevels);
        editor.commit();
    }

    //call this from a thread with valid GLES20 context
    static void writeResultsWithGLESContextBound(final Context c){
        String eglExtensions = EGL14.eglQueryString(EGL14.eglGetCurrentDisplay(), EGL14.EGL_EXTENSIONS);
        //System.out.println(eglExtensions);
        isEGL_KHR_reusable_syncAvailable(eglExtensions);
        String gles20Extensions = GLES20.glGetString(GLES20.GL_EXTENSIONS);
        //System.out.println(gles20Extensions);
        //
        float[] lineWidthRange= {0.0f, 0.0f};
        GLES20.glGetFloatv(GLES20.GL_LINE_WIDTH, lineWidthRange,0);
        //System.out.println("non aliased: "+lineWidthRange[0]+lineWidthRange[1]);
        GLES20.glGetFloatv(GLES20.GL_ALIASED_LINE_WIDTH_RANGE, lineWidthRange,0);
        //System.out.println("Aliased: "+lineWidthRange[0]+lineWidthRange[1]);
        final int MAX_LINE_WIDTH=(int)lineWidthRange[1];
        //
        SharedPreferences pref = c.getSharedPreferences(PREFERENCES_TAG, MODE_PRIVATE);
        SharedPreferences.Editor editor = pref.edit();
        editor.putBoolean(GL_QCOM_tiled_rendering, isGL_QCOM_tiled_renderingAvailable(gles20Extensions));
        editor.putBoolean(EGL_KHR_reusable_sync, isEGL_KHR_reusable_syncAvailable(eglExtensions));
        editor.putBoolean(EGL_KHR_mutable_render_buffer,isEGL_KHR_mutable_render_bufferAvailable(eglExtensions));
        editor.putBoolean(EGL_ANDROID_front_buffer_auto_refresh, isEGL_ANDROID_front_buffer_auto_refreshAvailable(eglExtensions));
        editor.putBoolean(SingleBufferedSurfaceCreatable, isCurrSurfaceSingleBuffered());
        editor.putInt(MAX_GL_LINE_WIDTH,MAX_LINE_WIDTH);
        editor.commit();
    }


    private static boolean isCurrSurfaceSingleBuffered(){
        boolean ret= surfaceHasAttributeWithValue(EGL14.EGL_RENDER_BUFFER, EGL14.EGL_SINGLE_BUFFER);
        if(ret){
            Log.d(TAG,"Surface is single buffered");
        }else{
            Log.d(TAG,"Surface is not single buffered");
        }
        return ret;
    }
    private static boolean surfaceHasAttributeWithValue(int attribute, int value) {
        int[] values = new int[1];
        EGL14.eglQuerySurface(EGL14.eglGetDisplay(EGL14.EGL_DEFAULT_DISPLAY),
                EGL14.eglGetCurrentSurface(EGL14.EGL_DRAW), attribute, values, 0);
        return values[0] == value;
    }
    private static boolean isGL_QCOM_tiled_renderingAvailable(String extensions){
        if(extensions.contains("GL_QCOM_tiled_rendering")){
            Log.d(TAG,"GL_QCOM_tiled_rendering is available");
            return true;
        }
        Log.d(TAG,"GL_QCOM_tiled_rendering is not available");
        return false;
    }
    private static boolean isEGL_KHR_reusable_syncAvailable(String extensions){
        if(extensions.contains("EGL_KHR_reusable_sync")){
            Log.d(TAG,"EGL_KHR_reusable_sync is available");
            return true;
        }
        Log.d(TAG,"EGL_KHR_reusable_sync is not available");
        return false;
    }
    private static boolean isEGL_KHR_mutable_render_bufferAvailable(String extensions){
        if(extensions.contains("EGL_KHR_mutable_render_buffer")){
            Log.d(TAG,"EGL_KHR_mutable_render_buffer is available");
            return true;
        }
        Log.d(TAG,"EGL_KHR_mutable_render_buffer is not available");
        return false;
    }
    private static boolean isEGL_ANDROID_front_buffer_auto_refreshAvailable(String eglExtensions){
        if(eglExtensions.contains("EGL_ANDROID_front_buffer_auto_refresh")){
            Log.d(TAG,"EGL_ANDROID_front_buffer_auto_refresh is available");
            return true;
        }
        Log.d(TAG,"EGL_ANDROID_front_buffer_auto_refresh is not available");
        return false;
    }
}
