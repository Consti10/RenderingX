package constantin.renderingx.core.views;

import android.content.Context;
import android.opengl.EGL14;
import android.opengl.GLES20;
import android.opengl.GLSurfaceView;
import android.os.Build;
import android.os.Process;
import android.util.Log;
import android.view.Choreographer;
import android.view.Display;
import android.view.WindowManager;

import androidx.lifecycle.Lifecycle;
import androidx.lifecycle.OnLifecycleEvent;

import java.util.Objects;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import static constantin.renderingx.core.views.MyEGLConfigChooser.EGL_ANDROID_front_buffer_auto_refresh;

/**
 * Wrapper around GvrLayout/GLSurfaceView that includes its own rendering interface, created for SuperSync
 * Provides a convenient IRendererSuperSync
 * SuperSync allows developers to build their own Time Warp /other similar latency reducing techniques without
 * Specifying the device as 'Daydream ready'
 */

public class ViewSuperSync extends MyVRLayout implements GLSurfaceView.Renderer, Choreographer.FrameCallback{
    private static final String TAG="ViewSuperSync";
    private final GLSurfaceView mGLSurfaceView;
    private IRendererSuperSync mRenderer;

    private float color=1.0f;
    private boolean doNotEnterAgain=true;
    private int frameC=0;

    private long choreographerVsyncOffsetNS;
    private Context context;

    public ViewSuperSync(Context context){
        super(context);
        this.context=context;
        //getUiLayout().setTransitionViewEnabled(false);
        //setAsyncReprojectionEnabled(false);
        mGLSurfaceView =new GLSurfaceView(context);
        mGLSurfaceView.setEGLContextClientVersion(2);
        mGLSurfaceView.setEGLConfigChooser(new MyEGLConfigChooser(true,0));
        mGLSurfaceView.setRenderer(this);
        mGLSurfaceView.setRenderMode(GLSurfaceView.RENDERMODE_CONTINUOUSLY);
        mGLSurfaceView.setPreserveEGLContextOnPause(false);
        setPresentationView(mGLSurfaceView);
        //
        final Display d=((WindowManager) Objects.requireNonNull(context.getSystemService(Context.WINDOW_SERVICE))).getDefaultDisplay();
        choreographerVsyncOffsetNS=d.getAppVsyncOffsetNanos();
        //System.out.println("refreshRate:"+d.getRefreshRate());
        //System.out.println("app Vsync offset"+choreographerVsyncOffsetNS/1000000.0f);
    }

    public void setRenderer(final IRendererSuperSync mRenderer){
        this.mRenderer=mRenderer;
    }


    @OnLifecycleEvent(Lifecycle.Event.ON_RESUME)
    private void onResume(){
        //FullscreenHelper.setImmersiveSticky(this);
        //FullscreenHelper.enableAndroidVRModeIfPossible(this);
        resumeX();
        mGLSurfaceView.onResume();
        mGLSurfaceView.queueEvent(new Runnable() {
            @Override
            public void run() {
                color=1.0f;
                frameC=0;
                doNotEnterAgain=false;
                debug("Setting frameC and doNotEnterAgain");
            }
        });
        Choreographer.getInstance().postFrameCallback(this);
    }

    @OnLifecycleEvent(Lifecycle.Event.ON_PAUSE)
    private void onPause(){
        mRenderer.requestExitSuperSyncLoop();
        Choreographer.getInstance().removeFrameCallback(this);
        mGLSurfaceView.onPause();
        pauseX();
        //FullscreenHelper.disableAndroidVRModeIfEnabled(this);
    }

    @OnLifecycleEvent(Lifecycle.Event.ON_DESTROY)
    private void onDestroy(){
        destroyX();
    }


    @Override
    public void onSurfaceCreated(GL10 gl10, EGLConfig eglConfig) {
        mRenderer.onSurfaceCreated();
    }

    @Override
    public void onSurfaceChanged(GL10 gl10, int i, int i1) {
        mRenderer.onSurfaceChanged(i,i1);
        MyEGLConfigChooser.setEglSurfaceAttrib(EGL14.EGL_RENDER_BUFFER,EGL14.EGL_SINGLE_BUFFER);
        MyEGLConfigChooser.setEglSurfaceAttrib(EGL_ANDROID_front_buffer_auto_refresh,EGL14.EGL_TRUE);
    }

    @Override
    public void onDrawFrame(GL10 gl10) {
        if(frameC<60){
            frameC++;
            //Clear and swap
            GLES20.glClearColor(0,color,color,1);
            color-=(1/60.0f);
            GLES20.glClear(GLES20.GL_DEPTH_BUFFER_BIT|GLES20.GL_COLOR_BUFFER_BIT|GLES20.GL_STENCIL_BUFFER_BIT);
            try {
                Thread.sleep(4);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
            EGL14.eglSwapBuffers(EGL14.eglGetCurrentDisplay(),EGL14.eglGetCurrentSurface(EGL14.EGL_DRAW));
            return;
        }
        if(doNotEnterAgain){
            //debug("Do not enter again flag set");
            return;
        }
        debug("Entering SS on GL thread");
        GLES20.glClearColor(0,0,0,0.0f);
        Thread.currentThread().setPriority(Thread.MAX_PRIORITY);
        Process.setThreadPriority(-20);

        mRenderer.enterSuperSyncLoop(getExclusiveVRCore());
        /*try {
            Thread.sleep(1000*1000*1000);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }*/
        debug("Exited SS on GL thread");
        doNotEnterAgain=true;
    }

    @Override
    public void doFrame(long frameTimeNanos) {
        //System.out.println("j time "+System.nanoTime());
        //System.out.println("j delta"+(System.nanoTime()-frameTimeNanos));
        //####google SurfaceFlinger.cpp ##################
        // We add an additional 1ms to allow for processing time and
        // differences between the ideal and actual refresh rate.
        /*final Display d=((WindowManager) Objects.requireNonNull(context.getSystemService(Context.WINDOW_SERVICE))).getDefaultDisplay();
        if(d.getAppVsyncOffsetNanos()!=choreographerVsyncOffsetNS){
            choreographerVsyncOffsetNS=d.getAppVsyncOffsetNanos();
            System.out.println("choreographerVsyncOffsetNS changed");
        }
        choreographerVsyncOffsetNS=d.getAppVsyncOffsetNanos();*/


        mRenderer.setLastVSYNC(frameTimeNanos-choreographerVsyncOffsetNS+1000000);
        Choreographer.getInstance().postFrameCallback(this);
    }

    private static int getExclusiveVRCore(){
        int exclusiveVRCore=2; //use the 3rd core default
        if (Build.VERSION.SDK_INT >= 24) {
            try {
                int[] cores= Process.getExclusiveCores();
                if(cores!=null){
                    if(cores.length>=1){
                        exclusiveVRCore=cores[0];
                    }
                }
            } catch (RuntimeException e){
                Log.w("", "getExclusiveCores() is not supported on this device.");
            }
        }
        return exclusiveVRCore;
    }


    private static void debug(final String s){
        Log.d(TAG,s);
    }


    public interface IRendererSuperSync {

        void onSurfaceCreated();
        void onSurfaceChanged(final int width, final int height);
        void enterSuperSyncLoop(final int exclusiveVRCore);
        void requestExitSuperSyncLoop();
        void setLastVSYNC(long lastVSYNC);
    }

}