using MQTTnet.Client;
using MQTTnet;
using System;
using System.Threading.Tasks;
using MQTTnet.Protocol;

namespace ConsoleApp1
{
    internal class Program
    {
        static async Task Main(string[] args)
        {
            var factory = new MqttFactory();
            var mqttClient = factory.CreateMqttClient();

            var options = new MqttClientOptionsBuilder()
                .WithClientId("ConsoleApp1Client")
                .WithTcpServer("localhost", 1883) // Change to your Mosquitto broker address and port
                .WithCleanSession()
                .Build();

            

            // ...

            mqttClient.ConnectedAsync += async e =>
            {
                Console.WriteLine("Connected to MQTT broker.");

                // Start a new task to publish messages every second
                _ = Task.Run(async () =>
                {
                    while (true)
                    {
                        var message = new MqttApplicationMessageBuilder()
                            .WithTopic("datetime")
                            .WithPayload(DateTime.Now.ToString())
                            .WithQualityOfServiceLevel(MqttQualityOfServiceLevel.ExactlyOnce)
                            .WithRetainFlag()
                            .Build();

                        await mqttClient.PublishAsync(message);
                        Console.WriteLine("Message published.");

                        // Wait for 1 second
                        await Task.Delay(1000);
                    }
                });
            };

            // ...

            mqttClient.DisconnectedAsync += e =>
            {
                Console.WriteLine("Disconnected from MQTT broker.");
                return Task.CompletedTask;
            };

            await mqttClient.ConnectAsync(options);

            Console.WriteLine("Press any key to exit.");
            Console.ReadLine();

            await mqttClient.DisconnectAsync();
        }
    }
}
