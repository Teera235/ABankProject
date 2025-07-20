import socket

UDP_IP = "0.0.0.0"  # รับทุกที่
UDP_PORT = 3000     # ตรงกับใน Arduino

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind((UDP_IP, UDP_PORT))

print(f"Listening on UDP port {UDP_PORT}...")

while True:
    data, addr = sock.recvfrom(1024)
    print("Received:", data.decode())
