package constantin.renderingx.example;


import android.content.Intent;

import androidx.test.filters.LargeTest;
import androidx.test.rule.ActivityTestRule;
import androidx.test.rule.GrantPermissionRule;
import androidx.test.runner.AndroidJUnit4;

import org.junit.Rule;
import org.junit.Test;
import org.junit.runner.RunWith;

import constantin.renderingX.GLESInfo.AWriteGLESInfo;

//Launch Main Activity
//Launch the writeGLESInfo Activity manually
//Launch the Rendering Activity and select all 4 options

@LargeTest
@RunWith(AndroidJUnit4.class)
public class MainActivityTest {
    private static final int WAIT_TIME = 2*1000; //2 seconds

    @Rule
    public ActivityTestRule<MainActivity> mActivityTestRule = new ActivityTestRule<>(MainActivity.class);

    @Rule
    public ActivityTestRule<AWriteGLESInfo> mGLESInfoRule = new ActivityTestRule<>(AWriteGLESInfo.class,false,false);
    @Rule
    public ActivityTestRule<ExampleRenderingA> mExampleRenderingRule = new ActivityTestRule<>(ExampleRenderingA.class,false,false);

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


    @Test
    public void mainActivityTest() {
        startGLESInfo();
        testRendering();
    }

}

