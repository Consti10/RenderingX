package constantin.renderingx.core.xglview;

import android.content.Context;
import android.opengl.EGL14;
import android.opengl.EGLConfig;
import android.opengl.EGLContext;
import android.opengl.EGLDisplay;
import android.opengl.EGLSurface;
import android.opengl.GLSurfaceView;
import android.os.Process;
import android.util.AttributeSet;
import android.util.Log;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

import androidx.appcompat.app.AppCompatActivity;
import androidx.lifecycle.Lifecycle;
import androidx.lifecycle.LifecycleObserver;
import androidx.lifecycle.OnLifecycleEvent;


import java.util.concurrent.atomic.AtomicBoolean;

import static android.opengl.EGL14.EGL_DEFAULT_DISPLAY;
import static android.opengl.EGL14.EGL_NO_DISPLAY;
import static android.opengl.EGL14.EGL_NO_SURFACE;
import static android.opengl.EGL14.EGL_NO_CONTEXT;
import static constantin.renderingx.core.xglview.XEGLConfigChooser.EGL_ANDROID_front_buffer_auto_refresh;

// TODO in Development
// First step: Make it usable everywhere haha :)

/**
 * This View is intended as an replacement for GLSurfaceView.
 * The Complexity of GLSurfaceView comes from its compatibility all the way down to Android 2.3.3 (API level 10) where for example EGL14 was not available
 * By replacing EGL10 with EGL14 and also having not to worry about 'hacks' that were needed on these old api versions I hope to reduce complexity
 * It is also going to be easier to add more specialized features,for example multiple OpenGL context(s) aka 'Shared context'
 */
public class XGLSurfaceView extends SurfaceView implements LifecycleObserver, SurfaceHolder.Callback {
    final AppCompatActivity activity;
    EGLDisplay eglDisplay = EGL_NO_DISPLAY;
    EGLSurface eglSurface = EGL_NO_SURFACE;
    EGLContext eglContext = EGL_NO_CONTEXT;
    EGLConfig eglConfig = null;
    private final Object eglSurfaceAvailable=new Object();

    private Thread mOpenGLThread;
    private GLSurfaceView.Renderer mRenderer;
    private Renderer2 mRenderer2;
    private int SURFACE_W,SURFACE_H;
    private boolean firstTimeSurfaceBound=true;
    //private XEGLConfigChooser xeglConfigChooser=null;
    // Populate with default parameters
    private XEGLConfigChooser.SurfaceParams mWantedSurfaceParams=new XEGLConfigChooser.SurfaceParams(0,0);

    public boolean DO_SUPERSYNC_MODS=false;

    //enum Message{START_RENDERING_FRAMES,STOP_RENDERING_FRAMES};
    //final BlockingQueue<Message> blockingQueue = new LinkedBlockingQueue<Message>();
    private final AtomicBoolean shouldRender=new AtomicBoolean(false);

    public XGLSurfaceView(final Context context){
        super(context);
        activity=((AppCompatActivity)context);
        ((AppCompatActivity)context).getLifecycle().addObserver(this);
        SurfaceHolder holder = getHolder();
        holder.addCallback(this);
    }
    public XGLSurfaceView(Context context, AttributeSet attrs) {
        super(context,attrs);
        activity=((AppCompatActivity)context);
        ((AppCompatActivity)context).getLifecycle().addObserver(this);
        SurfaceHolder holder = getHolder();
        holder.addCallback(this);
    }

    public void setEGLConfigPrams(final XEGLConfigChooser.SurfaceParams wantedSurfaceParams){
        this.mWantedSurfaceParams=wantedSurfaceParams;
    }

    public void setRenderer(final GLSurfaceView.Renderer renderer){
        this.mRenderer=renderer;
    }
    public void setRenderer(final Renderer2 renderer2){
        this.mRenderer2=renderer2;
    }

    /**
     * Create the OpenGL context, but not the EGL Surface since I have to wait for the
     * android.view.SurfaceHolder.Callback until the native window is available
     */
    @OnLifecycleEvent(Lifecycle.Event.ON_CREATE)
    private void onCreate() {
        log("onCreate");
        eglDisplay = EGL14.eglGetDisplay(EGL_DEFAULT_DISPLAY);
        int[] major = new int[]{0};
        int[] minor = new int[]{0};
        EGL14.eglInitialize(eglDisplay, major, 0, minor, 0);
        eglConfig = XEGLConfigChooser.chooseConfig(eglDisplay,mWantedSurfaceParams);
        final int[] contextAttributes = new int[]{
                EGL14.EGL_CONTEXT_CLIENT_VERSION, 2,
                EGL14.EGL_NONE
        };
        // https://www.khronos.org/registry/EGL/sdk/docs/man/html/eglCreateContext.xhtml
        eglContext = EGL14.eglCreateContext(eglDisplay, eglConfig, EGL_NO_CONTEXT, contextAttributes, 0);
        if (eglContext==EGL_NO_CONTEXT) {
            throw new AssertionError("Cannot create eglContext");
        }
    }

