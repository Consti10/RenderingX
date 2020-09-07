package constantin.renderingx.core.xglview;

import android.opengl.EGL14;
import android.opengl.EGLContext;
import android.opengl.EGLDisplay;
import android.opengl.EGLSurface;

public class Helper {

    // Throws a run time exception when EGL14.eglMakeCurrent fails (should never happen,but then at least I get the proper message in the console)
    public static void eglMakeCurrentSafe(final EGLDisplay eglDisplay, EGLSurface eglSurfaceReadDraw, EGLContext eglContext) {
        //log("makeCurrent");
        boolean result= EGL14.eglMakeCurrent(eglDisplay,eglSurfaceReadDraw,eglSurfaceReadDraw,eglContext);
        if(!result){
            throw new RuntimeException("Cannot make surface current "+eglSurfaceReadDraw);
        }
    }

}
