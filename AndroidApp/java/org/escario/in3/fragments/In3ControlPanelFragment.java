package org.escario.in3.fragments;

import android.app.Fragment;
import android.bluetooth.BluetoothSocket;
import android.graphics.Color;
import android.graphics.LinearGradient;
import android.graphics.Paint;
import android.graphics.Point;
import android.graphics.Shader;
import android.os.AsyncTask;
import android.os.Bundle;
import android.view.Display;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.SeekBar;
import android.widget.TextView;

import com.androidplot.xy.LineAndPointFormatter;
import com.androidplot.xy.XYPlot;
import com.androidplot.xy.XYSeries;
import com.androidplot.xy.XYStepMode;

import org.escario.in3.R;
import org.escario.in3.model.Value;
import org.escario.in3.model.TransmittedObject;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.text.DecimalFormat;
import java.text.FieldPosition;
import java.text.Format;
import java.text.ParsePosition;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;

/**
 * Created by alejandro on 7/5/15.
 */
public class In3ControlPanelFragment extends Fragment {

    private static BluetoothSocket socket;
    private View view;
    private SeekBar skbTemp;
    private TextView txtTarget;

    private XYPlot plotTemperature;
    private XYPlot plotHumidity;
    ArrayList<Value> temp_val, hum_val, target;
    final static int MAX_TEMP = 100;
    float target_temp = 36.5f;

    public In3ControlPanelFragment(){
        temp_val = new ArrayList<>();
        hum_val = new ArrayList<>();
        target = new ArrayList<>();
    }

    public void setSocket(BluetoothSocket socket){
        this.socket = socket;
    }


    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        if(view == null) {
            view = inflater.inflate(R.layout.in3_console, container, false);
            txtTarget = (TextView) view.findViewById(R.id.txtTarget);
            txtTarget.setText(String.valueOf(target_temp));
            skbTemp = (SeekBar) view.findViewById(R.id.skbTemp);
            skbTemp.setProgress((int) target_temp);
            skbTemp.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
                @Override
                public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                    target_temp = progress;
                    byte[] arr = {1, 2, 3, 4, (byte) target_temp, 0};
                    txtTarget.setText(String.valueOf(progress));
                    sendMessage(arr);
                }

                @Override
                public void onStartTrackingTouch(SeekBar seekBar) {

                }

