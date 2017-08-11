package com.haier.networkdetect.networkdetect;

import android.util.Log;

/**
 * Created by GourdBoy on 2017/8/11.
 */

public class NativeNetworkUtils
{
	static
	{
		System.loadLibrary("networkdetect");
	}
	private NativeNetworkUtils()
	{

	}
	private static class NativeMethodHolder
	{
		public static final  NativeNetworkUtils INSTANCE = new NativeNetworkUtils();
	}
	public static NativeNetworkUtils getInstance()
	{
		return NativeMethodHolder.INSTANCE;
	}
	public native void networkDetectInit();
	public native void networkDetectSetIP1(String ip);
	public native void networkDetectSetIP2(String ip);
	public native void networkDetectThreadStart();
	public native void networkDetectThreadStop();
	private void networkDetectCallback1(boolean isConnect)
	{
		Log.i("NETWORKDETECT","ip1 connnect "+isConnect);
	}
	private void networkDetectCallback2(boolean isConnect)
	{
		Log.i("NETWORKDETECT","ip2 connnect "+isConnect);
	}
}
