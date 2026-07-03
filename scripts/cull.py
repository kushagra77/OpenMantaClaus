import os
import re
from pathlib import Path

# Hardcoded settings
TARGET_DIRECTORY = "bags/finetune2"
N = 2

IMAGE_EXTENSIONS = {".jpg", ".jpeg", ".png", ".bmp", ".tiff", ".webp"}


def numeric_sort_key(path: Path):
    """Sort by embedded number when available, otherwise by filename."""
    matches = re.findall(r"\d+", path.stem)
    if matches:
        return (0, int(matches[-1]), path.name.lower())
    return (1, path.name.lower())


def renumber_images_consecutively(target: Path):
    """Rename all images in a folder to img_0001, img_0002, ... in sorted order."""
    images = [
        p for p in target.iterdir()
        if p.is_file() and p.suffix.lower() in IMAGE_EXTENSIONS
    ]
    images.sort(key=numeric_sort_key)

    if not images:
        print(f"No images left to renumber in {target}")
        return 0

    temp_paths = []

    # First pass: move files to temporary names to avoid rename collisions.
    for index, src in enumerate(images, start=1):
        tmp = target / f".__renumber_tmp__{index}{src.suffix.lower()}"
        src.rename(tmp)
        temp_paths.append((tmp, index))

    # Second pass: assign final consecutive names.
    for tmp, index in temp_paths:
        final = target / f"img_{index:04d}{tmp.suffix.lower()}"
        tmp.rename(final)
        print(f"Renamed: {final.name}")

    return len(temp_paths)


def main():
    target = Path(TARGET_DIRECTORY)

    if N <= 0:
        print(f"N must be greater than 0. Got: {N}")
        return

    if not target.exists() or not target.is_dir():
        print(f"Directory does not exist or is not a folder: {target}")
        return

    images = [
        p for p in target.iterdir()
        if p.is_file() and p.suffix.lower() in IMAGE_EXTENSIONS
    ]

    images.sort(key=numeric_sort_key)

    if not images:
        print(f"No images found in {target}")
        return

    # Delete every Nth image in sorted order: N, 2N, 3N, ... (1-based positions)
    to_delete = [images[i] for i in range(N - 1, len(images), N)]

    deleted = 0
    for image_path in to_delete:
        try:
            image_path.unlink()
            deleted += 1
            print(f"Deleted: {image_path}")
        except OSError as exc:
            print(f"Failed to delete {image_path}: {exc}")

    renumbered = renumber_images_consecutively(target)
    print(
        f"Done. Deleted {deleted} of {len(images)} images from {target}. "
        f"Renumbered {renumbered} images consecutively."
    )


if __name__ == "__main__":
    main()