                @Override
                public void onStopTrackingTouch(SeekBar seekBar) {

                }
            });
            plotTemperature = (XYPlot) view.findViewById(R.id.plotTemperature);
            setUpPlot(plotTemperature);
            // getInstance and position datasets:
            addTemp2Plot(plotTemperature);
            addTargetTemp2Plot(plotTemperature);
            plotHumidity = (XYPlot) view.findViewById(R.id.plotHumidity);
            setUpPlot(plotHumidity);
            addHumidity2Plot(plotHumidity);
        }
        return view;
    }


    @Override
    public void onDestroyView(){
        super.onStop();
        if(socket.isConnected()) {
            try {
                socket.close();
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
    }

    private void addTargetTemp2Plot(XYPlot plot){
        SampleDynamicSeries sine1Series = new SampleDynamicSeries(target, "Target");

        LineAndPointFormatter formatter1 = new LineAndPointFormatter(
                Color.rgb(250, 0, 0),                   // line color
                null,                   // point color
                null,
                null);
        formatter1.getLinePaint().setStrokeJoin(Paint.Join.ROUND);
        formatter1.getLinePaint().setStrokeWidth(5);
        plot.addSeries(sine1Series,
                formatter1);
    }

    private int getWindowHeight(){
        Display display = getActivity().getWindowManager().getDefaultDisplay();
        Point size = new Point();
        display.getSize(size);
        return size.y;
    }

    private void addHumidity2Plot(XYPlot plot){
        SampleDynamicSeries sine2Series = new SampleDynamicSeries(hum_val, "Humidity");

        LineAndPointFormatter formatter2 =
                new LineAndPointFormatter(Color.rgb(0, 0, 250), null, null, null);
        formatter2.getLinePaint().setStrokeWidth(5);
        Paint lineFill = new Paint();
        lineFill.setAlpha(200);
        lineFill.setShader(new LinearGradient(0, 0, 0, getWindowHeight(), Color.WHITE, Color.BLUE, Shader.TileMode.MIRROR));

        formatter2.setFillPaint(lineFill);
        formatter2.getLinePaint().setStrokeJoin(Paint.Join.ROUND);

        plot.addSeries(sine2Series, formatter2);
    }

    private void addTemp2Plot(XYPlot plot){
        SampleDynamicSeries sine2Series = new SampleDynamicSeries(temp_val, "Temp");

        LineAndPointFormatter formatter2 =
                new LineAndPointFormatter(Color.rgb(0, 250, 0), null, null, null);
        formatter2.getLinePaint().setStrokeWidth(5);
        Paint lineFill = new Paint();
        lineFill.setAlpha(200);
        lineFill.setShader(new LinearGradient(0, 0, 0, getWindowHeight(), Color.WHITE, Color.GREEN, Shader.TileMode.MIRROR));

        formatter2.setFillPaint(lineFill);
        formatter2.getLinePaint().setStrokeJoin(Paint.Join.ROUND);

        plot.addSeries(sine2Series, formatter2);
    }

    private void setUpPlot(XYPlot plot) {

        // only display whole numbers in domain labels
        plot.getGraphWidget().setDomainValueFormat(new DecimalFormat("0"));

        // thin out domain tick labels so they dont overlap each other:
        plot.setDomainStepMode(XYStepMode.SUBDIVIDE);
        plot.setDomainStepValue(5);
        plot.setDomainValueFormat(new Format() {
            private SimpleDateFormat dateFormat = new SimpleDateFormat("HH:mm:ss");

            @Override
            public StringBuffer format(Object obj, StringBuffer toAppendTo, FieldPosition pos) {
                long timestamp = ((Number) obj).longValue();
                Date date = new Date(timestamp);
                return dateFormat.format(date, toAppendTo, pos);
            }

            @Override
            public Object parseObject(String source, ParsePosition pos) {
                return null;

            }
        });

        plot.setRangeStepMode(XYStepMode.INCREMENT_BY_PIXELS);
        plot.setRangeStepValue(100);

        plot.setRangeValueFormat(new DecimalFormat("##.##"));

        plot.setBorderStyle(XYPlot.BorderStyle.NONE, null, null);
        plot.getGraphWidget().getBackgroundPaint().setColor(Color.TRANSPARENT);
        plot.getGraphWidget().getGridBackgroundPaint().setColor(Color.WHITE);
        plot.getGraphWidget().getDomainLabelPaint().setColor(Color.BLACK);
        plot.getGraphWidget().getRangeLabelPaint().setColor(Color.BLACK);

        plot.getGraphWidget().getDomainOriginLabelPaint().setColor(Color.BLACK);

        plot.getLegendWidget().getTextPaint().setColor(Color.BLACK);
        plot.getDomainLabelWidget().getLabelPaint().setColor(Color.BLACK);
        plot.getRangeLabelWidget().getLabelPaint().setColor(Color.BLACK);
        plot.getTitleWidget().getLabelPaint().setColor(Color.BLACK);
    }

    @Override
    public void onStart() {
        super.onStart();
        if(!socket.isConnected()){
            try {
                socket.connect();
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
        new ListenThread().execute(socket);
    }

    private class ListenThread extends AsyncTask<BluetoothSocket, TransmittedObject, Void> {

        protected Void doInBackground(BluetoothSocket... socket) {
            byte[] buffer = new byte[7];
            byte[] read = new byte[1];
            try{
                InputStream is = socket[0].getInputStream();
                while(true){
                    int readBytes = is.read(read);
                    if(readBytes != -1){
                        buffer[0] = buffer[1];
                        buffer[1] = buffer[2];
                        buffer[2] = buffer[3];
                        buffer[3] = buffer[4];
                        buffer[4] = buffer[5];
                        buffer[5] = buffer[6];
                        buffer[6] = read[0];
                    }
                    if(buffer[0] == 1 && buffer[1] == 2 && buffer[2] == 3 && buffer[3] == 4) {
                        Float value = buffer[5] + (float)buffer[6]/100;
                        switch(buffer[4]){
                            case 'T':
                            case 'H':
                                publishProgress(new TransmittedObject(buffer[4], value));
                                break;
                            case 'O':
                                skbTemp.setProgress(value.intValue());
                                break;
                        }
                    }
                }
            }catch(IOException e){
                e.printStackTrace();
            }
            return null;
        }

        protected void onProgressUpdate(TransmittedObject... temp) {
            long now = System.currentTimeMillis();
            if(temp[0].type == 'T') {
                Value val = new Value(temp[0].value, now);
                temp_val.add(val);
                if (temp_val.size() >= MAX_TEMP) {
                    temp_val.remove(0);
                }

                val = new Value(target_temp, now);
                target.add(val);
                if (target.size() >= MAX_TEMP) {
                    target.remove(0);
                }

                plotTemperature.redraw();
            }else if(temp[0].type == 'H') {
                Value val = new Value(temp[0].value, now);
                hum_val.add(val);
                if (hum_val.size() >= MAX_TEMP) {
                    hum_val.remove(0);
                }

                plotHumidity.redraw();
            }
        }
    };

    class SampleDynamicSeries implements XYSeries {
        private ArrayList<Value> datasource;
        private String title;

        public SampleDynamicSeries(ArrayList<Value> datasource, String title) {
            this.datasource = datasource;
            this.title = title;
        }

        @Override
        public String getTitle() {
            return title;
        }

        @Override
        public int size() {
            return datasource.size();
        }

        @Override
        public Number getX(int index) {
            return datasource.get(index).time;
        }

        @Override
        public Number getY(int index) {
            return datasource.get(index).temperature;
        }
    }



    private void sendMessage(byte[] msg){
        OutputStream os;
        try {
            os = socket.getOutputStream();
            os.write(msg);
        }catch(IOException e){
            e.printStackTrace();
        }
    }
}