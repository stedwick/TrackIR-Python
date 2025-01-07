import usb.core
import usb.util

def initialize_trackir():
    # TrackIR vendor and product IDs
    VENDOR_ID = 0x1784  # NaturalPoint
    PRODUCT_ID = 0x0030  # TrackIR 5
    
    # Find the TrackIR device
    device = usb.core.find(idVendor=VENDOR_ID, idProduct=PRODUCT_ID)
    
    if device is None:
        raise ValueError('TrackIR device not found')
        
    # Set the active configuration
    device.set_configuration()
    
    # Get an endpoint instance
    cfg = device.get_active_configuration()
    intf = cfg[(0,0)]
    
    # Get the IN and OUT endpoints
    ep_out = usb.util.find_descriptor(
        intf,
        custom_match = lambda e: \
            usb.util.endpoint_direction(e.bEndpointAddress) == \
            usb.util.ENDPOINT_OUT
    )
    
    ep_in = usb.util.find_descriptor(
        intf,
        custom_match = lambda e: \
            usb.util.endpoint_direction(e.bEndpointAddress) == \
            usb.util.ENDPOINT_IN
    )
    
    if ep_out is None or ep_in is None:
        raise ValueError('Could not find endpoints')
        
    return device, ep_in, ep_out

def turn_on_led(device, ep_out):
    # Send initialization sequence
    # Note: This is a placeholder - we'll need to determine the actual sequence
    init_sequence = [0x00]  # Replace with actual init sequence
    ep_out.write(init_sequence)
