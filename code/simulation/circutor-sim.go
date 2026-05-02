package main

import (
	"fmt"
	"math/rand"
	"os"
	"path/filepath"
	"time"

	"github.com/tbrandon/mbserver"
)

func main() {
	// 1. Initialize the Modbus Server
	serv := mbserver.NewServer()
	err := serv.ListenTCP("127.0.0.1:5020")
	if err != nil {
		fmt.Printf("Failed to start server: %v\n", err)
		return
	}

	// 2. Background Task: Update Modbus Registers & Virtual Pins
	go func() {
		parentDir := "./virtual_pins"
		
		// Ensure the directory exists (Uses 'os')
		_ = os.MkdirAll(parentDir, 0755)
		fmt.Printf("Simulator active. Writing pins to: %s\n", parentDir)

		for {
			// --- Wiggle Modbus Values (Uses 'math/rand') ---
			// Voltage: 230V +/- 5V (2250 to 2350)

            // Linia 1
			voltage := 2250 + rand.Intn(100)
			serv.HoldingRegisters[0] = uint16(0)
			serv.HoldingRegisters[1] = uint16(voltage)

            corriente := 5
            serv.HoldingRegisters[2] = uint16(0)
            serv.HoldingRegisters[3] = uint16(corriente)

            potencia_activa := 25
            serv.HoldingRegisters[4] = uint16(0)
            serv.HoldingRegisters[5] = uint16(potencia_activa)

            // Linia 2
			voltage = 2250 + rand.Intn(100)
			serv.HoldingRegisters[10] = uint16(0)
			serv.HoldingRegisters[11] = uint16(voltage)

            corriente = 5
            serv.HoldingRegisters[12] = uint16(0)
            serv.HoldingRegisters[13] = uint16(corriente)

            potencia_activa = 25
            serv.HoldingRegisters[14] = uint16(0)
            serv.HoldingRegisters[15] = uint16(potencia_activa)

            // Linia 3
			voltage = 2250 + rand.Intn(100)
			serv.HoldingRegisters[20] = uint16(0)
			serv.HoldingRegisters[21] = uint16(voltage)

            corriente = 5
            serv.HoldingRegisters[22] = uint16(0)
            serv.HoldingRegisters[23] = uint16(corriente)

            potencia_activa = 25
            serv.HoldingRegisters[24] = uint16(0)
            serv.HoldingRegisters[25] = uint16(potencia_activa)

            energia_activa := 20
            serv.HoldingRegisters[60] = uint16(0)
            serv.HoldingRegisters[61] = uint16(energia_activa)

			// Temperature: 34.0C +/- 2C (320 to 360)
			temp := 320 + rand.Intn(40)
			serv.HoldingRegisters[80] = uint16(0)
			serv.HoldingRegisters[81] = uint16(temp)

			// --- Randomize Virtual Pins (Uses 'os' and 'path/filepath') ---
			entries, _ := os.ReadDir(parentDir)
			for _, entry := range entries {
				if entry.IsDir() {
					valPath := filepath.Join(parentDir, entry.Name(), "value")
					newVal := "0"
					if rand.Float32() > 0.5 {
						newVal = "1"
					}
					_ = os.WriteFile(valPath, []byte(newVal), 0644)
				}
			}

			time.Sleep(2 * time.Second)
		}
	}()

	fmt.Println("Circutor Simulator listening on 127.0.0.1:5020")
	fmt.Println("\nRegister  0: Voltage")
	fmt.Println("Register 80: Temp")
	fmt.Println("\nPress Ctrl+C to stop")
	
	select {} // Block forever
}
