package constantin.renderingx.example.supersync;

import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;

import constantin.renderingx.core.ViewSuperSync;

public class AExampleSuperSync extends AppCompatActivity {
    private ViewSuperSync mViewSuperSync;
    private GLRExampleSuperSync mGLRStereoSuperSync;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mViewSuperSync=new ViewSuperSync(this);
        mGLRStereoSuperSync = new GLRExampleSuperSync(this,mViewSuperSync.getGvrApi());
        mViewSuperSync.setRenderer(mGLRStereoSuperSync);
        setContentView(mViewSuperSync);
    }

    @Override
    protected void onResume() {
        super.onResume();
        System.out.println("YYY onResumeX()");
    }

    @Override
    protected void onPause(){
        super.onPause();
        System.out.println("YYY onPauseX()");
    }

    @Override
    protected void onDestroy(){
        super.onDestroy();
        mViewSuperSync=null;
        mGLRStereoSuperSync=null;
    }
}
