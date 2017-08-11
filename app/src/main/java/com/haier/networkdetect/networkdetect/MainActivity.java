package com.haier.networkdetect.networkdetect;

import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;

public class MainActivity extends AppCompatActivity
{

	private NativeNetworkUtils nwu;
	@Override
	protected void onCreate(Bundle savedInstanceState)
	{
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);
		Button ping1_btn = (Button) findViewById(R.id.ping1_btn);
		Button ping2_btn = (Button) findViewById(R.id.ping2_btn);
		Button close_btn = (Button) findViewById(R.id.close_btn);
		nwu = NativeNetworkUtils.getInstance();
		nwu.networkDetectInit();
		nwu.networkDetectSetIP1("192.168.1.171");
		nwu.networkDetectSetIP2("192.168.2.171");
		nwu.networkDetectThreadStart();
		ping1_btn.setOnClickListener(new View.OnClickListener()
		{
			@Override
			public void onClick(View v)
			{
				nwu.networkDetectSetIP1("192.168.3.101");
			}
		});
		ping2_btn.setOnClickListener(new View.OnClickListener()
		{
			@Override
			public void onClick(View v)
			{
				nwu.networkDetectSetIP2("192.168.1.102");
			}
		});
		close_btn.setOnClickListener(new View.OnClickListener()
		{
			@Override
			public void onClick(View v)
			{
				nwu.networkDetectThreadStop();
			}
		});
	}

	@Override
	protected void onDestroy()
	{
		super.onDestroy();
		nwu.networkDetectThreadStop();
	}
}
