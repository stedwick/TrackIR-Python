import usb.core
import usb.util
import time
from typing import Tuple, List, Optional
import struct

class TrackIR:
    # TrackIR vendor and product IDs
    VENDOR_ID = 0x131d  # NaturalPoint
    PRODUCT_ID = 0x0155  # TrackIR
    
    def __init__(self):
        self.device = None
        self.ep_in = None
        self.ep_out = None
        self.initialize()
    
    def initialize(self):
        """Initialize the TrackIR device and set up endpoints"""
        # Find the TrackIR device
        self.device = usb.core.find(idVendor=self.VENDOR_ID, idProduct=self.PRODUCT_ID)
        
        if self.device is None:
            raise ValueError('TrackIR device not found')
            
        # Reset the device
        self.device.reset()
        
        # Set the active configuration
        self.device.set_configuration()
        
        # Get an endpoint instance
        cfg = self.device.get_active_configuration()
        intf = cfg[(0,0)]
        
        # Get the IN and OUT endpoints
        self.ep_out = usb.util.find_descriptor(
            intf,
            custom_match = lambda e: \
                usb.util.endpoint_direction(e.bEndpointAddress) == \
                usb.util.ENDPOINT_OUT
        )
        
        self.ep_in = usb.util.find_descriptor(
            intf,
            custom_match = lambda e: \
                usb.util.endpoint_direction(e.bEndpointAddress) == \
                usb.util.ENDPOINT_IN
        )
        
        if self.ep_out is None or self.ep_in is None:
            raise ValueError('Could not find endpoints')

        # After setting up endpoints, detach the kernel driver if it's active
        if self.device.is_kernel_driver_active(0):
            self.device.detach_kernel_driver(0)

        # Print device information after initialization
        self.print_device_info()

    def send_command(self, data: List[int], timeout: int = 1000) -> None:
        """Send a command to the device and log it"""
        try:
            bytes_written = self.ep_out.write(data, timeout=timeout)
            print(f"Sent: {' '.join([hex(x) for x in data])} ({bytes_written} bytes)")
        except usb.core.USBError as e:
            print(f"Error sending command: {e}")

    def read_data(self, length: int = 64, timeout: int = 1000) -> Optional[bytes]:
        """Read data from the device and log it"""
        try:
            data = self.ep_in.read(length, timeout=timeout)
            if len(data) > 0:  # Only print if we actually got data
                # Print in hex format for debugging
                hex_data = ' '.join([f"{x:02x}" for x in data])
                print(f"Received [{len(data)} bytes]: {hex_data}")
            return data
        except usb.core.USBError as e:
            if e.errno == 110:  # Operation timed out
                return None
            print(f"Error reading data: {e}")
            return None

    def turn_on_led(self):
        """Attempt to turn on the TrackIR LED"""
        # Based on USB captures, we'll need to experiment with these values
        init_sequence = [0x51, 0x0d, 0x01]  # Example sequence - needs verification
        self.send_command(init_sequence)
        
    def read_frame(self) -> Optional[dict]:
        """Read the most recent frame of tracking data"""
        # Flush any old data from the buffer
        while True:
            try:
                self.ep_in.read(64, timeout=1)
            except usb.core.USBError as e:
                if e.errno == 110:  # Timeout means buffer is empty
                    break
                else:
                    print(f"Error flushing buffer: {e}")
                    break
        
        # Now read the latest frame
        data = self.read_data()
        if data is None:
            return None
            
        return self._parse_frame(data)
    
    def _parse_frame(self, data: bytes) -> dict:
        """Parse a frame of data from the device based on LinuxTrack's implementation"""
        if len(data) < 6:
            return None
            
        frame_length = data[0]
        frame_type = data[1]
        
        if frame_type == 0x1C:
            # Get sensor data bytes
            sensor_data = data[2:frame_length]
            
            # Process data in 4-byte chunks
            pixels = []
            for i in range(0, len(sensor_data), 4):
                if i + 4 <= len(sensor_data):
                    row = sensor_data[i]
                    x = sensor_data[i + 1]
                    y = sensor_data[i + 2]
                    delimiter = sensor_data[i + 3]
                    pixels.append((row, x, y, delimiter))
            
            frame = {
                'type': 'data_frame',
                'length': frame_length,
                'data': [data[i] for i in range(2, frame_length)],
                'pixels': pixels,
                'raw_data': data,
                'timestamp': time.time()
            }
            
            frame['visualization'] = self._visualize_pixels(pixels)
            return frame
        elif frame_type == 0x10:
            return {
                'type': 'info_frame',
                'length': frame_length,
                'data': [data[i] for i in range(2, frame_length)],
                'raw_data': data,
                'timestamp': time.time()
            }
        elif frame_type == 0x40:
            return {
                'type': 'config_frame',
                'length': frame_length,
                'data': [data[i] for i in range(2, frame_length)],
                'raw_data': data,
                'timestamp': time.time()
            }
        else:
            return {
                'type': f'unknown_frame_0x{frame_type:02x}',
                'length': frame_length,
                'data': [data[i] for i in range(2, frame_length)],
                'raw_data': data,
                'timestamp': time.time()
            }

    def _visualize_frame(self, bits: List[int], width: int = 8) -> str:
        """Create an ASCII visualization of the frame data"""
        if not bits:
            return "No data"
            
        # TrackIR 3 appears to use 8-bit wide data
        lines = []
        for i in range(0, len(bits), width):
            line_bits = bits[i:i + width]
            # Show intensity values
            line = ''.join('█' if bit else '.' for bit in line_bits)
            lines.append(line)
            
        return '\n'.join(lines)

    def send_control(self, request_type, request, value, index, data=None):
        """Send a control transfer to the device"""
        try:
            result = self.device.ctrl_transfer(
                request_type,
                request,
                value,
                index,
                data if data else [],
                timeout=1000
            )
            print(f"Control transfer - Type: {hex(request_type)}, Request: {hex(request)}, "
                  f"Value: {hex(value)}, Index: {hex(index)}, Result: {result}")
            return result
        except usb.core.USBError as e:
            print(f"Control transfer error: {e}")
            return None

    def init_device(self):
        """Complete device initialization sequence based on LinuxTrack's implementation"""
        print("Starting device initialization...")
        
        # Initial setup commands from LinuxTrack
        init_sequence = [
            [0x12],           # Request version
            [0x14, 0x01],     # Initialize
            [0x14, 0x02],     # Start streaming
            [0x10, 0x02],     # Set data format
            [0x11, 0x80],     # Set exposure
            [0x11, 0x70],     # Set gain
            [0x11, 0x81],     # Enable sensor
            [0x13],           # Get info
            [0x17],           # Get config
        ]
        
        # Send initial setup commands
        for i, cmd in enumerate(init_sequence):
            print(f"\nSending initialization command {i+1}/{len(init_sequence)}...")
            try:
                bytes_written = self.ep_out.write(cmd)
                print(f"Sent: {' '.join([hex(x) for x in cmd])} ({bytes_written} bytes)")
                time.sleep(0.1)
                
                # Try to read any response
                response = self.read_data(timeout=100)
                if response:
                    print(f"Response received: {' '.join([hex(x) for x in response])}")
            
            except usb.core.USBError as e:
                print(f"Command {i+1} failed: {e}")
                return False

        # LED control constants from tir4.py
        TIR_LED_MSGID = 0x10
        TIR_IR_LED_BIT_MASK = 0x80
        TIR_GREEN_LED_BIT_MASK = 0x20
        
        # Turn on IR and Green LEDs with full intensity
        led_sequence = [
            [TIR_LED_MSGID, TIR_IR_LED_BIT_MASK | TIR_GREEN_LED_BIT_MASK, 0xFF],  # Turn on IR and Green LEDs
            [0x11, 0x50],  # Set LED brightness
        ]
        
        # Send LED commands
        for cmd in led_sequence:
            try:
                bytes_written = self.ep_out.write(cmd)
                print(f"Sent LED command: {' '.join([hex(x) for x in cmd])} ({bytes_written} bytes)")
                time.sleep(0.1)
            except usb.core.USBError as e:
                print(f"LED command failed: {e}")
                return False

        return True

    def print_device_info(self):
        """Print detailed USB device information"""
        print("\nDevice Information:")
        print(f"Device ID: {self.device.idVendor:04x}:{self.device.idProduct:04x}")
        
        # Get device configuration
        cfg = self.device.get_active_configuration()
        print(f"\nConfiguration:")
        print(f"  bNumInterfaces: {cfg.bNumInterfaces}")
        
        # Print interface information
        for intf in cfg:
            print(f"\nInterface {intf.bInterfaceNumber}:")
            print(f"  bInterfaceClass: {intf.bInterfaceClass}")
            print(f"  bInterfaceSubClass: {intf.bInterfaceSubClass}")
            print(f"  bInterfaceProtocol: {intf.bInterfaceProtocol}")
            
            # Print endpoint information
            for ep in intf:
                print(f"\n  Endpoint {ep.bEndpointAddress:02x}:")
                print(f"    Type: {usb.util.endpoint_type(ep.bmAttributes):02x}")
                print(f"    Max Packet Size: {ep.wMaxPacketSize}")
                print(f"    Interval: {ep.bInterval}")

    def start_data_stream(self):
        """Start the continuous data stream from the device"""
        # Based on the LinuxTrack code, send the start streaming command
        try:
            # Command to start streaming
            self.send_command([0x14, 0x02])
            time.sleep(0.1)
            
            # Verify we're getting data frames
            response = self.read_data(timeout=100)
            if response and response[1] == 0x1C:
                print("Data stream started successfully")
                return True
            else:
                print("Failed to start data stream")
                return False
                
        except usb.core.USBError as e:
            print(f"Error starting data stream: {e}")
            return False

    def _visualize_pixels(self, pixels: List[Tuple[int, int, int, int]], width: int = 64, height: int = 24) -> str:
        """Create an ASCII visualization of the pixel data at 1/4 size"""
        if not pixels:
            return "No data"
            
        # Create border and grid
        output = []
        output.append('┌' + '─' * width + '┐')
        
        # Create a 2D grid for visualization (quarter width and height)
        grid = [['.'] * width for _ in range(height)]
        
        # Plot each pixel with scaling
        for row, x, y, _ in pixels:
            # Scale coordinates to fit our smaller grid
            scaled_x = int((x / 255.0) * (width - 1))
            scaled_y = int((y / 255.0) * (height - 1))
            
            if 0 <= scaled_x < width and 0 <= scaled_y < height:
                grid[scaled_y][scaled_x] = '█'
                # Add some neighboring pixels to make it more visible
                for dx in [-1, 0, 1]:
                    for dy in [-1, 0, 1]:
                        nx = scaled_x + dx
                        ny = scaled_y + dy
                        if 0 <= nx < width and 0 <= ny < height:
                            if grid[ny][nx] == '.':
                                grid[ny][nx] = '▒'
        
        # Add grid lines with borders
        for row in grid:
            output.append('│' + ''.join(row) + '│')
        
        # Add bottom border
        output.append('└' + '─' * width + '┘')
        
        return '\n'.join(output)

