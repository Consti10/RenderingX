package constantin.renderingx.example.Mono;

import android.content.Context;
import android.opengl.GLSurfaceView;

import com.dinuscxj.gesture.MultiTouchGestureDetector;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;


//See native code for documentation
public class GLRExample implements GLSurfaceView.Renderer, MultiTouchGestureDetector.OnMultiTouchGestureListener{
    static {
        System.loadLibrary("example-mono");
    }
    private native void nativeOnSurfaceCreated(final Context context);
    private native void nativeOnSurfaceChanged(int width,int height);
    private native void nativeOnDrawFrame(int renderingMode);
    private native void nativeMoveCamera(float scale,float x,float y);
    private native void nativeSetSeekBarValues(float val1,float val2,float val3);

    private final Context mContext;
    //Handles the user input
    public MultiTouchGestureDetector mMultiTouchGestureDetector;
    private float distance =10;
    private float moveX=0,moveY=0;
    private float width,height;
    private int renderingMode=0; //Lines or text usw


    public GLRExample(final Context context){
        mContext=context;
        mMultiTouchGestureDetector=new MultiTouchGestureDetector(mContext,this);
    }

    @Override
    public void onSurfaceCreated(GL10 gl, EGLConfig config) {
        nativeOnSurfaceCreated(mContext);
    }

    @Override
    public void onSurfaceChanged(GL10 gl, int width, int height) {
        nativeOnSurfaceChanged(width,height);
        this.width=width;
        this.height=height;
    }

    @Override
    public void onDrawFrame(GL10 gl) {
        nativeMoveCamera(distance,moveX,moveY);
        nativeOnDrawFrame(renderingMode);
    }

    public void setSelectedMode(int mode){
        distance=10;
        moveX=0;
        moveY=0;
        renderingMode=mode;
    }

    public void setSeekBarValues(float val1,float val2,float val3){
        nativeSetSeekBarValues(val1,val2,val3);
    }

    @Override
    public void onScale(MultiTouchGestureDetector detector) {
        distance *=detector.getScale();
        if(distance>19)distance=19;
        if(distance<1)distance=1f;
        System.out.println("Scale:"+ distance);
    }

    @Override
    public void onMove(MultiTouchGestureDetector detector) {
        moveX-=detector.getMoveX()/width*3;
        moveY+=detector.getMoveY()/height*3;
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
