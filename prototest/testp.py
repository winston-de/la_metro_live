import gtfs_realtime_pb2

with open('vehicles124.bin', 'rb') as file:
    # Read the entire file content into a bytes object
    bytes_data = file.read()
    # Convert the immutable bytes object to a mutable bytearray
    data = bytearray(bytes_data)

msg = gtfs_realtime_pb2.FeedMessage()
msg.ParseFromString(data)

with open('testo.txt', 'w+') as file2:
    file2.write(str(msg))