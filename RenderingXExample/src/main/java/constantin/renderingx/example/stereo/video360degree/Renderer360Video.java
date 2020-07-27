package constantin.renderingx.example.stereo.video360degree;

import android.annotation.SuppressLint;
import android.content.Context;

import androidx.appcompat.app.AppCompatActivity;

import com.google.vr.ndk.base.GvrApi;

import constantin.renderingx.core.MVrHeadsetParams;
import constantin.renderingx.core.STHelper;
import constantin.renderingx.core.xglview.XGLSurfaceView;
import constantin.video.core.gl.ISurfaceAvailable;
import constantin.video.core.gl.VideoSurfaceHolder;


//See native code for documentation

public class Renderer360Video implements /*GLSurfaceView.Renderer*/XGLSurfaceView.Renderer2 {
    static {
        System.loadLibrary("example-2");
    }
    private native long nativeConstruct(Context context,long nativeGvrContext,boolean RENDER_SCENE_USING_GVR_RENDERBUFFER,
                                        boolean RENDER_SCENE_USING_VERTEX_DISPLACEMENT,int SPHERE_MODE);
    private native void nativeDelete(long p);
    private native void nativeOnSurfaceCreated(long p,final Context context,int videoTexture);
    private native void nativeOnDrawFrame(long p);
    private native void nativeUpdateHeadsetParams(long nativePointer,final MVrHeadsetParams p);

    private final Context mContext;
    private final long nativeRenderer;
    private final VideoSurfaceHolder mVideoSurfaceHolder;
    private int nSurfaceTextureUpdateReturnedTrue=0;

    @SuppressLint("ApplySharedPref")
    public Renderer360Video(final AppCompatActivity context, final ISurfaceAvailable iSurfaceAvailable, final GvrApi gvrApi, boolean RENDER_SCENE_USING_GVR_RENDERBUFFER,
                            boolean RENDER_SCENE_USING_VERTEX_DISPLACEMENT, int SPHERE_MODE){
        mContext=context;
        mVideoSurfaceHolder=new VideoSurfaceHolder(context);
        mVideoSurfaceHolder.setCallBack(iSurfaceAvailable);
        nativeRenderer=nativeConstruct(context,
                gvrApi.getNativeGvrContext(),RENDER_SCENE_USING_GVR_RENDERBUFFER,RENDER_SCENE_USING_VERTEX_DISPLACEMENT,SPHERE_MODE);
        final MVrHeadsetParams params=new MVrHeadsetParams(context);
        nativeUpdateHeadsetParams(nativeRenderer,params);
    }
    // For testing the 'check if update was successfully'
    public int getNSurfaceTextureUpdateReturnedTrue(){
        return nSurfaceTextureUpdateReturnedTrue;
    }

    @Override
    public void onContextCreated(int width,int height) {
        mVideoSurfaceHolder.createSurfaceTextureGL();
        nativeOnSurfaceCreated(nativeRenderer,mContext,mVideoSurfaceHolder.getTextureId());
    }

    @Override
    public void onDrawFrame() {
        if(STHelper.updateAndCheck(mVideoSurfaceHolder.getSurfaceTexture())){
            nSurfaceTextureUpdateReturnedTrue++;
        }
        nativeOnDrawFrame(nativeRenderer);
    }

    @Override
    protected void finalize() throws Throwable {
        try {
            nativeDelete(nativeRenderer);
        } finally {
            super.finalize();
        }
    }
}
