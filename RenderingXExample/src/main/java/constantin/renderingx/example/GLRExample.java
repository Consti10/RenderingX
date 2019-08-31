package constantin.renderingx.example;

import android.content.Context;
import android.opengl.GLSurfaceView;
import android.view.MotionEvent;

import com.dinuscxj.gesture.MultiTouchGestureDetector;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;



public class GLRExample implements GLSurfaceView.Renderer, MultiTouchGestureDetector.OnMultiTouchGestureListener{
    static {
        System.loadLibrary("example-renderer");
    }
    private native void nativeOnSurfaceCreated(final Context context);
    private native void nativeOnSurfaceChanged(int width,int height);
    private native void nativeOnDrawFrame();
    private native void nativeMoveCamera(float scale,float x,float y);

    private final Context mContext;
    //Handles the user input
    private MultiTouchGestureDetector mMultiTouchGestureDetector;
    private float distance =10;
    private float moveX=0,moveY=0;
    private float width,height;


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
        nativeOnDrawFrame();
    }

    public void touch(MotionEvent event) {
        mMultiTouchGestureDetector.onTouchEvent(event);
        //System.out.println("touch");
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
