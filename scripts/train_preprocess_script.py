import os
import shutil
import re
import random
import cv2
import sys

# --- Configuration ---
CHUNK_SIZE = 50        # Number of contiguous frames per chunk
RANDOM_SEED = 50       # Fixed seed for reproducible Train/Val/Test splits

# Format: (directory_path, suffix, list_of_channels_to_exclude)
# Valid exclusion strings: "green", "gray", "hsv_v", "yuv_y"
sources = [
    ("bags/bucket_rectangle/obj_train_data/bucket", "bucket", []), 
    ("bags/bucket_rectangle/obj_train_data/bucket_bright", "bucket_bright", ["hsv_v"]), 
    ("bags/bucket_rectangle/obj_train_data/bucket_bright+gate", "bucket_bright+gate", ["hsv_v"]),
    ("bags/multiple_bucket_rectangle/labels/train/buckets_multiple", "buckets_multiple", []),
    ("bags/multiple_bucket_rectangle/labels/train/flag", "flag", []), 
    ("bags/multiple_bucket_rectangle/labels/train/maingate_buckets", "maingate_buckets", []),
    ("bags/multiple_bucket_rectangle/labels/train/misc_everything", "misc_everything", []),
]

# Destination directories
dest_dir = "bags/training/bucket_final_gray"
dest_labels_dir = os.path.join(dest_dir, "labels")
dest_images_dir = os.path.join(dest_dir, "images")

# --- Helper Functions ---
def extract_number(filename):
    match = re.search(r'(\d+)', filename)
    return int(match.group(1)) if match else -1

def preprocess(img_path, exclusions=None):
    """
    Reads an image and extracts base channels and their inverses.
    Dynamically skips any channel (and its inverse) listed in 'exclusions'.
    The red channel is permanently excluded.
    """
    if exclusions is None:
        exclusions = []
        
    img = cv2.imread(img_path)
    if img is None:
        print(f"Warning: Could not read image {img_path}")
        return None
    
    # Extract base channels (Red is entirely removed)
    green = img[:, :, 1]
    gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
    hsv_v = cv2.cvtColor(img, cv2.COLOR_BGR2HSV)[:, :, 2]
    yuv_y = cv2.cvtColor(img, cv2.COLOR_BGR2YUV)[:, :, 0] 

    bases = {
        "green": green,
        "gray": gray,
        "hsv_v": hsv_v,
        "yuv_y": yuv_y
    }

    results = {}
    for name, channel_img in bases.items():
        if name in exclusions:
            continue # Skip generating this channel and its inverse
            
        results[name] = channel_img                     # The base channel
        results[f"{name}_inv"] = 255 - channel_img      # The inverted complement

    return results

def preview_augmentations(source_list):
    """
    Randomly selects one image per directory and displays the non-excluded base variations.
    Waits for a key press to proceed to the next directory.
    """
    print("--- Starting Image Previews ---")
    print("Ensure the OpenCV windows are in focus. Press ANY KEY to advance to the next directory.")
    
    for _, suffix, exclusions in source_list:
        image_dir = f"bags/{suffix}"
        
        if not os.path.exists(image_dir):
            print(f"Skipping preview for {suffix}: Directory not found.")
            continue
            
        images = [f for f in os.listdir(image_dir) if f.lower().endswith(('.png', '.jpg', '.jpeg'))]
        if not images:
            print(f"Skipping preview for {suffix}: No images found.")
            continue
            
        random_img_name = random.choice(images)
        img_path = os.path.join(image_dir, random_img_name)
        
        # Use our preprocess function to get exactly what will be generated
        augmented_images = preprocess(img_path, exclusions)
        if augmented_images is None:
            continue
            
        # Always show the original for reference
        original_img = cv2.imread(img_path)
        cv2.imshow(f"Original - {suffix}", original_img)
        
        # Display windows dynamically based on what wasn't excluded
        for name, img_data in augmented_images.items():
            # Skip showing the inverses in the preview to keep the screen uncluttered
            if not name.endswith("_inv"):
                cv2.imshow(f"{name} - {suffix}", img_data)

        cv2.waitKey(0)
        cv2.destroyAllWindows()
        
    print("--- Previews Finished ---")
    
    while True:
        choice = input("Do you want to continue generating the dataset? (y/n): ").strip().lower()
        if choice == 'y':
            return True
        elif choice == 'n':
            return False
        else:
            print("Invalid input. Please enter 'y' or 'n'.")

# --- Main Execution ---

if not preview_augmentations(sources):
    print("Dataset generation aborted by user.")
    sys.exit(0)

print("\nProceeding with dataset generation...")

for split in ["train", "val", "test"]:
    os.makedirs(os.path.join(dest_labels_dir, split), exist_ok=True)
    os.makedirs(os.path.join(dest_images_dir, split), exist_ok=True)

for label_dir, suffix, exclusions in sources:
    image_dir = f"bags/{suffix}"
    
    if not os.path.exists(label_dir) or not os.path.exists(image_dir):
        print(f"Missing directories for {suffix}, skipping.")
        continue

    label_files = [f for f in os.listdir(label_dir) if f.startswith("img_") and f.endswith(".txt")]
    label_files = sorted(label_files, key=extract_number)

    # --- CHANGED: Integer-based Dictionary Mapping ---
    image_files = os.listdir(image_dir)
    img_dict = {}
    for img_f in image_files:
        if img_f.lower().endswith(('.png', '.jpg', '.jpeg')):
            img_num = extract_number(img_f)
            if img_num != -1:
                img_dict[img_num] = img_f

    # --- CHANGED: Integer-based Label Filtering ---
    valid_labels = []
    for lf in label_files:
        label_num = extract_number(lf)
        if label_num != -1 and label_num in img_dict:
            valid_labels.append(lf)

    chunks = [valid_labels[i:i + CHUNK_SIZE] for i in range(0, len(valid_labels), CHUNK_SIZE)]
    random.Random(RANDOM_SEED).shuffle(chunks)
    shuffled_files = [file for chunk in chunks for file in chunk]

    sz = len(shuffled_files)
    trainsz = int(sz * 0.7)
    valsz = int(sz * 0.85)

    for i, filename in enumerate(shuffled_files):
        if i < trainsz:
            split = "train"
        elif i < valsz:
            split = "val"
        else:
            split = "test"

        name, _ = os.path.splitext(filename) # name remains something like "img_0001" for output
        label_src_path = os.path.join(label_dir, filename)
        
        # --- CHANGED: Integer-based Image Lookup ---
        label_num = extract_number(filename)
        img_filename = img_dict[label_num]
        
        _, img_ext = os.path.splitext(img_filename)
        img_src_path = os.path.join(image_dir, img_filename)

        with open(label_src_path, 'r') as f:
            label_content = f.read()

        augmented_images = preprocess(img_src_path, exclusions)
        
        if augmented_images is None:
            continue

        for aug_name, aug_img in augmented_images.items():
            # Output will be something like img_0001_buckets_multiple_green.jpg
            new_base_name = f"{name}_{suffix}_{aug_name}"
            
            new_img_path = os.path.join(dest_images_dir, split, new_base_name + img_ext)
            new_label_path = os.path.join(dest_labels_dir, split, new_base_name + ".txt")

            cv2.imwrite(new_img_path, aug_img)

            with open(new_label_path, 'w') as f:
                f.write(label_content)
                
    print(f"Processed {suffix}: {sz} source frames -> {sz * len(augmented_images)} augmented files.")

print("\nDataset successfully generated!")