    private final Runnable mOpenGLRunnable=new Runnable() {
        @Override
        public void run() {
            eglMakeCurrentSafe(eglDisplay,eglSurface,eglContext);
            if(DO_SUPERSYNC_MODS){
                XEGLConfigChooser.setEglSurfaceAttrib(EGL14.EGL_RENDER_BUFFER,EGL14.EGL_SINGLE_BUFFER);
                XEGLConfigChooser.setEglSurfaceAttrib(EGL_ANDROID_front_buffer_auto_refresh,EGL14.EGL_TRUE);
                eglSwapBuffersSafe(eglDisplay,eglSurface);
                Thread.currentThread().setPriority(Thread.MAX_PRIORITY);
                Process.setThreadPriority(-20);
            }
            if(firstTimeSurfaceBound){
                if(mRenderer2!=null){
                    mRenderer2.onContextCreated(SURFACE_W,SURFACE_H);
                }
                firstTimeSurfaceBound=false;
            }
            if(mRenderer!=null){
                mRenderer.onSurfaceCreated(null,null);
                mRenderer.onSurfaceChanged(null,SURFACE_W,SURFACE_H);
            }
            while (!Thread.currentThread().isInterrupted()){
                if(mRenderer!=null){
                    mRenderer.onDrawFrame(null);
                }
                if(mRenderer2!=null){
                    mRenderer2.onDrawFrame();
                }
                if(!DO_SUPERSYNC_MODS){
                    eglSwapBuffersSafe(eglDisplay,eglSurface);
                }
            }
            eglMakeCurrentSafe(eglDisplay,EGL_NO_SURFACE,EGL_NO_CONTEXT);
        }
    };

    @OnLifecycleEvent(Lifecycle.Event.ON_RESUME)
    private void onResume(){
        log("onResume");
        // wait until the EGL Surface has been created
        // e.g wait until the SurfaceHolder callback is called
    }

    private static void eglSwapBuffersSafe(final EGLDisplay eglDisplay,final EGLSurface eglSurface){
        if(!EGL14.eglSwapBuffers(eglDisplay,eglSurface)){
            log("Cannot swap buffers");
        }
    }
    private static void eglMakeCurrentSafe(final EGLDisplay eglDisplay, EGLSurface eglSurface, EGLContext eglContext) {
        //log("makeCurrent");
        boolean result= EGL14.eglMakeCurrent(eglDisplay,eglSurface,eglSurface,eglContext);
        if(!result){
            throw new AssertionError("Cannot make surface current "+eglSurface);
        }
    }

    @OnLifecycleEvent(Lifecycle.Event.ON_PAUSE)
    private void onPause(){
        log("onPause");
        // We have to make sure that the OpenGL thread is not running or the context is not bound since
        // after onPause() the surface might have to be destroyed
        if(mOpenGLThread!=null){
            mOpenGLThread.interrupt();
            try {
                mOpenGLThread.join();
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }
    }

    @OnLifecycleEvent(Lifecycle.Event.ON_DESTROY)
    private void onDestroy(){
        log("onDestroy");
        EGL14.eglDestroyContext(eglDisplay, eglContext);
        EGL14.eglTerminate(eglDisplay);
    }


    @Override
    public void surfaceCreated(SurfaceHolder holder) {
        log("surfaceCreated");
    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
        SURFACE_W=width;
        SURFACE_H=height;
        log("surfaceChanged");
        if(eglSurface!=EGL_NO_SURFACE){
            throw new AssertionError("Changing Surface is not supported");
        }
        // We should never get the Surface before onCreate() is called
        if(!activity.getLifecycle().getCurrentState().isAtLeast(Lifecycle.State.CREATED)){
            throw new AssertionError("Got surface before onCreate()");
        }
        if(!activity.getLifecycle().getCurrentState().isAtLeast(Lifecycle.State.RESUMED)){
            throw new AssertionError("Got surface before onResume()");
        }
        eglSurface = EGL14.eglCreateWindowSurface(eglDisplay, eglConfig,holder.getSurface(),null,0);
        if(eglSurface==EGL_NO_SURFACE){
            throw new AssertionError("Cannot create window surface");
        }
        mOpenGLThread=new Thread(mOpenGLRunnable);
        mOpenGLThread.start();
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {
        log("surfaceDestroyed");
        // We should never have to destroy the surface before onPause() is called
        if(activity.getLifecycle().getCurrentState().isAtLeast(Lifecycle.State.RESUMED)){
            throw new AssertionError("Destroyed surface before onPause()");
        }
        EGL14.eglDestroySurface(eglDisplay, eglSurface);
        eglSurface=EGL_NO_SURFACE;
    }

    static void log(String message){
        Log.d("MyGLView",message);
    }


    public interface Renderer2{
        // Called as soon as the OpenGL context is created
        // The lifetime of the OpenGL context is tied to the lifetime of the Activity (onCreate / onDestroy)
        // Therefore this callback is called at most once
        // Also, since the https://www.khronos.org/registry/EGL/extensions/KHR/EGL_KHR_surfaceless_context.txt
        // extension is not available on all devices, a Surface is always bound when
        // onContextCreated is called
        void onContextCreated(int width,int height);
        // Called repeatedly in between onResume() / onPause()
        void onDrawFrame();
        // Called once the opengl context has to be destroyed,
        // but here the context is still bound for cleanup operations
        //void onContextDestroyed();
    }


}
