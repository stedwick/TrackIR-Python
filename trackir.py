import usb.core
import usb.util
import time
from typing import Tuple, List, Optional
import struct

class TrackIR:
    # TrackIR vendor and product IDs
    VENDOR_ID = 0x131d  # NaturalPoint
    PRODUCT_ID = 0x0159  # TrackIR
    
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
            print(f"Received: {' '.join([hex(x) for x in data])}")
            return data
        except usb.core.USBError as e:
            print(f"Error reading data: {e}")
            return None

    def turn_on_led(self):
        """Attempt to turn on the TrackIR LED"""
        # Based on USB captures, we'll need to experiment with these values
        init_sequence = [0x51, 0x0d, 0x01]  # Example sequence - needs verification
        self.send_command(init_sequence)
        
    def read_frame(self) -> Optional[dict]:
        """Read a frame of tracking data"""
        data = self.read_data()
        if data is None:
            return None
            
        # We'll need to implement the proper parsing logic once we understand the protocol
        return self._parse_frame(data)
    
    def _parse_frame(self, data: bytes) -> dict:
        """Parse a frame of data from the device"""
        # This is a placeholder - we'll need to implement the actual parsing logic
        # based on the protocol analysis
        return {
            'raw_data': data,
            'timestamp': time.time()
        }

def main():
    """Main function for testing and protocol analysis"""
    try:
        trackir = TrackIR()
        print("TrackIR device initialized successfully")
        
        # Turn on LED
        print("Attempting to turn on LED...")
        trackir.turn_on_led()
        
        # Read some frames for analysis
        print("Reading frames...")
        for _ in range(10):
            frame = trackir.read_frame()
            if frame:
                print(f"Frame received: {frame}")
            time.sleep(0.1)
            
    except Exception as e:
        print(f"Error: {e}")

if __name__ == "__main__":
    main()
