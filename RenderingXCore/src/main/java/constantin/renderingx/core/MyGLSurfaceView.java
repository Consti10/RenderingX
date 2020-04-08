package constantin.renderingx.core;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.content.Context;
import android.opengl.GLSurfaceView;
import android.util.AttributeSet;

import androidx.appcompat.app.AppCompatActivity;
import androidx.lifecycle.Lifecycle;
import androidx.lifecycle.LifecycleObserver;
import androidx.lifecycle.LifecycleOwner;
import androidx.lifecycle.OnLifecycleEvent;

/**
 * same as android GLSurfaceView with 1 improvement:
 * Uses Android lifecycle to handle onPause / onResume()
 * It is easy to forget to call the GLSurfaceView's onPause() / onResume() functions
 * It also reduces code complexity of the implementation(s) significantly !
 */

//No default constructor because we explicitly need the LifecycleOwner !
public class MyGLSurfaceView extends GLSurfaceView implements LifecycleObserver {
    //Do not pause / resume if no renderer is set (e.g. the view exists but was not properly configured in onCreate() )
    private boolean rendererSet=false;

    public MyGLSurfaceView(Context context) {
        super(context);
        ((AppCompatActivity)context).getLifecycle().addObserver(this);
    }

    public MyGLSurfaceView(Context context, AttributeSet attrs) {
        super(context,attrs);
        ((AppCompatActivity)context).getLifecycle().addObserver(this);
    }

    public MyGLSurfaceView(Context context,LifecycleOwner lifecycleOwner) {
        super(context);
        lifecycleOwner.getLifecycle().addObserver(this);
    }

    @Override
    public void setRenderer(Renderer renderer){
        rendererSet=true;
        super.setRenderer(renderer);
    }

    @OnLifecycleEvent(Lifecycle.Event.ON_RESUME)
    private void resume(){
        if(!rendererSet)return;
        super.onResume();
    }

    @OnLifecycleEvent(Lifecycle.Event.ON_PAUSE)
    private void pause(){
        if(!rendererSet)return;
        super.onPause();
    }

    //onPause() / onResume is called automatically, do not call externally !
    @Override
    public void onResume() {
        throw new IllegalStateException("Illegal access to onResume");
    }
    @Override
    public void onPause() {
        throw new IllegalStateException("Illegal access to oPause");
    }

}
