package constantin.renderingx.core.xglview;

import android.opengl.EGL14;
import android.opengl.EGLConfig;
import android.opengl.EGLContext;
import android.opengl.EGLDisplay;
import android.opengl.EGLSurface;
import android.util.Log;

import static android.opengl.EGL14.EGL_DEFAULT_DISPLAY;
import static android.opengl.EGL14.EGL_HEIGHT;
import static android.opengl.EGL14.EGL_NONE;
import static android.opengl.EGL14.EGL_NO_CONTEXT;
import static android.opengl.EGL14.EGL_NO_DISPLAY;
import static android.opengl.EGL14.EGL_NO_SURFACE;
import static android.opengl.EGL14.EGL_WIDTH;

public class GLContextSurfaceLess {
    private static final String TAG=" GLContextSurfaceLess";
    private EGLDisplay eglDisplay = EGL_NO_DISPLAY;
    private EGLSurface eglSurface = EGL_NO_SURFACE;
    private EGLContext eglContext = EGL_NO_CONTEXT;
    private EGLConfig eglConfig = null;

    private Thread renderThread;

    private final SecondarySharedContext secondarySharedContext;
    public GLContextSurfaceLess(SecondarySharedContext i){
        secondarySharedContext=i;
    }

    public void create(){
        eglDisplay = EGL14.eglGetDisplay(EGL_DEFAULT_DISPLAY);
        int[] major = new int[]{0};
        int[] minor = new int[]{0};
        EGL14.eglInitialize(eglDisplay, major, 0, minor, 0);
        eglConfig = XEGLConfigChooser.chooseConfig(eglDisplay,new XSurfaceParams(8,0,false));
        final int[] contextAttributes = new int[]{
                EGL14.EGL_CONTEXT_CLIENT_VERSION, 2,
                EGL14.EGL_NONE
        };
        // https://www.khronos.org/registry/EGL/sdk/docs/man/html/eglCreateContext.xhtml
        eglContext = EGL14.eglCreateContext(eglDisplay, eglConfig, EGL_NO_CONTEXT, contextAttributes, 0);
        if (eglContext==EGL_NO_CONTEXT) {
            throw new AssertionError("Cannot create eglContext");
        }
        String extensions=EGL14.eglQueryString(eglDisplay,EGL14.EGL_EXTENSIONS);
        if(extensions.contains("EGL_KHR_surfaceless_context")){
            Log.d(TAG,"Got EGL_KHR_surfaceless_context extension");
            eglSurface=EGL_NO_SURFACE;
        }else{
            // The egl context surface less extension is not available on all devices. As a workaround, create a small but unused pbuffer if needed
            eglSurface=createDefaultPBufferSurface(eglDisplay,eglConfig);
        }
    }

    public void resumeWork(){
        renderThread=new Thread(mOpenGLRunnable);
        renderThread.start();
    }

    public void pauseWork(){
        renderThread.interrupt();
        try {
            renderThread.join();
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
    }

    public void destroy(){
        // surface can be null due to extension
        if(eglSurface!=EGL_NO_SURFACE){
            EGL14.eglDestroySurface(eglDisplay, eglSurface);
        }
        EGL14.eglDestroyContext(eglDisplay, eglContext);
        EGL14.eglTerminate(eglDisplay);
    }


    public EGLContext getEglContext(){
        return eglContext;
    }


    private final Runnable mOpenGLRunnable=new Runnable() {
        @Override
        public void run() {
            Thread.currentThread().setName("SecondRenderer");
            eglMakeCurrentSafe(eglDisplay,eglSurface,eglContext);
            secondarySharedContext.onSecondaryContextCreated();
            while(!Thread.currentThread().isInterrupted()){
                secondarySharedContext.onSecondaryContextDoWork();
            }
            eglMakeCurrentSafe(eglDisplay,EGL_NO_SURFACE,EGL_NO_CONTEXT);
        }
    };

    // Create a small pbuffer surface for EGL context's that do not support surfaceless
    private static EGLSurface createDefaultPBufferSurface(final EGLDisplay eglDisplay,final EGLConfig eglConfig){
        int[] attrib_list = {EGL_WIDTH, 16, EGL_HEIGHT, 16, EGL_NONE };
        final EGLSurface eglSurface=EGL14.eglCreatePbufferSurface(eglDisplay,eglConfig,attrib_list,0);
        if(eglSurface==EGL_NO_SURFACE){
            throw new AssertionError("Cannot create PBuffer surface");
        }
        return eglSurface;
    }

    private static void eglMakeCurrentSafe(final EGLDisplay eglDisplay, EGLSurface eglSurface,EGLContext eglContext) {
        //log("makeCurrent");
        boolean result= EGL14.eglMakeCurrent(eglDisplay,eglSurface,eglSurface,eglContext);
        if(!result){
            throw new AssertionError("Cannot make surface current "+eglSurface);
        }
    }

    public interface SecondarySharedContext{
        public void onSecondaryContextCreated();
        public void  onSecondaryContextDoWork();
    }
}
