package constantin.renderingx.core.xglview;

import android.content.Context;
import android.opengl.EGL14;
import android.opengl.EGLConfig;
import android.opengl.EGLContext;
import android.opengl.EGLDisplay;
import android.opengl.EGLSurface;
import android.opengl.GLSurfaceView;
import android.os.Build;
import android.os.Process;
import android.util.AttributeSet;
import android.util.Log;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

import androidx.annotation.Nullable;
import androidx.annotation.RequiresApi;
import androidx.appcompat.app.AppCompatActivity;
import androidx.lifecycle.Lifecycle;
import androidx.lifecycle.LifecycleObserver;
import androidx.lifecycle.OnLifecycleEvent;

import com.google.common.primitives.Ints;

import java.util.ArrayList;

import javax.microedition.khronos.opengles.GL10;

import constantin.renderingx.core.Extensions;
import constantin.renderingx.core.views.VRLayout;
import constantin.video.core.gl.ISurfaceTextureAvailable;

import static android.opengl.EGL14.EGL_DEFAULT_DISPLAY;
import static android.opengl.EGL14.EGL_NO_CONTEXT;
import static android.opengl.EGL14.EGL_NO_DISPLAY;
import static android.opengl.EGL14.EGL_NO_SURFACE;
import static android.opengl.EGLExt.EGL_CONTEXT_FLAGS_KHR;
import static constantin.renderingx.core.xglview.EGLContextPriority.EGL_CONTEXT_PRIORITY_HIGH_IMG;
import static constantin.renderingx.core.xglview.EGLContextPriority.EGL_CONTEXT_PRIORITY_LEVEL_IMG;
import static constantin.renderingx.core.xglview.XEGLConfigChooser.EGL_ANDROID_front_buffer_auto_refresh;

// TODO in Development

/**
 * This View is intended as an replacement for {@link android.opengl.GLSurfaceView}
 * https://developer.android.com/reference/android/opengl/GLSurfaceView
 * The Complexity of GLSurfaceView comes from its compatibility all the way down to Android 2.3.3 (API level 10) where for example EGL14 was not available
 * By replacing EGL10 with EGL14 and also having not to worry about 'hacks' that were needed on these old api versions I hope to reduce complexity
 * It is also going to be easier to add more specialized features,for example multiple OpenGL context(s) aka 'Shared context'
 * Most notable differences to GLSurfaceView:
 * 1) While rendering frames is obviously decoupled from the UI thread, creating the egl context and surface is done on the UI thread
 * * *to reduce inter-thread synchronization. For example, the egl surface has to be destroyed on the UI thread but cannot be destroyed
 * * *until the GL thread is guaranteed to not use the surface anymore
 * 2) The Callbacks have a slightly different naming. By having an onContextCreated callback it is easier to create OpenGL objects that are not affected by the
 * * *surface width/height only once
 * 3) The OpenGL context is preserved between onPause()/onResume().There are no devices anymore that only support one concurrent OpenGL context
 */
@RequiresApi(Build.VERSION_CODES.LOLLIPOP)
public class XGLSurfaceView extends SurfaceView implements LifecycleObserver, SurfaceHolder.Callback {
    private static final String TAG="XGLSurfaceView";
    // Reference to the base activity for obtaining lifecycle status and more
    private final AppCompatActivity activity;
    // All these members are created/ destroyed on the UI thread
    // EGL_NO_DISPLAY is the same as null
    private EGLDisplay eglDisplay = EGL_NO_DISPLAY;
    private EGLSurface eglSurface = EGL_NO_SURFACE;
    private EGLContext eglContext = EGL_NO_CONTEXT;
    private EGLConfig eglConfig = null;
    // The Thread that renders OpenGL frames
    private Thread mOpenGLThread;
    // Choose between one of the interfaces.
    // For compability with code that uses the GLSurfaceView.Renderer an interface really similar to
    // this interface also exists
    private LegacyRenderer mRenderer0;
    private FullscreenRenderer mRenderer1;
    private FullscreenRendererWithSurfaceTexture mRenderer2;
    // For VR applications the surface width and height is equal to the display w/h and therefore
    // Does never change
    private int SURFACE_W,SURFACE_H;
    // Keep track if we should call the onContextCreated callback
    private boolean firstTimeSurfaceBound=true;
    // Use setEGLConfigPrams to customize the created surface.
    private XSurfaceParams mWantedSurfaceParams=new XSurfaceParams(0,0);

    public boolean DO_SUPERSYNC_MODS=false;
    // When enabled you can use the EGL_KHR_DEBUG extension (no checkGLError yeah !)
    public boolean ENABLE_EGL_KHR_DEBUG=false;
    // When enabled try to get a high priority context
    public boolean ENABLE_HIGH_PRIORITY_CONTEXT=false;

    private GLContextSurfaceLess glContextSurfaceLess=null;

    private SurfaceTextureHolder surfaceTextureHolder;
    private int exclusiveCPUCore=-1;
    public static final int EGL_CONTEXT_OPENGL_DEBUG_BIT_KHR= 0x00000001;

