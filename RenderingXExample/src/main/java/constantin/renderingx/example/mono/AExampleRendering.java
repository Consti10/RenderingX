package constantin.renderingx.example.mono;

import android.annotation.SuppressLint;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.view.MotionEvent;
import android.view.View;
import android.view.WindowManager;
import android.widget.AdapterView;
import android.widget.SeekBar;
import android.widget.Spinner;

import androidx.appcompat.app.AppCompatActivity;

import constantin.renderingx.example.R;
import constantin.renderingx.example.databinding.ActivityExampleRenderingBinding;

public class AExampleRendering extends AppCompatActivity implements SeekBar.OnSeekBarChangeListener {
    private GLRExample renderer;
    private ActivityExampleRenderingBinding binding;

    @SuppressLint("ClickableViewAccessibility")
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        binding = ActivityExampleRenderingBinding.inflate(getLayoutInflater());
        setContentView(binding.getRoot());
        renderer=new GLRExample(this);
        binding.MyGLSurfaceView.ENABLE_EGL_KHR_DEBUG=true;
        binding.MyGLSurfaceView.setRenderer(renderer);
        binding.MyGLSurfaceView.setOnTouchListener(new View.OnTouchListener() {
            @Override
            public boolean onTouch(View v, MotionEvent event) {
                renderer.mMultiTouchGestureDetector.onTouchEvent(event);
                return true;
            }
        });
        binding.MyGLSurfaceView.setZOrderOnTop(false);
        final Spinner spinner=findViewById(R.id.spinner);
        spinner.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
                renderer.setSelectedMode(position);
                enableDisableSeekBars(position);
                setDefaultValues(position);
            }
            @Override
            public void onNothingSelected(AdapterView<?> parent) { }
        });
        binding.seekBar1.setOnSeekBarChangeListener(this);
        binding.seekBar2.setOnSeekBarChangeListener(this);
        binding.seekBar3.setOnSeekBarChangeListener(this);
        enableDisableSeekBars(0);
        setDefaultValues(0);
    }

    @Override
    protected void onResume() {
        super.onResume();
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
    }

    private void enableDisableSeekBars(final int mode){
        if(mode==0 || mode==1 || mode==2){
            binding.seekBar1.setVisibility(View.VISIBLE);
            binding.seekBar2.setVisibility(View.VISIBLE);
            binding.seekBar3.setVisibility(View.VISIBLE);
        }else{
            binding.seekBar1.setVisibility(View.GONE);
            binding.seekBar2.setVisibility(View.GONE);
            binding.seekBar3.setVisibility(View.GONE);
        }
    }

    private void setDefaultValues(final int mode){
        if(mode==0 || mode==1){
            binding.seekBar1.setProgress(20);
            binding.seekBar2.setProgress(10);
            binding.seekBar3.setProgress(10);
        }else{
            binding.seekBar1.setProgress(40);
            binding.seekBar2.setProgress(10);
            binding.seekBar3.setProgress(10);
        }
    }

    @Override
    public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
        System.out.println("SeekBar values:"+binding.seekBar1.getProgress()+" "+binding.seekBar2.getProgress());
        renderer.setSeekBarValues(binding.seekBar1.getProgress(),binding.seekBar2.getProgress(),binding.seekBar3.getProgress());
    }

    @Override
    public void onStartTrackingTouch(SeekBar seekBar) { }

    @Override
    public void onStopTrackingTouch(SeekBar seekBar) { }
}
