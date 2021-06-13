package com.example.ffmpegr4.util;

import android.content.ContentUris;
import android.content.Context;
import android.content.Intent;
import android.database.Cursor;
import android.net.Uri;
import android.os.Build;
import android.provider.DocumentsContract;
import android.provider.MediaStore;
import android.util.Log;

import androidx.annotation.RequiresApi;

import com.example.ffmpegr4.helper.Logger;

/**
 * 虚拟路径转实际路径？
 * 方法有待理解
 */
public class PathConvertor {
    public  static String getImagePath(Context context, Uri uri, String selection){
        String path="";
        Cursor cursor=context.getContentResolver().query(uri,null,selection,null,null);
        if(cursor!=null){
            if(cursor.moveToFirst()){
                path=cursor.getString(cursor.getColumnIndex(MediaStore.Video.Media.DATA));
            }
            cursor.close();
        }
        return path;
    }

    @RequiresApi(api = Build.VERSION_CODES.KITKAT)
    public static String utilhandler(Context context, Intent data) {
        Log.d("111before", data.getDataString());
        String imagePath = null;
        Uri uri = data.getData();
        if (DocumentsContract.isDocumentUri(context, uri)) {
            String docId = DocumentsContract.getDocumentId(uri);
            if ("com.android.providers.media.documents".equals(uri.getAuthority())) {
                String id = docId.split(":")[1];
                String selection = MediaStore.Video.Media._ID + "=" + id;
                imagePath = getImagePath(context,MediaStore.Video.Media.EXTERNAL_CONTENT_URI, selection);
            }
            else if("com.android.providers.downloads.documents".equals(uri.getAuthority())){
                Uri contentUri= ContentUris.withAppendedId(Uri.parse("content://downloads/public_downloads"),Long.valueOf(docId));
                imagePath=getImagePath(context,contentUri,null);
            }
        }
        else if ("content".equalsIgnoreCase(uri.getScheme())) {
            imagePath=getImagePath(context,uri,null);
        }else if("file".equalsIgnoreCase(uri.getScheme())){
            imagePath=uri.getPath() ;
        }
        Logger.D(imagePath);
        return imagePath;
    }
}
