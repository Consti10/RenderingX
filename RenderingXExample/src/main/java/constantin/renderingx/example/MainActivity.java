package constantin.renderingx.example;

import android.Manifest;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.widget.Spinner;
import android.widget.Toast;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;

import constantin.renderingx.core.deviceinfo.AWriteGLESInfo;
import constantin.renderingx.core.deviceinfo.Extensions;
import constantin.renderingx.example.mono.AExampleRendering;
import constantin.renderingx.example.stereo.distortion.AExampleDistortion;
import constantin.renderingx.example.stereo.video360degree.AExample360Video;
import constantin.renderingx.example.supersync.AExampleSuperSync;
import constantin.helper.RequestPermissionHelper;

public class MainActivity extends AppCompatActivity {
    private Context context;
    private final RequestPermissionHelper requestPermissionHelper=new RequestPermissionHelper(new String[]{
            Manifest.permission.WRITE_EXTERNAL_STORAGE,
            Manifest.permission.READ_EXTERNAL_STORAGE
    });
    private Spinner mSpinner;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        context=this;
        setContentView(R.layout.activity_main);
        mSpinner=findViewById(R.id.spinner_360_video_type);

        //This retreives any HW info needed for the app
        AWriteGLESInfo.writeGLESInfoIfNeeded(this);

        findViewById(R.id.button).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                startActivity(new Intent().setClass(context, AExampleRendering.class));
            }
        });
        findViewById(R.id.button2).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if(isSuperSyncSupported()){
                    startActivity(new Intent().setClass(context, AExampleSuperSync.class));
                }else{
                    Toast.makeText(context,"SuperSync is not supported on this phone",Toast.LENGTH_LONG).show();
                }
            }
        });
        findViewById(R.id.button3).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                final Intent intent=new Intent().setClass(context, AExampleDistortion.class);
                startActivity(intent);
            }
        });
        findViewById(R.id.button4).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                final Intent intent=new Intent().setClass(context, AExample360Video.class);
                final int position = mSpinner.getSelectedItemPosition()==Spinner.INVALID_POSITION ? 0 : mSpinner.getSelectedItemPosition();
                final int SPHERE_MODE;
                final String VIDEO_FILENAME;
                if(position==0){
                    SPHERE_MODE= AExample360Video.SPHERE_MODE_GVR_EQUIRECTANGULAR;
                    VIDEO_FILENAME="360DegreeVideos/testRoom1_1920Mono.mp4";
                }else if(position==1){
                    SPHERE_MODE= AExample360Video.SPHERE_MODE_GVR_EQUIRECTANGULAR;
                    VIDEO_FILENAME="360DegreeVideos/paris_by_diego.mp4";
                } else if(position==2){
                    SPHERE_MODE= AExample360Video.SPHERE_MODE_INSTA360_TEST;
                    VIDEO_FILENAME="360DegreeVideos/insta_webbn_1_shortened.h264";
                }else{
                    SPHERE_MODE= AExample360Video.SPHERE_MODE_INSTA360_TEST2;
                    VIDEO_FILENAME="360DegreeVideos/insta_webbn_1_shortened.h264";
                }
                intent.putExtra(AExample360Video.KEY_SPHERE_MODE,SPHERE_MODE);
                intent.putExtra(AExample360Video.KEY_VIDEO_FILENAME,VIDEO_FILENAME);
                startActivity(intent);
            }
        });
        requestPermissionHelper.checkAndRequestPermissions(this);
    }

    @Override
    protected void onResume() {
        super.onResume();
    }



    @Override
    public void onRequestPermissionsResult(int requestCode,
                                           @NonNull String[] permissions, @NonNull int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        requestPermissionHelper.onRequestPermissionsResult(requestCode,permissions,grantResults);
    }

    public boolean isSuperSyncSupported(){
        return Extensions.available(context,Extensions.EGL_ANDROID_front_buffer_auto_refresh) && Extensions.available(context,Extensions.EGL_KHR_mutable_render_buffer);
    }

}
