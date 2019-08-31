package constantin.renderingx.example;

import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;

import constantin.renderingX.PerformanceHelper;
import constantin.renderingX.ViewSuperSync;

public class ExampleSuperSyncA extends AppCompatActivity {
    private ViewSuperSync mViewSuperSync;
    private GLRSuperSyncExample mGLRStereoSuperSync;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mViewSuperSync=new ViewSuperSync(this);
        mGLRStereoSuperSync = new GLRSuperSyncExample(this,mViewSuperSync.getGvrApi().getNativeGvrContext());
        mViewSuperSync.setRenderer(mGLRStereoSuperSync);
        setContentView(mViewSuperSync);
    }

    @Override
    protected void onResume() {
        super.onResume();
        System.out.println("YYY onResume()");
        PerformanceHelper.setImmersiveSticky(this);
        if(true){
            PerformanceHelper.enableAndroidVRModeIfPossible(this);
        }
        PerformanceHelper.enableSustainedPerformanceIfPossible(this);
        mViewSuperSync.resume();
    }

    @Override
    protected void onPause(){
        super.onPause();
        System.out.println("YYY onPause()");
        mViewSuperSync.pause();
        PerformanceHelper.disableSustainedPerformanceIfEnabled(this);
        PerformanceHelper.disableAndroidVRModeIfEnabled(this);
    }

    @Override
    protected void onDestroy(){
        super.onDestroy();
        mViewSuperSync.destroy();
        mViewSuperSync=null;
        mGLRStereoSuperSync=null;
    }
}
