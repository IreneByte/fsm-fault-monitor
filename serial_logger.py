import serial
import csv

with serial.Serial() as ser:
    ser.baudrate = 9600
    ser.port = ''
    ser.open()

    filename = 'state_log.csv'

    with open(filename, 'w') as csvfile:
        csvwriter = csv.writer(csvfile)

        csvwriter.writerow(['timestamp', 'event'])

        try:
            while True:
                line = ser.readline().decode('utf-8').strip()
                if line:
                    data = line.split('] ')
                    data[0] = data[0].replace('[', '')
                    print(line)
                    csvwriter.writerow(data)
        except KeyboardInterrupt:
            print("Logging stopped.")
        except serial.SerialException:
            print("Arduino disconnected.")
        except Exception as e:
            print(f"Unexpected error: {e}")


