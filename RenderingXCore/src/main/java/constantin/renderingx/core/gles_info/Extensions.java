package constantin.renderingx.core.gles_info;

import android.annotation.SuppressLint;
import android.content.Context;
import android.content.SharedPreferences;
import android.opengl.EGL14;
import android.opengl.GLES20;

import static android.content.Context.MODE_PRIVATE;

// OpenGL btw EGL extensions are available if you can find their name in the queried list of supported names
public class Extensions {
    private static final String PREF_NAME="pref_gl_egl_extensions";
    // GL extensions
    public static final String GL_QCOM_tiled_rendering="GL_QCOM_tiled_rendering";
    // EGL extensions
    public static final String EGL_KHR_reusable_sync="EGL_KHR_reusable_sync";
    public static final String EGL_ANDROID_front_buffer_auto_refresh="EGL_ANDROID_front_buffer_auto_refresh";
    public static final String EGL_KHR_mutable_render_buffer="EGL_KHR_mutable_render_buffer";

    // Call this with OpenGL context bound
    @SuppressLint("ApplySharedPref")
    public static void storeInSharedPreference(final Context c){
        final String gles20Extensions = GLES20.glGetString(GLES20.GL_EXTENSIONS);
        final String eglExtensions = EGL14.eglQueryString(EGL14.eglGetCurrentDisplay(), EGL14.EGL_EXTENSIONS);
        System.out.println(eglExtensions);
        System.out.println(gles20Extensions);
        final SharedPreferences pref = c.getSharedPreferences(PREF_NAME, MODE_PRIVATE);
        SharedPreferences.Editor editor=pref.edit();
        // Clear to remove any old values
        editor.clear().commit();
        final String[] gles20ExtensionsAsList = gles20Extensions.split(" ");
        for(final String extension: gles20ExtensionsAsList){
            editor.putBoolean(extension,true);
        }
        final String[]  eglExtensionsAsList = eglExtensions.split(" ");
        for(final String extension: eglExtensionsAsList){
            editor.putBoolean(extension,true);
        }
        editor.commit();
    }

    // Returns true if the queried extension is available
    public static boolean available(final Context c, final String extension){
        final SharedPreferences pref = c.getSharedPreferences(PREF_NAME, MODE_PRIVATE);
        return pref.getBoolean(extension,false);
    }

    private static boolean available(final SharedPreferences sharedPreferences, final String extension){
        return sharedPreferences.getBoolean(extension,false);
    }
}
