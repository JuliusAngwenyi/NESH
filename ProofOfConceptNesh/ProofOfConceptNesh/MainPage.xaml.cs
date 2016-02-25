﻿using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices.WindowsRuntime;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Controls.Primitives;
using Windows.UI.Xaml.Data;
using Windows.UI.Xaml.Input;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Navigation;
using Microsoft.Maker.Serial;
using Microsoft.Maker.RemoteWiring;

// The Blank Page item template is documented at http://go.microsoft.com/fwlink/?LinkId=402352&clcid=0x409

namespace ProofOfConceptNesh
{
    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public sealed partial class MainPage : Page
    {
        private bool useBluetooth = false;
        /*
         * select one of the following connection types, Bluetooth or USB. This solution contains code for both, but you should only use one at a time.
         *  to select one, set the boolean property above to TRUE to use bluetooth, or leave FALSE to use USB.
         *
         * If using Bluetooth + Windows Phone, you will need to remove the "USB" capability from this solution.
         *   - See the <DeviceCapabilities> near the bottom of the .appxmanifest file, which you can find in the Solution Explorer
         */

        //only one of these will be used, depending on the useBluetooth property above.
        BluetoothSerial bluetooth;
        UsbSerial usb;

        RemoteDevice arduino;

        public MainPage()
        {
            this.InitializeComponent();

            if (useBluetooth)
            {
                /*
                 * I've written my bluetooth device name as a parameter to the BluetoothSerial constructor. You should change this to your previously-paired
                 * device name if using Bluetooth. You can also use the BluetoothSerial.listAvailableDevicesAsync() function to list
                 * available devices, but that is not covered in this sample.
                 */
                bluetooth = new BluetoothSerial("RNBT-E072");

                arduino = new RemoteDevice(bluetooth);
                bluetooth.ConnectionEstablished += OnConnectionEstablished;

                //these parameters don't matter for bluetooth
                bluetooth.begin(0, 0);
            }
            else
            {
                /*
                 * I've written my Arduino device VID and PID as a parameter to the BluetoothSerial constructor. You should change this to your 
                 * device VID and PID if using USB. You can also use the UsbSerial.listAvailableDevicesAsync() function to list
                 * available devices, but that is not covered in this sample.
                 */
                usb = new UsbSerial("VID_2341", "PID_824E");   //I've written in my device D directly USB\VID_2341&PID_824E&MI_00


                arduino = new RemoteDevice(usb);
                usb.ConnectionEstablished += OnConnectionEstablished;

                //SerialConfig.8N1 is the default config for Arduino devices over USB
                usb.begin(115200, SerialConfig.SERIAL_8N1);
            }

        }

        private void OnConnectionEstablished()
        {
            //enable the buttons on the UI thread!
            var action = Dispatcher.RunAsync(Windows.UI.Core.CoreDispatcherPriority.Normal, new Windows.UI.Core.DispatchedHandler(() => {
                OnButton.IsEnabled = true;
                OffButton.IsEnabled = true;
            }));
        }

        private void OnButton_Click(object sender, RoutedEventArgs e)
        {
            //turn the LED connected to pin 6 ON
            arduino.digitalWrite(6, PinState.HIGH);
        }

        private void OffButton_Click(object sender, RoutedEventArgs e)
        {
            //turn the LED connected to pin 6 OFF
            arduino.digitalWrite(6, PinState.LOW);
        }
    }
}