def main():
    """Main function for testing and protocol analysis"""
    try:
        trackir = TrackIR()
        print("TrackIR device initialized successfully")
        
        if not trackir.init_device():
            print("Device initialization failed!")
            return
        
        # Wait for device to stabilize and discard initial frames
        print("\nWaiting for device to stabilize...")
        time.sleep(1.0)  # Wait 1 second
        
        # Discard a few frames
        for _ in range(5):
            trackir.read_frame()
        
        # LED control constants
        TIR_LED_MSGID = 0x10
        TIR_IR_LED_BIT_MASK = 0x80
        TIR_GREEN_LED_BIT_MASK = 0x20
        
        # Clear screen and hide cursor
        print("\033[2J\033[?25l", end='')
        
        try:
            # Read frames at 10 FPS for 10 seconds
            print("\nStarting 10 second capture at 10 FPS...\n")
            frame_time = 1.0 / 10.0  # 100ms per frame
            start_time = time.time()
            frame_count = 0
            
            while time.time() - start_time < 10.0:  # Run for 10 seconds
                frame_start = time.time()
                
                # Move cursor to top and clear screen
                print("\033[H\033[2J", end='')
                print(f"Frame {frame_count} at {time.time():.2f} ({(time.time() - start_time):.1f}s elapsed)")
                
                # Maintain LED state
                trackir.send_command([TIR_LED_MSGID, TIR_IR_LED_BIT_MASK | TIR_GREEN_LED_BIT_MASK, 0xFF])
                
                frame = trackir.read_frame()
                if frame and frame['type'] == 'data_frame':
                    data_bytes = frame['data']
                    
                    if len(data_bytes) == 0:
                        print("Warning: No data in frame")
                        continue
                    
                    # Convert data into pixels format
                    pixels = []
                    for i in range(0, len(data_bytes), 4):
                        if i + 4 <= len(data_bytes):
                            row = data_bytes[i]
                            x = data_bytes[i + 1]
                            y = data_bytes[i + 2]
                            delimiter = data_bytes[i + 3]
                            pixels.append((row, x, y, delimiter))
                    
                    print(trackir._visualize_pixels(pixels))
                else:
                    print("Warning: Invalid or empty frame")
                
                # Calculate time to sleep
                frame_end = time.time()
                sleep_time = frame_time - (frame_end - frame_start)
                if sleep_time > 0:
                    time.sleep(sleep_time)
                
                frame_count += 1
                
        finally:
            # Show cursor again
            print("\033[?25h", end='')
            
    except KeyboardInterrupt:
        print("\nCapture stopped by user")
    except Exception as e:
        print(f"Error: {e}")
    finally:
        # Turn off LED and restore terminal
        if hasattr(trackir, 'device'):
            trackir.send_command([TIR_LED_MSGID, 0x00, 0xFF])  # Turn off all LEDs
            print("\033[?25h", end='')  # Show cursor
            usb.util.dispose_resources(trackir.device)

if __name__ == "__main__":
    main()
