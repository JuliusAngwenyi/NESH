using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Microsoft.Azure.Devices.Client;
using Newtonsoft.Json;

namespace SimulatedDevice
{
    class Program
    {
        static DeviceClient deviceClient;
        static string iotHubUri = "{iot hub hostname};
        static string deviceKey = "{device key}";

        static void Main(string[] args)
        {
            Console.WriteLine("Simulated device\n");
            deviceClient = DeviceClient.Create(iotHubUri, new DeviceAuthenticationWithRegistrySymmetricKey("NeshMKR100Dev1", deviceKey));

            SendDeviceToCloudMessagesAsync();
            Console.ReadLine();
        }

        private static async void SendDeviceToCloudMessagesAsync()
        {
            string deviceId = "YourDeviceID";

            while (true)
            {
                //var motionAvailable = rand.Next(0,1); //1 = yes, 0= No
                string messageString = await SendMessageNow(deviceId,1);
                Console.WriteLine("{0} > Sending message: {1}", DateTime.Now, messageString);
                Task.Delay(5000).Wait();
                messageString = await SendMessageNow(deviceId, 0);
                Console.WriteLine("{0} > Sending message: {1}", DateTime.Now, messageString);

                Task.Delay(15000).Wait();
            }
        }

        private static async Task<string> SendMessageNow(string thisDeviceId,byte motionDetected)
        {
            var motionDataPoint = new
            {
                deviceId = thisDeviceId,
                motion = motionDetected
                //timestamp = DateTime.UtcNow.ToString("yyyy-M-dTHH:m")
            };
            var messageString = JsonConvert.SerializeObject(motionDataPoint);
            var message = new Message(Encoding.ASCII.GetBytes(messageString));

            await deviceClient.SendEventAsync(message);
            return messageString;
        }
    }
}
