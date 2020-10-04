package constantin.renderingx.example;


import android.content.Intent;

import androidx.test.filters.LargeTest;
import androidx.test.rule.ActivityTestRule;
import androidx.test.rule.GrantPermissionRule;

import org.junit.Rule;
import org.junit.Test;

import constantin.renderingx.core.deviceinfo.AWriteGLESInfo;
import constantin.renderingx.example.mono.AExampleRendering;
import constantin.renderingx.example.stereo.distortion.AExampleDistortion;
import constantin.renderingx.example.stereo.video360degree.AExample360Video;
import constantin.renderingx.example.supersync.AExampleSuperSync;

//Launch Main Activity
//Launch the writeGLESInfo Activity manually
//Launch the Rendering Activity and select all 4 options

@LargeTest
public class MainActivityTest {
    private static final int WAIT_TIME = 5*1000; //2 seconds

    @Rule
    public ActivityTestRule<MainActivity> mMainActivityRule = new ActivityTestRule<>(MainActivity.class);
    @Rule
    public ActivityTestRule<AWriteGLESInfo> mGLESInfoRule = new ActivityTestRule<>(AWriteGLESInfo.class,false,false);
    @Rule
    public ActivityTestRule<AExampleSuperSync> mExampleSuperSyncRule = new ActivityTestRule<>(AExampleSuperSync.class,false,false);
    @Rule
    public ActivityTestRule<AExampleRendering> mExampleRenderingRule = new ActivityTestRule<>(AExampleRendering.class,false,false);
    @Rule
    public ActivityTestRule<AExampleDistortion> mExampleVRRenderingVDDCRule = new ActivityTestRule<>(AExampleDistortion.class,false,false);
    @Rule
    public ActivityTestRule<AExample360Video> mExampleVRRendering360VideoRule = new ActivityTestRule<>(AExample360Video.class,false,false);

    @Rule
    public GrantPermissionRule mGrantPermissionRule =
            GrantPermissionRule.grant(
                    "android.permission.READ_EXTERNAL_STORAGE",
                    "android.permission.WRITE_EXTERNAL_STORAGE");


    private void startGLESInfo(){
        Intent i = new Intent();
        mGLESInfoRule.launchActivity(i);
        mGLESInfoRule.finishActivity();
    }

    private void testRendering(){
        Intent i = new Intent();
        mExampleRenderingRule.launchActivity(i);
        try { Thread.sleep(WAIT_TIME); } catch (InterruptedException e) { e.printStackTrace(); }
        mExampleRenderingRule.finishActivity();
    }


    private void testVrVDDC(){
        Intent i = new Intent();
        mExampleVRRenderingVDDCRule.launchActivity(i);
        try { Thread.sleep(WAIT_TIME); } catch (InterruptedException e) { e.printStackTrace(); }
        mExampleVRRenderingVDDCRule.finishActivity();
    }

    private void testVr360Video(){
        Intent i = new Intent();
        i.putExtra(AExample360Video.KEY_SPHERE_MODE,AExample360Video.SPHERE_MODE_GVR_EQUIRECTANGULAR);
        i.putExtra(AExample360Video.KEY_VIDEO_FILENAME,"360DegreeVideos/testRoom1_1920Mono.mp4");
        mExampleVRRendering360VideoRule.launchActivity(i);
        try { Thread.sleep(WAIT_TIME); } catch (InterruptedException e) { e.printStackTrace(); }
        final int nUpdates=mExampleVRRendering360VideoRule.getActivity().getNSurfaceTextureUpdateReturnedTrue();
        if(nUpdates<60){
            throw new RuntimeException("Error nUpdates < 60");
        }
        mExampleVRRendering360VideoRule.finishActivity();
    }

    private void testSuperSyncIfPossible(){
        if(mMainActivityRule.getActivity().isSuperSyncSupported()){
            Intent i = new Intent();
            mExampleSuperSyncRule.launchActivity(i);
            try { Thread.sleep(WAIT_TIME); } catch (InterruptedException e) { e.printStackTrace(); }
            mExampleSuperSyncRule.finishActivity();
        }
    }

    @Test
    public void mainActivityTest() {
        startGLESInfo();
        //without VDDC / VR
        testRendering();
        testVrVDDC();
        testVr360Video();
        testSuperSyncIfPossible();
    }

}

