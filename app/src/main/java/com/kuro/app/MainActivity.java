package com.kuro.app;

import android.app.Activity;
import android.content.pm.InstallSourceInfo;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.widget.TextView;
import android.widget.Toast;

import com.kuro.R;

import java.io.FileInputStream;
import java.security.MessageDigest;

public class MainActivity extends Activity {
    TextView signature1, signature2;
    TextView apkPath1, apkPath2, apkPath3, apkPath4;
    TextView apkcrc;
    TextView apkinstaller1, apkinstaller2;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        signature1 = findViewById(R.id.signature1);
        signature2 = findViewById(R.id.signature2);
        apkPath1 = findViewById(R.id.apkPath1);
        apkPath2 = findViewById(R.id.apkPath2);
        apkPath3 = findViewById(R.id.apkPath3);
        apkPath4 = findViewById(R.id.apkPath4);
        apkcrc = findViewById(R.id.apkcrc);
        apkinstaller1 = findViewById(R.id.apkinstaller1);
        apkinstaller2 = findViewById(R.id.apkinstaller2);

        try {
            String packageName = getPackageName();

            PackageManager pm = getPackageManager();

            String sig1 = md5(pm.getPackageInfo(packageName, PackageManager.GET_SIGNATURES).signatures[0].toByteArray());
            signature1.setText(signature1.getText() + String.valueOf(sig1));
            String sig2 = "";
            if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.P) {
                sig2 = md5(pm.getPackageInfo(packageName, PackageManager.GET_SIGNING_CERTIFICATES).signingInfo.getApkContentsSigners()[0].toByteArray());
            }
            signature2.setText(signature2.getText() + String.valueOf(sig2));

            apkPath1.setText(apkPath1.getText() + pm.getPackageInfo(packageName, 0).applicationInfo.sourceDir + " | " + pm.getPackageInfo(packageName, 0).applicationInfo.publicSourceDir);
            apkPath2.setText(apkPath2.getText() + getApplicationInfo().sourceDir + " | " + getApplicationInfo().publicSourceDir);
            apkPath3.setText(apkPath3.getText() + getPackageCodePath());
            apkPath4.setText(apkPath4.getText() + getPackageResourcePath());

            apkinstaller1.setText(apkinstaller1.getText() + pm.getInstallerPackageName(packageName));

            String installer2 = "";
            if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.R) {
                InstallSourceInfo installSourceInfo = pm.getInstallSourceInfo(packageName);
                installer2 += installSourceInfo.getInitiatingPackageName();
                installer2 += "|";
                installer2 += installSourceInfo.getInitiatingPackageSigningInfo() != null ? md5(installSourceInfo.getInitiatingPackageSigningInfo().getApkContentsSigners()[0].toByteArray()) : "";
                installer2 += "|";
                installer2 += installSourceInfo.getOriginatingPackageName();
                installer2 += "|";
                installer2 += installSourceInfo.getInstallingPackageName();

                apkinstaller2.setText(apkinstaller2.getText() + installer2);
            }
        } catch (Exception e) {
            Toast.makeText(this, e.getMessage(), Toast.LENGTH_LONG).show();
        }

        try {
            FileInputStream fis = new FileInputStream(getPackageCodePath());

            byte[] buffer = new byte[fis.available()];
            fis.read(buffer);
            fis.close();

            int crc = crc32(buffer);
            apkcrc.setText(apkcrc.getText() + "0x" + Integer.toHexString(crc));
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    public String md5(byte[] data) {
        final String MD5 = "MD5";
        try {
            MessageDigest digest = java.security.MessageDigest.getInstance(MD5);
            digest.update(data);
            byte messageDigest[] = digest.digest();

            StringBuilder hexString = new StringBuilder();
            for (byte aMessageDigest : messageDigest) {
                String h = Integer.toHexString(0xFF & aMessageDigest);
                while (h.length() < 2)
                    h = "0" + h;
                hexString.append(h);
            }
            return hexString.toString();

        } catch (Exception e) {
            e.printStackTrace();
        }
        return "";
    }

    int crc32(byte[] buffer) {
        int crc = 0xffffffff;
        for (byte b : buffer) {
            crc = crc ^ b;
            for (int i = 0; i < 8; i++) {
                if ((crc & 1) == 1) {
                    crc = (crc >>> 1) ^ 0xedb88320;
                } else {
                    crc = crc >>> 1;
                }
            }
        }
        return ~crc;
    }
}
