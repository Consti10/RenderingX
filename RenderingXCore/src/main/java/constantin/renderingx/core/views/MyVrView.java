package constantin.renderingx.core.views;

import android.content.Context;
import android.opengl.GLSurfaceView;
import android.util.AttributeSet;
import android.view.View;

import androidx.appcompat.app.AppCompatActivity;

public class MyVrView extends MyVRLayout {
    MyGLSurfaceView glSurfaceView;

    public MyVrView(Context context) {
        super(context);
    }

    public MyVrView(Context context, AttributeSet attrs) {
        super(context,attrs);
    }

    private void init(final AppCompatActivity activity){
        glSurfaceView=new MyGLSurfaceView(activity);
        super.setPresentationView(glSurfaceView);
    }

    public void setRenderer(final GLSurfaceView.Renderer renderer){
        glSurfaceView.setRenderer(renderer);
    }

    public void setEGLContextClientVersion(final int version){
        glSurfaceView.setEGLContextClientVersion(version);
    }

    @Override
    public void setPresentationView(View presentationView){
        throw new IllegalStateException("Illegal access to setPresentationView");
    }
}
