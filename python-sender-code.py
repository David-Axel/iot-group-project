import serial  # Serial communication library
import datetime  # Date-time library
import urllib3  # HTTP request library
import time  # For delay

# Initialize serial communication
try:
    ser = serial.Serial('COM6', 9600)  # Replace 'COM6' with the correct port
    ser.flushInput()  # Clear the input buffer
    print("Serial connection established on COM6.")
except serial.SerialException as e:
    print(f"Error: Could not open serial port. {e}")
    exit(1)

# ThingSpeak setup
baseURL = 'https://api.thingspeak.com/update?api_key='
API_key = 'QWXTD1VKQ2QR6DF7'

# Main loop
while True:
    try:
        # Read a line of data from the serial port
        ser_bytes = ser.readline()
        decoded_bytes = ser_bytes.decode("utf-8").strip()  # Strip removes trailing newline or spaces
        print(f"Received data: {decoded_bytes}")  # Debugging line to show raw data

        # Parse incoming CSV data
        try:
            temperature, humidity, sound_level, distance, fan_state = decoded_bytes.split(",")
        except ValueError:
            print(f"Invalid data received: {decoded_bytes}")
            continue  # Skip processing if data format is incorrect

        # Convert fields to proper types
        fan_state = fan_state.strip()  # Remove any trailing spaces

        # Construct ThingSpeak URL
        try:
            http = urllib3.PoolManager()
            url = (
                f"{baseURL}{API_key}"
                f"&field3={temperature}&field4={humidity}&field5={sound_level}"
                f"&field6={distance}&field2={fan_state}"
            )
            response = http.request('GET', url)
            print(f"Constructed URL: {url}")  # Debugging URL

            # Check HTTP response
            if response.status == 200:
                print(f"Data sent successfully to ThingSpeak.")
            else:
                print(f"Failed to send data. HTTP Status: {response.status}")
        except Exception as e:
            print(f"Error sending data to ThingSpeak: {e}")

        # Log data with timestamp
        now = datetime.datetime.now()
        now_str = now.strftime("%Y-%m-%d %H:%M:%S")
        print(f"{now_str} | {temperature}, {humidity}, "
              f"{sound_level}, {distance}, Fan State: {'ON' if fan_state == '1' else 'OFF'}")


    except KeyboardInterrupt:
        print("\nScript interrupted by user. Exiting...")
        break  # Exit the loop when the user presses Ctrl+C
    except Exception as e:
        print(f"Unexpected error: {e}")
        break

# Close the serial connection on exit
ser.close()
print("Serial connection closed.")