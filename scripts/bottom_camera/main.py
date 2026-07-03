import sensor, image, time, os, gc, math
from pyb import UART

print("Initializing camera...")
sensor.reset()
sensor.set_pixformat(sensor.RGB565)
sensor.set_framesize(sensor.QQVGA) # 160x120
sensor.skip_frames(time = 2000)
print("Camera ready.")

uart = UART(3, 115200, timeout_char=1000)

# --- Configuration ---
CX = 80
CY = 60
DIST_THRESHOLD = 40.0
REACHED_THRESHOLD = 20.0

# Grayscale Thresholds (Min, Max)
DARK_THRESH = [(0, 100)]

LAB_THRESH = [(0, 50, -50, 43, -55, -3)]
BRIGHT_THRESH = [(220, 255)]

current_state = 2

print("Listening for commands...")

while True:
    # 1. NON-BLOCKING UART CHECK
    if uart.any():
        data = uart.readline()
        try:
            msg_str = data.decode('utf-8').strip()
            if msg_str:
                command = int(msg_str)

                if command == 0:
                    current_state = 0
                    sensor.set_auto_exposure(True)
                    sensor.set_auto_whitebal(True)
                    sensor.set_auto_gain(True)
                    sensor.skip_frames(time = 3000)
                    sensor.set_auto_exposure(False)
                    sensor.set_auto_whitebal(False)
                    sensor.set_auto_gain(False)

                elif command == 1:
                    current_state = 1

                elif command == 2:
                    current_state = 2

                elif 3 <= command <= 99:
                    current_state = 0
                    duration_sec = command
                    session_id = time.ticks_ms()
                    attempt = 1

                    while True:
                        filename = "vid_%d_%d.bin" % (session_id, attempt)
                        try:
                            m = image.ImageIO(filename, "w")
                            clock = time.clock()
                            start_ticks = time.ticks_ms()
                            while time.ticks_diff(time.ticks_ms(), start_ticks) < (duration_sec * 1000):
                                clock.tick()
                                img = sensor.snapshot()
                                m.write(img)
                            m.close()
                            del m
                        except OSError as e:
                            try:
                                if 'm' in locals(): del m
                            except: pass

                        gc.collect()
                        time.sleep_ms(200)
                        os.sync()
                        time.sleep_ms(200)

                        try:
                            file_info = os.stat(filename)
                            if file_info[6] > 0:
                                break
                            else:
                                try: os.remove(filename)
                                except: pass
                        except OSError:
                            pass

                        attempt += 1
                        time.sleep_ms(500)

        except ValueError:
            pass
        except Exception as e:
            pass
    # 2. CONTINUOUS STATE EXECUTION
    if current_state in [1, 2]:
        img = sensor.snapshot()
        img.gaussian(3)
        dark_blobs = img.find_blobs(LAB_THRESH, pixels_threshold=1500, area_threshold=2000, merge=True)
        img.to_grayscale()

        if dark_blobs:
            largest_dark = max(dark_blobs, key=lambda b: b.pixels())

            target_cx = -1
            target_cy = -1

            if current_state == 1:
                # MODE 1: Target is the dark blob itself
                img.draw_rectangle(largest_dark.rect(), color=127)
                img.draw_cross(largest_dark.cx(), largest_dark.cy(), color=127)
                target_cx = largest_dark.cx()
                target_cy = largest_dark.cy()

            elif current_state == 2:
                # MODE 2: Target is the bright white blob strictly INSIDE the dark bucket's radius
                box_x, box_y, box_w, box_h = largest_dark.rect()

                # Check 1: Is the center of the camera frame inside the dark bounding box?
                is_frame_centered_in_box = (box_x <= CX <= box_x + box_w) and (box_y <= CY <= box_y + box_h)

                # Check 2: Is the bucket's center close to the camera's center?
                bucket_dist_to_center = math.sqrt((largest_dark.cx() - CX)**2 + (largest_dark.cy() - CY)**2)
                is_bucket_close = bucket_dist_to_center <= DIST_THRESHOLD

                if is_frame_centered_in_box and is_bucket_close:
                    # Proceed with finding the ball inside the bucket
                    bright_blobs = img.find_blobs(BRIGHT_THRESH, roi=largest_dark.rect(), pixels_threshold=15, area_threshold=50, merge=True)

                    if bright_blobs:
                        valid_bright_blobs = []
                        bucket_radius = max(box_w, box_h) / 2.0

                        for b in bright_blobs:
                            dist = math.sqrt((b.cx() - largest_dark.cx())**2 + (b.cy() - largest_dark.cy())**2)
                            if dist <= bucket_radius:
                                valid_bright_blobs.append(b)

                        if valid_bright_blobs:
                            largest_bright = max(valid_bright_blobs, key=lambda b: b.pixels())

                            img.draw_rectangle(largest_dark.rect(), color=75)
                            img.draw_circle(largest_dark.cx(), largest_dark.cy(), int(bucket_radius), color=75)

                            img.draw_rectangle(largest_bright.rect(), color=255)
                            img.draw_cross(largest_bright.cx(), largest_bright.cy(), color=255)

                            target_cx = largest_bright.cx()
                            target_cy = largest_bright.cy()

                # If we didn't find a valid bright blob OR the bucket wasn't centered enough, fallback to the bucket
                if target_cx == -1:
                    img.draw_rectangle(largest_dark.rect(), color=127)
                    img.draw_cross(largest_dark.cx(), largest_dark.cy(), color=127)
                    target_cx = largest_dark.cx()
                    target_cy = largest_dark.cy()

            # --- Centroid Math ---
            if target_cx != -1 and target_cy != -1:
                dy = CX - target_cx
                dx = CY - target_cy

                angle_rad = math.atan2(dy, dx)
                angle_deg = int((math.degrees(angle_rad) + 360) % 360)

                dist = math.sqrt((dx ** 2) + (dy ** 2))

                if dist > DIST_THRESHOLD:
                    final_value = angle_deg + 360
                elif dist < REACHED_THRESHOLD:
                    final_value = 0
                else:
                    final_value = angle_deg

                uart.write("%d\n" % final_value)
                print("%d\n" % final_value)
            else:
                uart.write("1000\n")
                print("1000\n")
        else:
            uart.write("1000\n")
            print("1000\n")

    time.sleep_ms(40)
