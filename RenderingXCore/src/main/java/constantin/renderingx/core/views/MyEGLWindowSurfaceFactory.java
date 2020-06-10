package constantin.renderingx.core.views;

import android.opengl.EGL14;
import android.opengl.GLSurfaceView;

import javax.microedition.khronos.egl.EGL10;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.egl.EGLDisplay;
import javax.microedition.khronos.egl.EGLSurface;

//Allows setting a single buffer flag in a 'best effort' manner

public class MyEGLWindowSurfaceFactory implements GLSurfaceView.EGLWindowSurfaceFactory{

    private final boolean mSingleBuffer;

    public MyEGLWindowSurfaceFactory(){
        mSingleBuffer=false;
    }

    public MyEGLWindowSurfaceFactory(final boolean singleBuffer){
        this.mSingleBuffer =singleBuffer;
    }

    @Override
    public EGLSurface createWindowSurface(EGL10 egl, EGLDisplay display, EGLConfig config, Object nativeWindow) {
        final int[] surfAttrSingleBuffer = {
                EGL14.EGL_RENDER_BUFFER, EGL14.EGL_SINGLE_BUFFER ,
                EGL14.EGL_NONE
        };
        if(mSingleBuffer){
            EGLSurface surf=egl.eglCreateWindowSurface(display,config,nativeWindow, surfAttrSingleBuffer);
            if(surf==null){
                System.out.println("SBS surface not supported");
                return egl.eglCreateWindowSurface(display,config,nativeWindow, null);
            }else{
                return surf;
            }
        }else{
            return egl.eglCreateWindowSurface(display,config,nativeWindow, null);
        }
    }

    @Override
    public void destroySurface(EGL10 egl, EGLDisplay display, EGLSurface surface) {
        egl.eglDestroySurface(display,surface);
    }
}
