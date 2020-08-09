package constantin.renderingx.core;

import android.content.Context;
import android.view.Choreographer;
import android.view.Display;
import android.view.WindowManager;

import androidx.appcompat.app.AppCompatActivity;
import androidx.lifecycle.Lifecycle;
import androidx.lifecycle.LifecycleObserver;
import androidx.lifecycle.OnLifecycleEvent;

import java.util.Objects;


public class VSYNC implements LifecycleObserver, Choreographer.FrameCallback{
    private native long nativeConstruct();
    private native void nativeSetVSYNCSentByChoreographer(long nativeInstance,long newVSYNC);
    //private native void nativePause(long nativeInstance);
    //private native void nativeResume(long nativeInstance);
    private native void nativeDelete(long p);

    private final long nativeInstance;

    private final long choreographerVsyncOffsetNS;

    public VSYNC(final AppCompatActivity parent){
        final Display d=((WindowManager) Objects.requireNonNull(parent.getSystemService(Context.WINDOW_SERVICE))).getDefaultDisplay();
        choreographerVsyncOffsetNS=d.getAppVsyncOffsetNanos();
        nativeInstance=nativeConstruct();
        parent.getLifecycle().addObserver(this);
    }
    public long getNativeInstance(){
        return nativeInstance;
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
        nativeSetVSYNCSentByChoreographer(nativeInstance,frameTimeNanos-choreographerVsyncOffsetNS+1000000);
        Choreographer.getInstance().postFrameCallback(this);
    }

    @OnLifecycleEvent(Lifecycle.Event.ON_RESUME)
    private void onResume(){
        Choreographer.getInstance().postFrameCallback(this);
        //nativeResume(nativeInstance);
    }

    @OnLifecycleEvent(Lifecycle.Event.ON_PAUSE)
    private void onPause(){
        Choreographer.getInstance().removeFrameCallback(this);
        //nativePause(nativeInstance);
    }

    @Override
    protected void finalize() throws Throwable {
        try {
            nativeDelete(nativeInstance);
        } finally {
            super.finalize();
        }
    }
}
