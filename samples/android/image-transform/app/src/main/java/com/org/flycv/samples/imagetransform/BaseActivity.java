package com.org.flycv.samples.imagetransform;

import android.Manifest;
import android.app.Activity;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.util.TypedValue;
import android.view.ViewGroup;
import android.widget.TextView;
import android.widget.Toast;

import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;

import java.util.ArrayList;
import java.util.List;


public class BaseActivity extends Activity {
    private Runnable doAfterAllPermissionsGranted;
    private static final int REQUEST_PERMISSIONS = 1;
    protected String[] neededPermissions = {
            Manifest.permission.WRITE_EXTERNAL_STORAGE,
            Manifest.permission.READ_EXTERNAL_STORAGE,
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        if (neededPermissions.length == 0) {
            return;
        }
        //  进行所有权限检测
        if (doAfterAllPermissionsGranted != null) {
            doAfterAllPermissionsGranted.run();
            doAfterAllPermissionsGranted = null;
        } else {

            List<String> deniedPermissions = new ArrayList<>();
            for (String permission : neededPermissions) {
                if (ContextCompat.checkSelfPermission(this, permission) != PackageManager.PERMISSION_GRANTED) {
                    deniedPermissions.add(permission);
                }
            }
            if (deniedPermissions.isEmpty()) {
                doAfterAllPermissionsGranted();
            } else {
                String[] array = new String[deniedPermissions.size()];
                array = deniedPermissions.toArray(array);
                ActivityCompat.requestPermissions(this, array, REQUEST_PERMISSIONS);
            }
        }
        TextView tvTitle = new TextView(this);
        tvTitle.setText(this.getPackageName());
        tvTitle.setTextSize(TypedValue.COMPLEX_UNIT_SP, 18);
        ((ViewGroup) getWindow().getDecorView()).addView(tvTitle);
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, String[] permissions, int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        if (requestCode == REQUEST_PERMISSIONS) {
            boolean permissionsAllGranted = true;
            for (int grantResult : grantResults) {
                if (grantResult != PackageManager.PERMISSION_GRANTED) {
                    permissionsAllGranted = false;
                    break;
                }
            }
            if (permissionsAllGranted) {
                doAfterAllPermissionsGranted = this::doAfterAllPermissionsGranted;
            } else {
                doAfterAllPermissionsGranted = () -> {
                    Toast.makeText(this, "权限申请被拒绝 , 退出 ...", Toast.LENGTH_SHORT).show();
                    finish();
                };
            }
        }
    }

    private void doAfterAllPermissionsGranted() {
        Toast.makeText(this, "恭喜您 , APP权限获取成功 ...", Toast.LENGTH_SHORT).show();
    }
}
