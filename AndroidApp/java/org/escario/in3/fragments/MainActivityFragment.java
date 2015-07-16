package org.escario.in3.fragments;

import android.app.Fragment;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Bundle;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.ImageButton;
import android.widget.ListView;
import android.widget.TabHost;
import android.widget.TextView;
import android.widget.Toast;
import android.widget.ToggleButton;

import org.escario.in3.R;

import java.io.IOException;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.List;
import java.util.Set;

public class MainActivityFragment extends Fragment {

    private final static String TAB_PAIRED = "paired";
    private final static String TAB_DISCOVER = "discover";

    private Set<BluetoothDevice> pairedDevices;
    private List<BluetoothDevice> discoveredDevices;

    private BluetoothAdapter bluetoothAdapter;

    private View view;
    private ToggleButton btnBluettothEnabled;
    private TabHost tbhConnect;
    private ListView lstPairedDevices, lstDiscoverDevices;

    public MainActivityFragment() {
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        view = inflater.inflate(R.layout.fragment_main, container, false);

        bluetoothAdapter = BluetoothAdapter.getDefaultAdapter();

        btnBluettothEnabled = (ToggleButton) view.findViewById(R.id.btnBluetoothEnabled);
        btnBluettothEnabled.setTextOff(getString(R.string.bluetooth) + " " + getString(R.string.off));
        btnBluettothEnabled.setTextOn(getString(R.string.bluetooth) + " " + getString(R.string.on));
        btnBluettothEnabled.setOnClickListener(toggleBluetooth);

        tbhConnect =(TabHost)view.findViewById(R.id.tbhConnect);
        tbhConnect.setOnTabChangedListener(new TabHost.OnTabChangeListener() {
            @Override
            public void onTabChanged(String tabId) {
                switch (tabId){
                    case TAB_PAIRED:
                        break;
                    case TAB_DISCOVER:
                        startDiscoverTask();
                        break;
                }
            }
        });
        tbhConnect.setup();

        TabHost.TabSpec spec=tbhConnect.newTabSpec(TAB_PAIRED);
        spec.setContent(R.id.lstPairedDevices);
        spec.setIndicator(getString(R.string.pairedDevices));
        tbhConnect.addTab(spec);

        spec=tbhConnect.newTabSpec(TAB_DISCOVER);
        spec.setContent(R.id.lstDiscoverDevices);
        spec.setIndicator(getString(R.string.discoverDevices));
        tbhConnect.addTab(spec);

        lstPairedDevices = (ListView) view.findViewById(R.id.lstPairedDevices);

        lstDiscoverDevices = (ListView) view.findViewById(R.id.lstDiscoverDevices);

        return view;
    }

    @Override
    public void onStart(){
        super.onStart();

        bluetoothStatusChanged();

        //Register the BroadcastReceiver
        IntentFilter filter = new IntentFilter();
        filter.addAction(BluetoothAdapter.ACTION_STATE_CHANGED); // bluetooth enabled/disabled
        filter.addAction(BluetoothDevice.ACTION_FOUND);
        getActivity().registerReceiver(bluetoothReceiver, filter);
    }

    @Override
    public void onStop(){
        super.onStop();
        stopDiscoverTask();
        getActivity().unregisterReceiver(bluetoothReceiver);
    }

    public void bluetoothStatusChanged(){
        btnBluettothEnabled.setChecked(bluetoothAdapter.isEnabled());
        if(bluetoothAdapter.isEnabled()){
            getPairedDevices();
        }
    }

    public void getPairedDevices(){
        pairedDevices = bluetoothAdapter.getBondedDevices();

        PairedDeviceListAdapter adapter = new PairedDeviceListAdapter(pairedDevices);
        lstPairedDevices.setAdapter(adapter);
    }

    public void startDiscoverTask(){
        if(!bluetoothAdapter.isDiscovering() && discoveredDevices == null){
            bluetoothAdapter.startDiscovery();
            discoveredDevices = new ArrayList<>();
        }
    }

    public void stopDiscoverTask(){
        if(bluetoothAdapter.isDiscovering()){
            bluetoothAdapter.cancelDiscovery();
        }
    }

