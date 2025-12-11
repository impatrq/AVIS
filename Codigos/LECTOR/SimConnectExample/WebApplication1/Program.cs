using System;
using Microsoft.FlightSimulator.SimConnect;
using System.Runtime.InteropServices;

class Program
{
    static SimConnect simconnect = null;

    static void Main()
    {
        try
        {
            IntPtr hwnd = new IntPtr(0); // SimConnect necesita un identificador de ventana
            simconnect = new SimConnect("Ejemplo SimConnect", hwnd, 0x402, null, 0);
            Console.WriteLine("✅ Conectado a Microsoft Flight Simulator");

            // Solicitar la altitud del avión
            simconnect.RequestDataOnSimObject(1, SIMVAR_ALTITUDE, SIMCONNECT_OBJECT_ID_USER, SIMCONNECT_PERIOD.SECOND);
        }
        catch (COMException ex)
        {
            Console.WriteLine("❌ Error de conexión: " + ex.Message);
        }
    }
}

// Definición de variable
enum SIMVAR_ALTITUDE
{
    Altitude
}

