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
        static string iotHubUri = "{iot hub hostname}";
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

            Random rand = new Random();

            while (true)
            {
                var motionAvailable = rand.Next(0,1); //1 = yes, 0= No

                var motionDataPoint = new
                {
                    deviceId = "YourDeviceId",
                    motion = motionAvailable,
                    timestamp = DateTime.UtcNow.ToString("yyyy-M-dTHH:m")
                };
                var messageString = JsonConvert.SerializeObject(motionDataPoint);
                var message = new Message(Encoding.ASCII.GetBytes(messageString));

                await deviceClient.SendEventAsync(message);
                Console.WriteLine("{0} > Sending message: {1}", DateTime.Now, messageString);

                Task.Delay(15000).Wait();
            }
        }
    }
}