    public XGLSurfaceView(final Context context){
        super(context);
        activity=((AppCompatActivity)context);
        ((AppCompatActivity)context).getLifecycle().addObserver(this);
        getHolder().addCallback(this);
    }
    public XGLSurfaceView(Context context, AttributeSet attrs) {
        super(context,attrs);
        activity=((AppCompatActivity)context);
        ((AppCompatActivity)context).getLifecycle().addObserver(this);
        getHolder().addCallback(this);
    }

    public void setEGLConfigPrams(final XSurfaceParams wantedSurfaceParams){
        this.mWantedSurfaceParams=wantedSurfaceParams;
    }

    public void setRenderer(final LegacyRenderer renderer){
        mRenderer0 =renderer;
    }
    public void setRenderer(final FullscreenRenderer renderer){
        this.mRenderer1 =renderer;
    }
    /**
     * Set the renderer that also has a SurfaceTexture,e.g for video playback
     * @param iSurfaceTextureAvailable Configure your video player using this interface. When passing null for the ISurfaceTextureAvailable
     * No SurfaceTexture holder is created (and the value passed to the renderer is also null)
     */
    public void setRenderer(final FullscreenRendererWithSurfaceTexture renderer,final @Nullable ISurfaceTextureAvailable iSurfaceTextureAvailable){
        this.mRenderer2 =renderer;
        if(iSurfaceTextureAvailable!=null){
            this.surfaceTextureHolder=new SurfaceTextureHolder(activity,iSurfaceTextureAvailable);
        }
    }

    public void setmISecondaryContext(final GLContextSurfaceLess.SecondarySharedContext i){
        glContextSurfaceLess=new GLContextSurfaceLess(i);
    }

    private final Runnable mOpenGLRunnable=new Runnable() {
        @Override
        public void run() {
            Thread.currentThread().setName("XGLRendererM");
            eglMakeCurrentSafe(eglDisplay,eglSurface,eglContext);
            if(DO_SUPERSYNC_MODS){
                XEGLConfigChooser.setEglSurfaceAttrib(EGL14.EGL_RENDER_BUFFER,EGL14.EGL_SINGLE_BUFFER);
                XEGLConfigChooser.setEglSurfaceAttrib(EGL_ANDROID_front_buffer_auto_refresh,EGL14.EGL_TRUE);
                eglSwapBuffersSafe(eglDisplay,eglSurface);
                Thread.currentThread().setPriority(Thread.MAX_PRIORITY);
                Process.setThreadPriority(-20);
                if(exclusiveCPUCore!=-1){
                    Extensions.nativeSetThreadAffinity(exclusiveCPUCore);
                }
            }
            if(firstTimeSurfaceBound){
                if(surfaceTextureHolder!=null){
                    surfaceTextureHolder.createOnOpenGLThread();
                }
                if(mRenderer1 !=null){
                    mRenderer1.onContextCreated(SURFACE_W,SURFACE_H);
                }
                if(mRenderer2 !=null){
                    mRenderer2.onContextCreated(SURFACE_W,SURFACE_H,surfaceTextureHolder);
                }
                firstTimeSurfaceBound=false;
            }
            if(mRenderer0 !=null){
                mRenderer0.onSurfaceCreated();
                mRenderer0.onSurfaceChanged(SURFACE_W,SURFACE_H);
            }
            if(glContextSurfaceLess!=null){
                glContextSurfaceLess.resumeWork();
            }
            while (!Thread.currentThread().isInterrupted()){
                if(mRenderer0 !=null){
                    mRenderer0.onDrawFrame();
                }
                if(mRenderer1 !=null){
                    mRenderer1.onDrawFrame();
                }
                if(mRenderer2 !=null){
                    mRenderer2.onDrawFrame();
                }
                // Swap buffers will ensure that onDrawFrame is not called more than 60 times per second
                // If the surface is NOT single buffered
                if(!DO_SUPERSYNC_MODS){
                    eglSwapBuffersSafe(eglDisplay,eglSurface);
                }
            }
            eglMakeCurrentSafe(eglDisplay,EGL_NO_SURFACE,EGL_NO_CONTEXT);
        }
    };

