using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SerialNumberFileGenerator
{
    internal class Program
    {
        static void Main(string[] args)
        {


            Console.WriteLine("Enter the serial number, can be alfanumeric string");

            string serialNumber = Console.ReadLine();

            GenerateHexFile(serialNumber, @".\serialNumber.hex");

            Console.ReadLine();


        }

        static void GenerateHexFile(string serialNumber, string filename)
        {
            byte[] serialBytes = Encoding.ASCII.GetBytes(serialNumber);
            //if (!BitConverter.IsLittleEndian)
            //    Array.Reverse(serialBytes);

            // Extended Linear Address Record for UICR (0x1000_0000)
            byte[] upperAddressRecord = { 0x02, 0x00, 0x00, 0x04, 0x10, 0x00 };
            byte upperChecksum = CalculateChecksum(upperAddressRecord);
            string upperAddressHex = $":{BitConverter.ToString(upperAddressRecord).Replace("-", "")}{upperChecksum:X2}";

            // Data Record for UICR.CUSTOMER[0] at 0x10001080
            byte[] dataRecord = new byte[20];
            dataRecord[0] = 0x10; // Data length (16 bytes)
            dataRecord[1] = 0x10; // Offset High Byte (0x1080)
            dataRecord[2] = 0x80; // Offset Low Byte
            dataRecord[3] = 0x00; // Record Type (Data)
            Array.Copy(serialBytes, 0, dataRecord, 4, Math.Min(serialBytes.Length, 16));
            for (int i = 4 + serialBytes.Length; i < 20; i++)
            {
                dataRecord[i] = 0x00;
            }
            byte dataChecksum = CalculateChecksum(dataRecord);
            string dataRecordHex = $":{BitConverter.ToString(dataRecord).Replace("-", "")}{dataChecksum:X2}";

            // End of File Record
            string eofRecord = ":00000001FF";

            // Write all records to the .hex file
            using (StreamWriter file = new StreamWriter(filename))
            {
                file.WriteLine(upperAddressHex);
                file.WriteLine(dataRecordHex);
                file.WriteLine(eofRecord);
            }

            Console.WriteLine($"Generated {filename} for serial number {serialNumber}");
        }

        static byte CalculateChecksum(byte[] record)
        {
            int sum = 0;
            foreach (var b in record)
            {
                sum += b;
            }
            byte checksum = (byte)((0x100 - (sum & 0xFF)) & 0xFF);
            return checksum;
        }
    }
}
