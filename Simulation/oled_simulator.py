import serial
import pygame
import sys

COM_PORT = 'COM3'
BAUD_RATE = 115200
SCALE = 4

pygame.init()
screen = pygame.display.set_mode((128 * SCALE, 64 * SCALE))
pygame.display.set_caption("STM32 OLED Simulator")

try:
    ser = serial.Serial(COM_PORT, BAUD_RATE, timeout=1)
except Exception as e:
    print(f"COM Port Opening Error: {e}")
    sys.exit()

def decode_and_draw(vram_bytes):
    surface = pygame.Surface((128, 64))
    surface.fill((0, 0, 0))

    for y in range(64):
        for x in range(128):
            page = y // 8
            bit = y % 8
            idx = page * 128 + x

            if vram_bytes[idx] & (1 << bit):
                surface.set_at((x, y), (255, 255, 255))
    return surface

print("Simulator is now Running.")
print("[A]:Left  [D]:Right  [E]:Menu Open/Select  [R]:Return")

buffer = bytearray()

while True:
    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            pygame.quit()
            sys.exit()
        elif event.type == pygame.KEYDOWN:
            if event.key == pygame.K_a: ser.write(b'A')
            elif event.key == pygame.K_d: ser.write(b'D')
            elif event.key == pygame.K_e: ser.write(b'E')
            elif event.key == pygame.K_r: ser.write(b'R')

    if ser.in_waiting > 0:
        buffer += ser.read(ser.in_waiting)

        while len(buffer) >= 1026:
            idx = buffer.find(b'\xff\xaa')

            if idx == -1:
                buffer = buffer[-1:]
                break

            if idx + 1026 <= len(buffer):
                vram = buffer[idx+2 : idx+1026]

                surface = decode_and_draw(vram)
                scaled = pygame.transform.scale(surface, (128 * SCALE, 64 * SCALE))
                screen.blit(scaled, (0, 0))
                pygame.display.flip()

                buffer = buffer[idx+1026:]
            else:
                break #
