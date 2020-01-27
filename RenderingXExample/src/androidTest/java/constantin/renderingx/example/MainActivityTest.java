package constantin.renderingx.example;


import android.content.Intent;

import androidx.test.filters.LargeTest;
import androidx.test.rule.ActivityTestRule;
import androidx.test.rule.GrantPermissionRule;

import org.junit.Rule;
import org.junit.Test;

import constantin.renderingx.core.GLESInfo.AWriteGLESInfo;
import constantin.renderingx.example.renderer1.AExampleRendering;
import constantin.renderingx.example.renderer2.AExampleVRRendering;

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
    public ActivityTestRule<AExampleRendering> mExampleRenderingRule = new ActivityTestRule<>(AExampleRendering.class,false,false);
    @Rule
    public ActivityTestRule<AExampleVRRendering> mExampleVRRenderingRule = new ActivityTestRule<>(AExampleVRRendering.class,false,false);


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

    private void testVRRendering(int mode){
        Intent i = new Intent();
        i.putExtra(AExampleVRRendering.KEY_MODE,mode);
        mExampleVRRenderingRule.launchActivity(i);
        try { Thread.sleep(WAIT_TIME); } catch (InterruptedException e) { e.printStackTrace(); }
        mExampleVRRenderingRule.finishActivity();
    }


    @Test
    public void mainActivityTest() {
        startGLESInfo();
        //without VDDC / VR
        testRendering();
        //VDDC
        testVRRendering(0);
        //Sphere Equi
        testVRRendering(1);
        //Sphere Gvr
        testVRRendering(2);
    }

}

