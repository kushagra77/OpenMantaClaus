from pathlib import Path
from typing import List

import cv2


class ImageSequence:
    def __init__(self, image_dir: str) -> None:
        self._image_dir = Path(image_dir)
        if not self._image_dir.is_dir():
            raise FileNotFoundError(f"Image directory does not exist: {self._image_dir}")

        self._image_files = self._discover_images()
        if not self._image_files:
            raise FileNotFoundError(f"No image files found in directory {self._image_dir}")

        self._index = 0

    def _discover_images(self) -> List[Path]:
        image_files = [
            path
            for path in self._image_dir.iterdir()
            if path.is_file() and path.suffix.lower() in {".jpg", ".jpeg", ".png"}
        ]

        def sort_key(path: Path):
            stem = path.stem
            if stem.startswith("img_"):
                suffix = stem[len("img_") :]
                if suffix.isdigit():
                    return (0, int(suffix), path.name)
            return (1, 0, path.name)

        return sorted(image_files, key=sort_key)

    def next_image(self):
        image_path = self._image_files[self._index]
        self._index = (self._index + 1) % len(self._image_files)
        image = cv2.imread(str(image_path))
        if image is None:
            raise RuntimeError(f"Failed to read image from {image_path}")
        return image_path, image
