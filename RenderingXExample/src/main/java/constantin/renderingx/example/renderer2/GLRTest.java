package constantin.renderingx.example.renderer2;

import android.content.Context;
import android.opengl.GLSurfaceView;

import com.google.vr.ndk.base.GvrApi;
import com.google.vr.sdk.base.GvrView;
import com.google.vr.sdk.base.GvrViewerParams;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;


public class GLRTest implements GLSurfaceView.Renderer{
    static {
        System.loadLibrary("example-renderer2");
    }
    private native long nativeConstruct(Context context,long nativeGvrContext,boolean RENDER_SCENE_USING_GVR_RENDERBUFFER,
                                        boolean RENDER_SCENE_USING_VERTEX_DISPLACEMENT,boolean MESH,boolean SPHERE,boolean SPHERE_2);
    private native void nativeDelete(long p);
    private native void nativeOnSurfaceCreated(long p,final Context context);
    private native void nativeOnSurfaceChanged(long p,int width,int height);
    private native void nativeOnDrawFrame(long p);
    private native void nativeUpdateHeadsetParams(long nativePointer,float screen_width_meters,
                                                  float screen_height_meters,
                                                  float screen_to_lens_distance,
                                                  float inter_lens_distance,
                                                  int vertical_alignment,
                                                  float tray_to_lens_distance,
                                                  float[] device_fov_left,
                                                  float[] radial_distortion_params,
                                                  int screenWidthP,int screenHeightP);

    private final Context mContext;
    private final long nativeRenderer;

    public GLRTest(final Context context, final GvrApi gvrApi,boolean RENDER_SCENE_USING_GVR_RENDERBUFFER,
                   boolean RENDER_SCENE_USING_VERTEX_DISPLACEMENT,boolean MESH,boolean SPHERE,boolean SPHERE2){
        mContext=context;
        GvrView view=new GvrView(context);
        final GvrViewerParams params=view.getGvrViewerParams();

        nativeRenderer=nativeConstruct(context,
                gvrApi.getNativeGvrContext(),RENDER_SCENE_USING_GVR_RENDERBUFFER,RENDER_SCENE_USING_VERTEX_DISPLACEMENT,MESH,SPHERE,SPHERE2);

        float[] fov=new float[4];
        fov[0]=params.getLeftEyeMaxFov().getLeft();
        fov[1]=params.getLeftEyeMaxFov().getRight();
        fov[2]=params.getLeftEyeMaxFov().getBottom();
        fov[3]=params.getLeftEyeMaxFov().getTop();
        float[] kN=params.getDistortion().getCoefficients();
        nativeUpdateHeadsetParams(nativeRenderer,view.getScreenParams().getWidthMeters(),view.getScreenParams().getHeightMeters(),
                params.getScreenToLensDistance(),params.getInterLensDistance(),params.getVerticalAlignment().ordinal(),params.getVerticalDistanceToLensCenter(),
                fov,kN,view.getScreenParams().getWidth(),view.getScreenParams().getHeight());

    }

    @Override
    public void onSurfaceCreated(GL10 gl, EGLConfig config) {
        nativeOnSurfaceCreated(nativeRenderer,mContext);
    }

    @Override
    public void onSurfaceChanged(GL10 gl, int width, int height) {
        nativeOnSurfaceChanged(nativeRenderer,width,height);
    }

    @Override
    public void onDrawFrame(GL10 gl) {
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
