package constantin.renderingx.core.old;

import android.opengl.GLSurfaceView;
import android.util.Log;

import javax.microedition.khronos.egl.EGL10;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.egl.EGLContext;
import javax.microedition.khronos.egl.EGLDisplay;

import static android.opengl.EGLExt.EGL_CONTEXT_FLAGS_KHR;

// Same as default but adds the 'DEBUG' flag to context

public class DebugEGLContextFactory implements GLSurfaceView.EGLContextFactory {
    public static final int EGL_CONTEXT_CLIENT_VERSION = 0x3098;
    public static final int EGL_CONTEXT_OPENGL_DEBUG_BIT_KHR= 0x00000001;

    public EGLContext createContext(EGL10 egl, EGLDisplay display, EGLConfig config) {
        int[] attrib_list = {EGL_CONTEXT_CLIENT_VERSION, 2,
                EGL_CONTEXT_FLAGS_KHR, EGL_CONTEXT_OPENGL_DEBUG_BIT_KHR,
                EGL10.EGL_NONE };
        Log.w("EGLWindowSurfaceFactory","enabled debug for context");

        return egl.eglCreateContext(display, config, EGL10.EGL_NO_CONTEXT,
                2 != 0 ? attrib_list : null);
    }

    public void destroyContext(EGL10 egl, EGLDisplay display,
                               EGLContext context) {
        if (!egl.eglDestroyContext(display, context)) {
            Log.e("DefaultContextFactory", "display:" + display + " context: " + context);
        }
    }
}
