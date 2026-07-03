import sensor, image, time, gc

print("Initializing Display...")
sensor.reset()
sensor.set_pixformat(sensor.RGB565)
sensor.set_framesize(sensor.QQVGA)
# Note: set_framesize() doesn't affect video playback, the video dictates its own size
sensor.skip_frames(time=2000)

try:
    reader = image.ImageIO("uni_neg.bin", "r")
    print("Video file opened successfully.")
except OSError:
    print("ERROR: Could not find the .bin file on the SD card.")
    raise

clock = time.clock()
print("Starting playback...")

LAB_THRESHOLD = (0, 50, -50, 43, -55, -3)
DARK_THRESHOLD = (0, 80)

while True:
    clock.tick()

    try:
        # 1. READ DIRECTLY TO FRAMEBUFFER
        # By setting copy_to_fb=True, we NEVER put the image on the Python heap.
        # It streams straight from the SD card to the IDE display memory.
        # 1. Read directly to the Framebuffer (Zero heap memory used!)
        img = reader.read(copy_to_fb=True)

        if img is None:
            print("End of video.")
            break
        # img.to_grayscale()
        # img.scale(x_scale=0.25,y_scale=0.25)
        # # img.histeq(adaptive=True, clip_limit=5)
        # # 2. Blur to reduce noise
        img.gaussian(3)
        # img.gaussian(1)
        # img.binary([DARK_THRESHOLD])
        # img.close(1)

        blobs = img.find_blobs([LAB_THRESHOLD], pixels_threshold=2000, area_threshold=2000, merge=True)

        if blobs:
            # Grab the largest dark mass
            biggest_bucket = max(blobs, key=lambda b: b.pixels())

            # Draw the bounding box.
            # Note: Because the image is grayscale, color=255 draws pure white.
            img.draw_rectangle(biggest_bucket.rect(), color=255)
            img.draw_cross(biggest_bucket.cx(), biggest_bucket.cy(), color=255)

            print("Bucket Centroid: X=%d, Y=%d" % (biggest_bucket.cx(), biggest_bucket.cy()))

        # 5. FORCE IDE UPDATE
        # This explicitly sends the fully drawn Framebuffer to your monitor.
        sensor.flush()
        # img.copy(copy_to_fb=True)

    except Exception as e:
        print("Playback stopped or file error:", e)
        break