    /**
     * Create the OpenGL context, but not the EGL Surface since I have to wait for the
     * android.view.SurfaceHolder.Callback until the native window is available
     */
    @OnLifecycleEvent(Lifecycle.Event.ON_CREATE)
    private void onCreate() {
        log("onCreate");
        if(mRenderer0==null && mRenderer1 ==null && mRenderer2 ==null){
            throw new AssertionError("No renderer set");
        }
        if(glContextSurfaceLess!=null){
            glContextSurfaceLess.create();
        }
        eglDisplay = EGL14.eglGetDisplay(EGL_DEFAULT_DISPLAY);
        int[] major = new int[]{0};
        int[] minor = new int[]{0};
        EGL14.eglInitialize(eglDisplay, major, 0, minor, 0);
        eglConfig = XEGLConfigChooser.chooseConfig(eglDisplay,mWantedSurfaceParams);
        final int GLESVersion=mWantedSurfaceParams.mUseMutableFlag ? 3 : 2;

        final ArrayList<Integer> contextAttributes=new ArrayList<>();
        contextAttributes.add(EGL14.EGL_CONTEXT_CLIENT_VERSION);
        contextAttributes.add(GLESVersion);
        if(ENABLE_EGL_KHR_DEBUG){
            contextAttributes.add(EGL_CONTEXT_FLAGS_KHR);
            contextAttributes.add(EGL_CONTEXT_OPENGL_DEBUG_BIT_KHR);
        }
        if(ENABLE_HIGH_PRIORITY_CONTEXT){
            contextAttributes.add(EGL_CONTEXT_PRIORITY_LEVEL_IMG);
            contextAttributes.add(EGL_CONTEXT_PRIORITY_HIGH_IMG);
        }
        contextAttributes.add( EGL14.EGL_NONE);

        // https://www.khronos.org/registry/EGL/sdk/docs/man/html/eglCreateContext.xhtml
        eglContext = EGL14.eglCreateContext(eglDisplay, eglConfig, glContextSurfaceLess==null ? EGL_NO_CONTEXT : glContextSurfaceLess.getEglContext(),
                Ints.toArray(contextAttributes), 0);
        if (eglContext==EGL_NO_CONTEXT) {
            throw new AssertionError("Cannot create eglContext");
        }
        // check if we actually got a high priority context
        if(ENABLE_HIGH_PRIORITY_CONTEXT){
            final int eglContextPriorityLevel=eglQueryContext(EGL_CONTEXT_PRIORITY_LEVEL_IMG);
            if(eglContextPriorityLevel==EGL_CONTEXT_PRIORITY_HIGH_IMG){
                Log.d(TAG,"Got high prio context"+eglContextPriorityLevel);
            }else{
                Log.d(TAG,"Cannot get high prio context"+eglContextPriorityLevel);
            }
        }
        //if(ENABLE_EGL_KHR_DEBUG){
        //}
    }

    @OnLifecycleEvent(Lifecycle.Event.ON_RESUME)
    private void onResume(){
        log("onResume");
        // If supported, use the exclusive core from the android os for the OpenGL thread
        exclusiveCPUCore= VRLayout.getExclusiveVRCore();
        // wait until the EGL Surface has been created
        // e.g wait until the SurfaceHolder callback is called
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
        if(glContextSurfaceLess!=null){
            glContextSurfaceLess.pauseWork();
        }
    }

    @OnLifecycleEvent(Lifecycle.Event.ON_DESTROY)
    private void onDestroy(){
        log("onDestroy");
        if(surfaceTextureHolder!=null){
            surfaceTextureHolder.destroyOnUiThread();
        }
        if(glContextSurfaceLess!=null){
            glContextSurfaceLess.destroy();
        }
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

    /**
     * @see android.opengl.GLSurfaceView.Renderer
     */
    public interface LegacyRenderer{
        void onSurfaceCreated();
        void onSurfaceChanged(int width, int height);
        void onDrawFrame();
    }

    public interface FullscreenRenderer {
        // Called as soon as the OpenGL context is created
        // The lifetime of the OpenGL context is tied to the lifetime of the Activity (onCreate / onDestroy)
        // Therefore this callback is called at most once
        // Also, since the https://www.khronos.org/registry/EGL/extensions/KHR/EGL_KHR_surfaceless_context.txt
        // extension is not available on all devices, a Surface is always bound when
        // onContextCreated is called
        // For a VR application, screen width and height do not change and are equal to the screen width and height
        void onContextCreated(int screenWidth,int screenHeight);
        // Called repeatedly in between onResume() / onPause()
        void onDrawFrame();
        // Called once the opengl context has to be destroyed,
        // but here the context is still bound for cleanup operations
        //void onContextDestroyed();
    }

    // Same as above but the GLSurfaceView also creates and manages a SurfaceTexture aka VideoTexture
    public interface FullscreenRendererWithSurfaceTexture {
        void onContextCreated(int screenWidth,int screenHeight,final SurfaceTextureHolder surfaceTextureHolder);
        void onDrawFrame();
    }
    private static void eglSwapBuffersSafe(final EGLDisplay eglDisplay,final EGLSurface eglSurface){
        if(!EGL14.eglSwapBuffers(eglDisplay,eglSurface)){
            log("Cannot swap buffers");
        }
    }
    private static void eglMakeCurrentSafe(final EGLDisplay eglDisplay, EGLSurface eglSurface,EGLContext eglContext) {
        //log("makeCurrent");
        boolean result= EGL14.eglMakeCurrent(eglDisplay,eglSurface,eglSurface,eglContext);
        if(!result){
            throw new AssertionError("Cannot make surface current "+eglSurface);
        }
    }
    private int eglQueryContext(int attribute){
        int[] ret=new int[1];
        EGL14.eglQueryContext(eglDisplay,eglContext,attribute,ret,0);
        return ret[0];
    }
}
