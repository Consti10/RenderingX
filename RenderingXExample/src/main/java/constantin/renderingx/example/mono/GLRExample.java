package constantin.renderingx.example.mono;

import android.content.Context;
import android.opengl.GLSurfaceView;

import com.dinuscxj.gesture.MultiTouchGestureDetector;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import constantin.renderingx.core.mglview.XGLSurfaceView;


//See native code for documentation
public class GLRExample implements GLSurfaceView.Renderer, MultiTouchGestureDetector.OnMultiTouchGestureListener{
    static {
        System.loadLibrary("example-mono");
    }
    private native void nativeOnSurfaceCreated(final Context context);
    private native void nativeOnSurfaceChanged(int width,int height);
    private native void nativeOnDrawFrame();
    private native void nativeScale(float scale);
    private native void nativeMove(float moveX,float moveY);
    private native void nativeSetRenderingMode(int renderingMode);
    private native void nativeSetSeekBarValues(float val1,float val2,float val3);

    private final Context mContext;
    //Handles the user input
    MultiTouchGestureDetector mMultiTouchGestureDetector;
    private int width=1920,height=1080;


    GLRExample(final Context context){
        mContext=context;
        mMultiTouchGestureDetector=new MultiTouchGestureDetector(mContext,this);
    }

    @Override
    public void onSurfaceCreated(GL10 gl, EGLConfig config) {
        nativeOnSurfaceCreated(mContext);
    }

    @Override
    public void onSurfaceChanged(GL10 gl, int width, int height) {
        this.width=width;
        this.height=height;
        nativeOnSurfaceChanged(width,height);
    }

    @Override
    public void onDrawFrame(GL10 gl) {
        nativeOnDrawFrame();
    }

    void setSelectedMode(int mode){
        nativeSetRenderingMode(mode);
    }

    void setSeekBarValues(float val1, float val2, float val3){
        nativeSetSeekBarValues(val1,val2,val3);
    }

    @Override
    public void onScale(MultiTouchGestureDetector detector) {
        nativeScale(detector.getScale());
        /*distance *=detector.getScale();
        if(distance>19)distance=19;
        if(distance<1)distance=1f;*/
        //System.out.println("Scale:"+detector.getScale()+" Distance:"+ distance);
    }

    @Override
    public void onMove(MultiTouchGestureDetector detector) {
        float moveX=(float)detector.getMoveX()/width;
        float moveY=(float)detector.getMoveY()/height;
        nativeMove(moveX,moveY);
        //System.out.println("MoveX:"+moveX+" MoveY:"+moveY);
    }

    @Override
    public void onRotate(MultiTouchGestureDetector detector) {
        //rotZ+=detector.getRotation();
        //System.out.println("Rotation:"+rotZ);
    }

    @Override
    public boolean onBegin(MultiTouchGestureDetector detector) {
        return true;
    }

    @Override
    public void onEnd(MultiTouchGestureDetector detector) { }


}
