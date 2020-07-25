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

import constantin.renderingx.core.mglview.XEGLConfigChooser;
import constantin.renderingx.core.mglview.XGLSurfaceView;

import static constantin.renderingx.core.views.MyEGLConfigChooser.EGL_ANDROID_front_buffer_auto_refresh;

/**
 * Wrapper around GvrLayout/GLSurfaceView that includes its own rendering interface, created for SuperSync
 * Provides a convenient IRendererSuperSync
 * SuperSync allows developers to build their own Time Warp /other similar latency reducing techniques without
 * Specifying the device as 'Daydream ready'
 */

public class ViewSuperSync extends MyVRLayout implements XGLSurfaceView.Renderer2, Choreographer.FrameCallback{
    private static final String TAG="ViewSuperSync";
    private final XGLSurfaceView mGLSurfaceView;
    private IRendererSuperSync mRenderer;

    private final long choreographerVsyncOffsetNS;
    private Context context;

    public ViewSuperSync(Context context){
        super(context);
        this.context=context;
        //getUiLayout().setTransitionViewEnabled(false);
        //setAsyncReprojectionEnabled(false);
        mGLSurfaceView =new XGLSurfaceView(context);
        mGLSurfaceView.setEGLConfigChooser(new XEGLConfigChooser(true,0));
        mGLSurfaceView.setRenderer(this);
        mGLSurfaceView.DO_SUPERSYNC_MODS=true;

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
        Choreographer.getInstance().postFrameCallback(this);
    }

    @OnLifecycleEvent(Lifecycle.Event.ON_PAUSE)
    private void onPause(){
        Choreographer.getInstance().removeFrameCallback(this);
        pauseX();
        //FullscreenHelper.disableAndroidVRModeIfEnabled(this);
    }

    @OnLifecycleEvent(Lifecycle.Event.ON_DESTROY)
    private void onDestroy(){
        destroyX();
    }


    /*@Override
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
        debug("Exited SS on GL thread");
        doNotEnterAgain=true;
    }*/

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


    @Override
    public void onContextCreated(int width, int height) {
        mRenderer.onContextCreated(width,height);
    }

    @Override
    public void onDrawFrame() {
        mRenderer.onDrawFrame();
    }


    public interface IRendererSuperSync {
        void onContextCreated(int width, int height);
        void onDrawFrame();
        void setLastVSYNC(long lastVSYNC);
    }

}