    public View.OnClickListener toggleBluetooth = new View.OnClickListener(){
        @Override
        public void onClick(View v) {
            if(bluetoothAdapter.isEnabled()){
                bluetoothAdapter.disable();
            }else{
                Intent turnOn = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
                startActivityForResult(turnOn, 1);
            }
        }
    };

    private final BroadcastReceiver bluetoothReceiver = new BroadcastReceiver() {

        @Override
        public void onReceive(Context context, Intent intent) {
            switch (intent.getAction()){
                case BluetoothAdapter.ACTION_STATE_CHANGED:
                    bluetoothStatusChanged();
                    break;
                case BluetoothDevice.ACTION_FOUND:
                    discoveredDevices.add((BluetoothDevice)intent.getParcelableExtra(BluetoothDevice.EXTRA_DEVICE));
                    lstDiscoverDevices.setAdapter(new PairedDeviceListAdapter(discoveredDevices));
                default:
                    Log.e("ERROR", intent.getAction());
            }
        }
    };

    @Override
    public void onActivityResult(int requestCode, int resultCode, Intent data) {
        // workaround to avoid wrong status if the user tries to enable the bluetooth from the app
        // but it denies the action in the action dialog
        bluetoothStatusChanged();
    }

    public class PairedDeviceListAdapter extends BaseAdapter {

        List<BluetoothDevice> devices;

        public PairedDeviceListAdapter(List<BluetoothDevice> list){
            this.devices = list;
        }

        public PairedDeviceListAdapter(Set<BluetoothDevice> set){
            devices = new ArrayList();
            devices.addAll(set);
        }

        @Override
        public int getCount() {
            return devices.size();
        }

        @Override
        public Object getItem(int position) {
            return devices.get(position);
        }

        @Override
        public long getItemId(int position) {
            return position;
        }

        @Override
        public View getView(int position, View view, final ViewGroup parent) {
            if (view == null) {
                LayoutInflater inflater = LayoutInflater.from(parent.getContext());
                view = inflater.inflate(R.layout.list_item_paired_devices, parent, false);
            }

            final BluetoothDevice device = devices.get(position);

            TextView txtName = (TextView) view.findViewById(R.id.txtName);
            txtName.setText(device.getName());

            ImageButton imgbtn = (ImageButton) view.findViewById(R.id.btnConnect);
            imgbtn.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View view) {
                    try {
                        BluetoothSocket socket = connect(device);
                        initIn3ControlPanel(socket);
                    } catch (IOException e) {
                        toast("error");
                        e.printStackTrace();
                    } catch (InvocationTargetException e) {
                        toast("error");
                        e.printStackTrace();
                    } catch (NoSuchMethodException e) {
                        toast("error");
                        e.printStackTrace();
                    } catch (IllegalAccessException e) {
                        toast("error");
                        e.printStackTrace();
                    }
                }
            });
            return view;
        }
    }

    private void initIn3ControlPanel(BluetoothSocket socket) {
        In3ControlPanelFragment f = new In3ControlPanelFragment();
        f.setSocket(socket);

        getFragmentManager().beginTransaction()
                .replace(R.id.fragment_container, f).addToBackStack(null).commit();
    }

    public BluetoothSocket connect(BluetoothDevice device) throws IOException, NoSuchMethodException, IllegalAccessException, InvocationTargetException {
        BluetoothSocket socket = null;
        if(!pairedDevices.contains(device)){
            pairDevice(device);
        }
        socket = getSocket(device);
        socket.connect();
        return socket;
    }


    private void pairDevice(BluetoothDevice device) throws NoSuchMethodException, InvocationTargetException, IllegalAccessException {
        Method method = device.getClass().getMethod("createBond", (Class[]) null);
        method.invoke(device, (Object[]) null);
    }

    private BluetoothSocket getSocket(BluetoothDevice device) throws NoSuchMethodException, InvocationTargetException, IllegalAccessException {
        stopDiscoverTask();
        Method m=device.getClass().getMethod("createRfcommSocket",new Class<?>[] {Integer.TYPE});
        return (BluetoothSocket)m.invoke(device,Integer.valueOf(1));
    }

    private void toast(String msg){
        Toast.makeText(getActivity(), msg, Toast.LENGTH_SHORT).show();
    }
}
